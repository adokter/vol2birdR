// for mistnet:
#include <torch/script.h>
#include <iostream>
// for solver:
#include <torch/torch.h>
#include <vector>
#include <cstring>
#include <cmath>


#ifdef _WIN32
#define MISTNET_API __declspec(dllexport)
#else
#define MISTNET_API
#endif

extern "C" {
MISTNET_API int _mistnet_run_mistnet(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size)
{
        // ***************************************************************************
        // *************************                           ***********************
        // ************************* the code to use the model ***********************
        // *************************                           ***********************
        // ***************************************************************************

        torch::jit::script::Module module;
        try {
            module = torch::jit::load(model_path);
        }
        catch (const c10::Error& e) {
            std::cerr << "\nError: failed to load MistNet model from file " << model_path << "\n";
            return -1;
        }

        // if you already have a 1d floating point array that is the tensor of size 15 x 608 x 608
        // pointed by a pointer (float*) tensor_in, you can convert it to a torch tensor by:
        at::Tensor inputs = torch::from_blob(tensor_in, {1, 15, 608, 608}, at::kFloat);

        std::vector<torch::jit::IValue> inputs_;
        inputs_.push_back(inputs);

        at::Tensor output = module.forward(inputs_).toTensor();

        float *output_array = output.data_ptr<float>();

        // copy result
        for (int i=0; i<3*5*608*608; i++) (*tensor_out)[i] = output_array[i];

        return 0;

}

}

// ---------------------------------------------------------
// Solve λ D0 * v = rhs for v, where D0 is standard 1D Laplacian:
// diag = -2, offdiags = 1, SPD
// (Dirichlet BC implied).
// This is the Thomas algorithm (specialised Gaussian elimination
// for tridiagonal systems).
// Complexity: O(n), Memory: O(n) for the cprime temporary.
// ---------------------------------------------------------
static void laplacian_tridiag_solve_inplace(double lambda, double dx, torch::Tensor rhs) {
  int64_t n = rhs.size(0);

  // Off-diagonal = λ / dx²
  double a = lambda / (dx*dx);
  // Main diagonal = -2λ / dx²
  double b = -2.0 * lambda / (dx*dx);

  std::vector<double> cprime(n-1);
  auto acc = rhs.accessor<double,1>(); // direct element access

  // Forward sweep: eliminate lower diagonal
  double denom = b;
  cprime[0] = a / denom;
  acc[0]    = acc[0] / denom;
  for (int64_t i = 1; i < n; ++i) {
    denom = b - a * cprime[i-1];            // pivot
    if (i < n-1) cprime[i] = a / denom;     // update super diag ratio
    acc[i] = (acc[i] - a * acc[i-1]) / denom; // update RHS
  }

  // Back substitution: eliminate upper diagonal
  for (int64_t i = n-2; i >= 0; --i) {
    acc[i] -= cprime[i] * acc[i+1];
  }
}

// ---------------------------------------------------------
// Solve (Aᵀ A + λ D0) x = b using the Woodbury identity.
// Inputs:
//   m, n   - dimensions of A (m rows << n columns)
//   nnz    - number of non-zero entries in A
//   rows[] - row indices of non-zeros (length nnz)
//   cols[] - col indices of non-zeros (length nnz)
//   vals[] - values of non-zeros (length nnz)
//   b_data - right-hand side vector (length n)
//   lambda - regularisation weight
//   dx     - spatial grid spacing for D0
// Output:
//   x_out  - solution vector (length n)
//
// The algorithm never builds n×n matrices, only uses sparse A
// and O(n) tridiagonal solves, plus an m×m dense Cholesky.
// ---------------------------------------------------------
extern "C" {
    MISTNET_API void _mistnet_inversion_solver_cholesky(
      int64_t m, int64_t n,
      int64_t nnz,
      const int64_t* rows, const int64_t* cols, const double* vals, // COO data
      const double* b_data,   // length n
      double lambda,
      double dx,
      double* x_out           // output length n
  ) {
    // ------------ Step 1: Wrap COO triplets into a Torch sparse tensor (A) ------------
    // COO indices = 2×nnz (first row IDs, then col IDs)
    auto optsI = torch::TensorOptions().dtype(torch::kInt64);
    auto optsV = torch::TensorOptions().dtype(torch::kDouble);

    // Copy C arrays into Torch tensors (clone so we own the memory inside Tensor)
    torch::Tensor row_ix = torch::from_blob((int64_t*)rows, {nnz}, optsI).clone();
    torch::Tensor col_ix = torch::from_blob((int64_t*)cols, {nnz}, optsI).clone();
    torch::Tensor vals_t = torch::from_blob((double*)vals,  {nnz}, optsV).clone();

    // Stack row+col index tensors to shape (2, nnz)
    torch::Tensor indices = torch::stack({row_ix, col_ix});

    // Create the m×n sparse COO tensor
    auto A_sparse = torch::sparse_coo_tensor(indices, vals_t, {m, n});

    // ------------ Step 2: Wrap b as a Torch dense tensor ------------
    auto b = torch::from_blob((double*)b_data, {n}, optsV);

    // ------------ Step 3: y_b = D^{-1} b ------------
    // Apply the tridiagonal solve to b: O(n) time, O(n) memory.
    auto y_b = b.clone();
    laplacian_tridiag_solve_inplace(lambda, dx, y_b);

    // ------------ Step 4: Form small m×m matrix K = I + A D^{-1} Aᵀ ------------
    // We'll do it column-by-column: For each basis vector e_j in R^m,
    //   - take col_j = Aᵀ e_j  (n-vector, sparse matvec)
    //   - apply D⁻¹ tridiag solve
    //   - compute A col_j (m-vector)
    //   - this result is column j of K - I (we start K=I so just add)
    //
    // Complexity: m times [O(nnz_in_column_j) + O(n) + O(nnz)], dominated by O(m*n).
    auto K = torch::eye(m, optsV);
    for (int64_t j = 0; j < m; ++j) {
      // e_j: m-vector [0,...,1,...,0]
      auto e_j = torch::zeros({m}, optsV);
      e_j[j] = 1.0;

      // col_j = Aᵀ e_j is length n: picks out row j of A
      // matmul(sparse, dense): O(nnz_in_row_j)
      auto col_j = torch::matmul(A_sparse.transpose(0,1), e_j).to_dense().clone();

      // Apply tridiagonal inverse of λ D0: O(n)
      laplacian_tridiag_solve_inplace(lambda, dx, col_j);

      // Aj = A * col_j, length m (sparse matvec O(nnz))
      auto Aj = torch::matmul(A_sparse, col_j);

      // Add this vector into the j-th column of K
      K.select(1, j).add_(Aj);
    }

    // ------------ Step 5: Cholesky factorisation of K ------------
    // K is m×m SPD (since D0 SPD and AᵀA PSD), m~hundreds: negligible memory/time.
    auto L = torch::linalg_cholesky(K);

    // ------------ Step 6: u = A y_b ------------
    // sparse matvec, cost O(nnz)
    auto u = torch::matmul(A_sparse, y_b);

    // ------------ Step 7: Solve K z = u via Cholesky ------------
    // Two triangular solves: cost O(m²)
    auto tmp = std::get<0>(torch::triangular_solve(u.unsqueeze(1), L, /*upper=*/false));
    auto z   = std::get<0>(torch::triangular_solve(tmp, L.transpose(0,1), /*upper=*/true)).squeeze(1);

    // ------------ Step 8: w = D^{-1} (Aᵀ z) ------------
    // sparse matvec Aᵀ z -> length n (cost O(nnz)),
    // then tridiag solve O(n)
    auto At_z = torch::matmul(A_sparse.transpose(0,1), z).to_dense().clone();
    laplacian_tridiag_solve_inplace(lambda, dx, At_z);

    // ------------ Step 9: x = y_b - w ------------
    auto x = y_b - At_z;

    // ------------ Step 10: Copy result back to C array ------------
    std::memcpy(x_out, x.data_ptr<double>(), n * sizeof(double));
  }
}
