#ifdef __cplusplus
extern "C" {
#endif

int run_mistnet(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size);

// function declaration for external c++ code
// function defined in mistnet/src/mistnet.cpp
extern void inversion_solver_cholesky(
    double* A_data, long m, long n,
    double* b_data, double lambda, double dx,
    double* x_out
);


#ifdef __cplusplus
}
#endif
