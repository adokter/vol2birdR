#include "libinversion.h"
#include "libvol2bird.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* ======================================================================= */
/*                     CSR MATRIX UTILITIES                                */
/* ======================================================================= */

/**
 * Allocate memory for CSR matrix arrays.
 *  - nrows: number of rows in matrix
 *  - ncols: number of columns
 *  - nnz: total nonzero entries anticipated
 *
 * Memory layout:
 *   row_ptr[nrows+1] : start index of each row in col_idx/values arrays
 *   col_idx[nnz]     : column index for each nonzero
 *   values[nnz]      : corresponding value
 */
int CSR_init(CSRMatrix *F, size_t nrows, size_t ncols, size_t nnz) {
    F->nrows = nrows;
    F->ncols = ncols;
    F->row_ptr = malloc((nrows+1) * sizeof(size_t));
    F->col_idx = malloc(nnz * sizeof(size_t));
    F->values  = malloc(nnz * sizeof(double));

    if (!F->row_ptr || !F->col_idx || !F->values) {
        CSR_free(F);
        return INV_ERR_ALLOC_FAIL;
    }
    return INV_SUCCESS;
}

/**
 * Free all arrays inside CSR matrix struct.
 */
int CSR_free(CSRMatrix *F) {
    if (!F) return INV_ERR_INVALID_ARG;
    free(F->row_ptr);
    free(F->col_idx);
    free(F->values);
    F->nrows = F->ncols = 0;
    F->row_ptr = NULL;
    F->col_idx = NULL;
    F->values = NULL;
    return INV_SUCCESS;
}

/**
 * Begin building CSR matrix: initializes first row_ptr entry to 0.
 */
int CSR_begin_build(CSRMatrix *F) {
    if (!F || !F->row_ptr) return INV_ERR_INVALID_ARG;
    F->row_ptr[0] = 0;
    return INV_SUCCESS;
}

/**
 * Add a single row's nonzero entries to CSR matrix.
 *  - row_index: 0-based row index
 *  - col_idx_row: pointer to array of column indices for this row's nonzeros
 *  - val_row: pointer to array of values corresponding to col_idx_row
 *  - row_nnz: number of nonzeros in this row
 *
 * This writes the nonzeros contiguously into col_idx[] and values[] at the
 * position given by row_ptr[row_index], and then sets row_ptr[row_index+1]
 * to mark the end of this row's data.
 */
int CSR_add_row(CSRMatrix *F, size_t row_index,
                const size_t *col_idx_row,
                const double *val_row,
                size_t row_nnz) {
    if (!F || !col_idx_row || !val_row) return INV_ERR_INVALID_ARG;
    size_t start = F->row_ptr[row_index];
    for (size_t k = 0; k < row_nnz; ++k) {
        F->col_idx[start + k] = col_idx_row[k];
        F->values[start + k]  = val_row[k];
    }
    F->row_ptr[row_index+1] = start + row_nnz;
    return INV_SUCCESS;
}


/**
 * Finalize CSR matrix building. Provided for completeness; nothing special here.
 */
int CSR_finish_build(CSRMatrix *F) {
    /* Optional: check consistency */
    return INV_SUCCESS;
}

/* ======================================================================= */
/*             INTERNAL HELPER FUNCTIONS FOR GENERAL SOLVER                */
/* ======================================================================= */

/**
 * Compute G^T * G (the normal equations matrix) without forming G explicitly:
 * For radial velocity problem:
 *  - G = [diag(UFactor)F, diag(VFactor)F, diag(WFactor)F]
 *  - GTG is size (3*m) x (3*m), where m = number of vertical layers
 *
 * This loops over each observation (row), scales the row of F by each geometry
 * factor (UFactor, VFactor, WFactor), and accumulates contributions into GTG.
 */
/**
 * Compute GTG (normal equations matrix) for arbitrary nBlocks.
 *
 * Each block corresponds to one physical component vector (e.g., U, V, W, or η).
 * factorArrays[bi][row] = scaling factor for block bi at observation row.
 * If factorArrays[bi] == NULL => scaling factor = 1 for all rows.
 *
 * Matrix G is built as:
 *   columns 0..m-1        -> block 0 (scaled F rows)
 *   columns m..2m-1       -> block 1
 *   ...
 */
static void compute_GTG_CSR_blocks(const CSRMatrix *F,
                                   const double **factorArrays,
                                   size_t nBlocks,
                                   gsl_matrix *GTG)
{
    size_t n = F->nrows;
    size_t m = F->ncols;
    gsl_matrix_set_zero(GTG);

    for (size_t row = 0; row < n; ++row) {
        size_t start = F->row_ptr[row];
        size_t end   = F->row_ptr[row+1];
        for (size_t bi = 0; bi < nBlocks; ++bi) {
            double ai = (factorArrays && factorArrays[bi]) ? factorArrays[bi][row] : 1.0;
            for (size_t bj = 0; bj < nBlocks; ++bj) {
                double aj = (factorArrays && factorArrays[bj]) ? factorArrays[bj][row] : 1.0;
                for (size_t nz_i = start; nz_i < end; ++nz_i) {
                    size_t col_i = F->col_idx[nz_i];
                    double vi = ai * F->values[nz_i];
                    for (size_t nz_j = start; nz_j < end; ++nz_j) {
                        size_t col_j = F->col_idx[nz_j];
                        double vj = aj * F->values[nz_j];
                        double prev = gsl_matrix_get(GTG, bi*m + col_i, bj*m + col_j);
                        gsl_matrix_set(GTG, bi*m + col_i, bj*m + col_j, prev + vi*vj);
                    }
                }
            }
        }
    }
}
/**
 * Apply chosen regularization to GTG.
 * REG_L2: adds lambda*I (ridge)
 * REG_CURVATURE: adds lambda*(D^T D) block-diagonally, where D is second-difference matrix
 */
static void add_regularization(gsl_matrix* M, size_t m, double lambda, RegularizationType regtype)
{
    if (regtype == REG_NONE || lambda == 0.0) return;
    size_t dim = M->size1;

    if (regtype == REG_L2) {
        // Simple ridge: add lambda along diagonal
        for (size_t i = 0; i < dim; ++i)
            gsl_matrix_set(M, i, i, gsl_matrix_get(M, i, i) + lambda);
    }
    else if (regtype == REG_CURVATURE) {
        gsl_matrix *DTD = gsl_matrix_calloc(m, m);
        /* Build second-difference penalty matrix */
        for (size_t i = 0; i < m; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double val = 0.0;
                for (size_t r = 0; r < m-2; ++r) {
                    double Di_r = 0.0, Dj_r = 0.0;
                    if (i == r)   Di_r = 1.0;
                    if (i == r+1) Di_r = -2.0;
                    if (i == r+2) Di_r = 1.0;
                    if (j == r)   Dj_r = 1.0;
                    if (j == r+1) Dj_r = -2.0;
                    if (j == r+2) Dj_r = 1.0;
                    val += Di_r * Dj_r;
                }
                gsl_matrix_set(DTD, i, j, val);
            }
        }
        /* Add λ DTD for each block separately */
        // for radial velocity blocks are U,V,W
        for (size_t block = 0; block < dim/m; ++block) {
            for (size_t i = 0; i < m; ++i) {
                for (size_t j = 0; j < m; ++j) {
                    gsl_matrix_set(M, block*m + i, block*m + j,
                                   gsl_matrix_get(M, block*m + i, block*m + j) +
                                   lambda * gsl_matrix_get(DTD, i, j));
                }
            }
        }
        gsl_matrix_free(DTD);
    }
}

/**
 * Compute GTy = G^T * data for arbitrary nBlocks.
 * data[row] = measurement for this row (already unfolded if velocity).
 * Uses same geometry factors and CSR structure to avoid forming full G.

 */
static void compute_GTy_CSR_blocks(const CSRMatrix *F,
                                   const double **factorArrays,
                                   const double *data,
                                   size_t nBlocks,
                                   gsl_vector *GTy)
{
    size_t n = F->nrows;
    size_t m = F->ncols;
    gsl_vector_set_zero(GTy);

    for (size_t row = 0; row < n; ++row) {
        size_t start = F->row_ptr[row];
        size_t end   = F->row_ptr[row+1];
        for (size_t bi = 0; bi < nBlocks; ++bi) {
            double ai = (factorArrays && factorArrays[bi]) ? factorArrays[bi][row] : 1.0;
            for (size_t nz_i = start; nz_i < end; ++nz_i) {
                size_t col_i = F->col_idx[nz_i];
                double vi = ai * F->values[nz_i];
                gsl_vector_set(GTy, bi*m + col_i,
                               gsl_vector_get(GTy, bi*m + col_i) + vi * data[row]);
            }
        }
    }
}

/**
 * Sparse dot product between CSR row and dense vector.
 */
static double dot_CSR_row(const CSRMatrix *F, size_t row, const double *comp)
{
    double sum = 0.0;
    size_t start = F->row_ptr[row];
    size_t end   = F->row_ptr[row+1];
    for (size_t nz = start; nz < end; ++nz)
        sum += F->values[nz] * comp[F->col_idx[nz]];
    return sum;
}

/* ======================================================================= */
/*                          GENERAL SOLVER                                 */
/* ======================================================================= */

int solve_with_nyquist_reg_CSR_general(const CSRMatrix *F,
                                        const float *points,
                                        size_t nPoints,
                                        size_t nColsPoints,
                                        const size_t *dataCols,
                                        const double **factorArrays,
                                        double **outputs,
                                        int *k_vec,
                                        size_t m,
                                        size_t nBlocks,
                                        size_t max_iters,
                                        double lambda,
                                        RegularizationType regtype)
{
    /* -------------------------
     Step 1: Precompute geometry factors and data arrays
     ------------------------- */
    if (F->nrows != nPoints || F->ncols != m) {
        vol2bird_err_printf("Dimension mismatch between CSR matrix and problem sizes\n");
        return(INV_ERR_F_MATRIX_DIM);
    }

    /* Extract measured data into array (in velocity case, it's VRAD) */
    double *dataArr = malloc(nPoints * sizeof(double));
    for (size_t i = 0; i < nPoints; ++i)
        dataArr[i] = points[i*nColsPoints + dataCols[0]];

    /* -------------------------
     Step 2: Compute GTG once and apply regularization
     ------------------------- */
    gsl_matrix *GTG = gsl_matrix_alloc(nBlocks*m, nBlocks*m);
    compute_GTG_CSR_blocks(F, factorArrays, nBlocks, GTG);
    add_regularization(GTG, m, lambda, regtype);

    // Pre-factorize GTG for repeated solves
    gsl_permutation *perm = gsl_permutation_alloc(nBlocks*m);
    int signum;
    gsl_linalg_LU_decomp(GTG, perm, &signum);

    /* -------------------------
     Step 3: Allocate vectors for solves
     ------------------------- */
    gsl_vector *GTy = gsl_vector_alloc(nBlocks*m);
    gsl_vector *x   = gsl_vector_alloc(nBlocks*m);

    /* -------------------------
     Step 4: Iterative Nyquist folding solution
     ------------------------- */

    double *data_work = malloc(nPoints * sizeof(double));

    size_t iters = (k_vec != NULL) ? max_iters : 1;
    for (size_t iter = 0; iter < iters; ++iter) {
        // "Unfold" VRAD by subtracting multiples of 2*nyquist depending on k_vec
        if (k_vec) {
            for (size_t i = 0; i < nPoints; ++i)
                data_work[i] = dataArr[i] - 2.0 *
                               points[i*nColsPoints + dataCols[1]] * k_vec[i];
        } else {
            /* Reflectivity inversion: no unfolding */
            for (size_t i = 0; i < nPoints; ++i)
                data_work[i] = dataArr[i];
        }

        // Compute GT * VRAD_unfold
        /* Assemble GTy for this (possibly unfolded) data */
        compute_GTy_CSR_blocks(F, factorArrays, data_work, nBlocks, GTy);

        // Solve GTG * x = GTy for stacked [U; V; W]
        /* Solve for x (stacked profile vectors) */
        gsl_linalg_LU_solve(GTG, perm, GTy, x);

        // Extract U, V, W blocks from x
        /* Split solution x into outputs arrays */
        for (size_t bi = 0; bi < nBlocks; ++bi)
            for (size_t j = 0; j < m; ++j)
                outputs[bi][j] = gsl_vector_get(x, bi*m + j);

        if (k_vec) {
            // Update folding counts k based on new model predictions
            int changed = 0;
            for (size_t row = 0; row < nPoints; ++row) {
                double vmodel = 0.0;
                for (size_t bi = 0; bi < nBlocks; ++bi)
                  // Model radial velocity for this observation
                  // If factorArrays[bi] is NULL use 1.0 as multiplier,
                  // otherwise factorArrays[bi][row]
                    vmodel += (factorArrays[bi] ? factorArrays[bi][row] : 1.0) *
                              dot_CSR_row(F, row, outputs[bi]);
                double nyq = points[row*nColsPoints + dataCols[1]];
                // Required fold count to match VRAD to vmodel
                int newk = (int)round((dataArr[row] - vmodel) / (2.0 * nyq));
                if (newk != k_vec[row]) { changed = 1; k_vec[row] = newk; }
            }
            vol2bird_printf("[Iter %zu] k changed: %s\n", iter, changed ? "yes" : "no");
            if (!changed) break;
        }
    }

    /* -------------------------
     Step 5: Cleanup
     ------------------------- */
    gsl_permutation_free(perm);
    gsl_matrix_free(GTG);
    gsl_vector_free(GTy);
    gsl_vector_free(x);
    free(data_work);
    free(dataArr);
    return(INV_SUCCESS);
}

/* ======================================================================= */
/*                      WRAPPER: VELOCITY INVERSION                        */
/* ======================================================================= */

void solve_velocity_with_nyquist_reg_CSR(const CSRMatrix *F,
                                         const float *points,
                                         size_t nPoints,
                                         size_t nColsPoints,
                                         size_t colVRAD,
                                         size_t colAzim,
                                         size_t colElev,
                                         size_t colNyquist,
                                         double *U, double *V, double *W,
                                         int *k_vec,
                                         size_t m,
                                         size_t max_iters,
                                         double lambda,
                                         RegularizationType regtype)
{
    /* Precompute geometry scaling factors for U, V, W blocks */
    double *UFactor = malloc(nPoints * sizeof(double));
    double *VFactor = malloc(nPoints * sizeof(double));
    double *WFactor = malloc(nPoints * sizeof(double));
    for (size_t i = 0; i < nPoints; ++i) {
        double az = points[i*nColsPoints + colAzim];
        double el = points[i*nColsPoints + colElev];
        UFactor[i] = sin(az) * cos(el);
        VFactor[i] = cos(az) * cos(el);
        WFactor[i] = sin(el);
        k_vec[i]   = 0; /* initialise folding counts */
    }
    /* Data columns: 0 -> VRAD, 1 -> Nyquist (for folding calc) */
    size_t dataCols[2] = { colVRAD, colNyquist };

    double *outputs[3]    = { U, V, W };
    const double *factors[3] = { UFactor, VFactor, WFactor };

    solve_with_nyquist_reg_CSR_general(F, points, nPoints, nColsPoints,
                                       dataCols, factors, outputs,
                                       k_vec, m, 3, max_iters,
                                       lambda, regtype);

    free(UFactor); free(VFactor); free(WFactor);
}

/* ======================================================================= */
/*                      WRAPPER: REFLECTIVITY INVERSION                    */
/* ======================================================================= */

void solve_reflectivity_CSR(const CSRMatrix *F,
                            const float *points,
                            size_t nPoints,
                            size_t nColsPoints,
                            size_t colEta,
                            double *etaOut,
                            size_t m,
                            double lambda,
                            RegularizationType regtype)
{
    /* Data column index for reflectivity */
    size_t dataCols[1] = { colEta };
    double *outputs[1] = { etaOut };
    /* No geometry factors, no folding => factorArrays=NULL, k_vec=NULL */
    solve_with_nyquist_reg_CSR_general(F, points, nPoints, nColsPoints,
                                       dataCols, NULL, outputs,
                                       NULL, m, 1, 1,
                                       lambda, regtype);
}
