// for mistnet:
#include <torch/script.h>
#include <iostream>
// for inversion solver:
#include <torch/torch.h>
#include <cstring> // for std::memcpy

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


// C callable interface to solve (A^T A + λ D) x = b
//
// the regularization term is the second difference matrix, i.e. the
// tridiagonal matrix approximating second derivative, ensuring smoothness.
extern "C" {
MISTNET_API void _mistnet_inversion_solver_cholesky(
    double* A_data,    // pointer to m×n matrix data, row-major
    int64_t m, int64_t n,
    double* b_data,    // pointer to length-n RHS vector
    double lambda,     // regularization weight
    double dx,         // grid spacing for D (set 1.0 for unit spacing)
    double* x_out      // pointer to length-n output vector
) {
    // Wrap raw data as Torch tensors WITHOUT copying
    torch::Tensor A = torch::from_blob(A_data, {m, n}, torch::kDouble);
    torch::Tensor b = torch::from_blob(b_data, {n}, torch::kDouble);

    // 1. Form ATA = A^T A
    torch::Tensor ATA = torch::matmul(A.transpose(0, 1), A);

    // 2. Build tridiagonal finite-difference second-derivative matrix D
    torch::Tensor D = torch::zeros({n, n}, torch::kDouble);
    D.diagonal(0).fill_(-2.0);
    D.diagonal(1).fill_(1.0);
    D.diagonal(-1).fill_(1.0);

    if (dx > 0.0) {
        D.mul_(1.0 / (dx * dx));
    }

    // 3. Add λ D regularization
    ATA += lambda * D;

    // Optional: enforce symmetry for numerical safety
    ATA = 0.5 * (ATA + ATA.transpose(0, 1));

    // 4. Cholesky factorization: ATA = L L^T (lower = true)
    torch::Tensor L = torch::linalg_cholesky(ATA);

    // 5. Solve L y = b  (forward substitution)
    auto y_tuple = torch::triangular_solve(
        b.unsqueeze(1),  // make b a column vector
        L,
        /*upper=*/false, /*transpose=*/false, /*unitriangular=*/false
    );
    torch::Tensor y = std::get<0>(y_tuple); // solution of L y = b

    // 6. Solve L^T x = y  (back substitution)
    auto x_tuple = torch::triangular_solve(
        y,
        L.transpose(0, 1),
        /*upper=*/true, /*transpose=*/false, /*unitriangular=*/false
    );
    torch::Tensor x = std::get<0>(x_tuple).squeeze(1); // back to 1D

    // 7. Copy result to user-supplied output buffer
    std::memcpy(x_out, x.data_ptr<double>(), n * sizeof(double));
}

}
