// for mistnet:
#include <torch/script.h>
#include <iostream>

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
        double* A_data,    // m×n, row-major
        int64_t m, int64_t n,
        double* b_data,    // length n
        double lambda,     // regularisation weight
        double dx,         // grid spacing
        double* x_out
    ) {
      // Wrap existing data without copying
      torch::Tensor A = torch::from_blob(A_data, {m, n}, torch::kDouble);
      torch::Tensor b = torch::from_blob(b_data, {n}, torch::kDouble);

      // Compute ATA = A^T A
      torch::Tensor ATA = torch::matmul(A.transpose(0, 1), A);

      // In-place addition of λ * D (second derivative tridiagonal)
      double scale = (dx > 0.0) ? lambda / (dx * dx) : lambda;

      // main diagonal: add -2 * scale
      ATA.diagonal(0).add_(scale * (-2.0));

      // upper diagonal: add 1 * scale
      if (n > 1) {
        ATA.diagonal(1).add_(scale * (1.0));
        // lower diagonal: symmetric
        ATA.diagonal(-1).add_(scale * (1.0));
      }

      // Cholesky factorisation: ATA = L L^T
      torch::Tensor L = torch::linalg_cholesky(ATA);

      // Solve L y = b
      auto y_tuple = torch::triangular_solve(
        b.unsqueeze(1), L, /*upper=*/false, /*transpose=*/false, /*unitriangular=*/false);
      torch::Tensor y = std::get<0>(y_tuple);

      // Solve L^T x = y
      auto x_tuple = torch::triangular_solve(
        y, L.transpose(0, 1), /*upper=*/true, /*transpose=*/false, /*unitriangular=*/false);
      torch::Tensor x = std::get<0>(x_tuple).squeeze(1);

      // Copy result to output
      std::memcpy(x_out, x.data_ptr<double>(), n * sizeof(double));
    }
}
