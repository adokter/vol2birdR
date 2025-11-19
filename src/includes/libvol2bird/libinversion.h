/**
 * @file libinversion.h
 * @brief Sparse inversion utilities for weather radar data problems.
 *
 * This header declares functions for:
 *  - CSR format sparse matrix storage and multiplication
 *  - Building normal equations for least-squares inversion
 *  - Robust "effective sample size" (N_eff) calculation
 *  - Iterative fold-aware inversion for velocity components (U,V,W)
 *  - Simple inversion for reflectivity profiles (ETA = F x)
 *
 * The code uses GSL (GNU Scientific Library) for dense matrix algebra,
 * and expects F to be given in Compressed Sparse Row format for efficiency.
 */

#ifndef LIBINVERSION_H
#define LIBINVERSION_H

#include <stddef.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>


/* -------------------------------------------------------------------------- */
/* Supported regularization types                                             */
/* -------------------------------------------------------------------------- */
typedef enum {
    REG_NONE = 0,        // no regularization term
    REG_L2 = 1,          // L2 regularization / ridge regression
    REG_SMOOTHNESS = 2,  // smoothness regularization with 2nd difference matrix
    REG_MIXED = 3        // mixed L2 + smoothness regularization
} regularization_type;


/* -------------------------------------------------------------------------- */
/* Sparse CSR matrix definition and helpers                                   */
/* -------------------------------------------------------------------------- */

/**
 * @brief Sparse matrix in Compressed Sparse Row (CSR) format.
 *
 * - `row_ptr` has length nrows+1.  row_ptr[i] gives index of first nonzero of row i in `values`.
 * - `col_idx` has length nnz.  Column indices for each nonzero value.
 * - `values` has length nnz.  Nonzero values.
 */
typedef struct {
    size_t nrows;   /**< Number of rows */
    size_t ncols;   /**< Number of columns */
    size_t nnz;     /**< Number of nonzero entries */
    double *values; /**< Nonzero values */
    size_t *col_idx;/**< Column indices of nonzero entries */
    size_t *row_ptr;/**< Row pointer array (length nrows+1) */
} csr_matrix;

/* CSR allocation and free */
csr_matrix *csr_alloc(size_t nrows, size_t ncols, size_t nnz);
void csr_free(csr_matrix *mat);

/* Multiply CSR matrix by dense vector */
void csr_matvec(const csr_matrix *mat, const double *x, double *y);

/* Build csr matrix from vol2bird points array */
csr_matrix *build_F_csr(size_t nPoints,
                        double* refHeight,
                        double* range,
                        double* elev,
                        const double layerThickness,
                        size_t nLayer,
                        const double antennaHeight,
                        const double beamWidth,
                        double cutoff);

/* -------------------------------------------------------------------------- */
/* Utility: Build F^T F and compute N_eff                                     */
/* -------------------------------------------------------------------------- */

/* Compute robust N_eff = 1 / ((F^T F)^-1)_{jj} for each altitude bin */
void compute_Neff(const csr_matrix *F, double *Neff);

/* -------------------------------------------------------------------------- */
/* Normal equations for velocity inversion                                    */
/* -------------------------------------------------------------------------- */

/* Build 3m x 3m normal matrix ATA and vector ATb for velocity inversion */
void compute_normal_eqs(const csr_matrix *F,
                        const double *A1, const double *A2, const double *A3,
                        const double *VRAD_prime,
                        gsl_matrix *ATA, gsl_vector *ATb);

/* Solve dense normal equations ATA * X = ATb */
int solve_normal_eqs(gsl_matrix *ATA, gsl_vector *ATb, gsl_vector *X);

/* -------------------------------------------------------------------------- */
/* Folding update and residual statistics                                     */
/* -------------------------------------------------------------------------- */

/* Update folding counts k[i] to keep prediction within [-M3[i], M3[i]] */
void update_k(const csr_matrix *F,
              const double *A1, const double *A2, const double *A3,
              const double *U, const double *V, const double *W,
              const double *M3, int *k);

/* Compute per-measurement residuals */
void compute_residuals(const csr_matrix *F,
                       const double *A1,const double *A2,const double *A3,
                       const double *U,const double *V,const double *W,
                       const double *VRAD,double *residuals);

/* Compute stddev of residuals per altitude */
void compute_stddev_per_altitude(const csr_matrix *F,
                                 const double *residuals,
                                 double *stddev);

/* -------------------------------------------------------------------------- */
/* velocity inversion                                       */
/* -------------------------------------------------------------------------- */

/**
 * @brief Iterative fold-aware inversion for wind components.
 *
 * Inputs:
 *  - F: projection matrix (nxm)
 *  - M1: azimuth
 *  - M2: elevation angle
 *  - M3: Nyquist velocity
 *  - VRAD: Radial velocity
 *  - regularization_type: one of REG_NONE, REG_L2, REG_SMOOTHNESS
 *  - lambda: regularization strength
 *
 * Outputs:
 *  - U,V,W: velocity components per altitude bin
 *  - N: robust effective sample size per altitude bin
 *  - sigma: stddev of residuals per altitude bin
 *
 * @param vel_tol  early-stop tolerance for ||delta velocity||_inf
 * @return stop_reason: 0=k stable, 1=vel tol reached, 2=max_iter reached
 */
int radar_inversion_full_reg(const csr_matrix *F,
                         const double *M1, const double *M2,
                         const double *VRAD,
                         double *U_out, double *V_out, double *W_out,
                         double *N_out, double *sigma_out,
                         double vel_tol,
                         regularization_type regtype,
                         double lambda_L2,
                         double lambda_smoothness);


/* -------------------------------------------------------------------------- */
/* reflectivity inversion                                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Solve ETA = F x in least-squares sense for reflectivity x.
 *
 * Outputs reflectivity x, robust N_eff, residual stddev per altitude.
 */
int reflectivity_inversion_reg(const csr_matrix *F,
                           const double *ETA,
                           double *x_out,
                           double *N_out,
                           double *sigma_out,
                           regularization_type regtype,
                           double lambda_L2,
                           double lambda_smoothness);

#endif
