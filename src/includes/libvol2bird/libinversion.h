#ifndef RADAR_SOLVER_H
#define RADAR_SOLVER_H

#include <stddef.h> /* for size_t */

/*
================================================================================
    Radar Wind / Reflectivity Profile Inversion Library (GSL-based)
    ---------------------------------------------------------------

    This library implements inversion algorithms for estimating vertical
    profiles of atmospheric quantities from weather radar data using
    Compressed Sparse Row (CSR) projection matrices.

    Two common inversion problems supported:
      1. Radial velocity inversion (U, V, W components) with Nyquist folding
      2. Reflectivity inversion (η profile) without folding

    Common features:
      - CSR sparse matrix format for projection weights
      - Least squares solution using precomputed G^T G normal matrix
      - Optional regularization: L2 or curvature
      - Designed for large N (observations) but small m (vertical layers)
      - Efficient: avoids forming huge dense design matrices

    Dependencies:
      - GSL (GNU Scientific Library) for linear algebra
================================================================================
*/

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
   Regularization options for inversion problems.
   ------------------------------------------------------------------------- */
typedef enum {
    REG_NONE,      /**< No regularization */
    REG_L2,        /**< Ridge regularization: λ * I */
    REG_CURVATURE  /**< Penalize curvature: λ * (D^T D), where D is second-difference */
} RegularizationType;

/* -------------------------------------------------------------------------
   Compressed Sparse Row (CSR) matrix structure for projection weights.
   F represents an nrows x ncols sparse matrix.
   - row_ptr: length nrows+1; row_ptr[i] is index in col_idx/values where row i starts
   - col_idx: column indices for each nonzero element
   - values:   numerical values for each nonzero element
   ------------------------------------------------------------------------- */
typedef struct {
    size_t nrows;     /**< number of rows (observations) */
    size_t ncols;     /**< number of columns (vertical layers) */
    size_t *row_ptr;  /**< length nrows+1; row start pointers */
    size_t *col_idx;  /**< length nnz; column index for each nonzero */
    double *values;   /**< length nnz; value for each nonzero */
} CSRMatrix;

/* -------------------------------------------------------------------------
   CSR Utility functions: building and freeing matrices.
   ------------------------------------------------------------------------- */

/**
 * @brief Allocate CSR matrix arrays.
 *
 * @param F         Pointer to CSRMatrix struct
 * @param nrows     Number of rows
 * @param ncols     Number of columns
 * @param nnz       Number of nonzero entries to allocate space for
 *
 * Allocates row_ptr (length nrows+1), col_idx (length nnz), values (length nnz).
 */
void CSR_init(CSRMatrix *F, size_t nrows, size_t ncols, size_t nnz);

/**
 * @brief Free CSR matrix arrays.
 *
 * @param F Pointer to CSRMatrix struct
 *
 * Resets struct fields to zero/null.
 */
void CSR_free(CSRMatrix *F);

/**
 * @brief Begin CSR build process.
 *
 * @param F Pointer to CSRMatrix
 *
 * Sets first entry of row_ptr[] to zero, indicating that
 * the first row’s nonzeros begin at index zero.
 */
void CSR_begin_build(CSRMatrix *F);

/**
 * @brief Add one row's nonzeros to CSR matrix.
 *
 * @param F            Pointer to CSRMatrix
 * @param row_index    Row index (0-based)
 * @param col_idx_row  Array of column indices for this row’s nonzeros
 * @param val_row      Array of values for this row’s nonzeros
 * @param row_nnz      Number of nonzeros in this row
 *
 * Writes nonzeros contiguously into col_idx[] and values[] arrays
 * at position row_ptr[row_index], sets row_ptr[row_index+1] appropriately.
 */
void CSR_add_row(CSRMatrix *F, size_t row_index,
                 const size_t *col_idx_row,
                 const double *val_row,
                 size_t row_nnz);

/**
 * @brief Finish CSR build.
 *
 * @param F Pointer to CSRMatrix
 *
 * Provided for completeness; can check consistency if desired.
 */
void CSR_finish_build(CSRMatrix *F);

/* -------------------------------------------------------------------------
   General solver for arbitrary number of "blocks" (component vectors)
   ------------------------------------------------------------------------- */

/**
 * @brief General inversion solver (velocity, reflectivity, etc.)
 *
 * Can solve for nBlocks unknown m-length profile vectors simultaneously:
 *    e.g., U,V,W (nBlocks=3), η (nBlocks=1)
 *
 * @param F             CSR projection matrix (nPoints x m)
 * @param points        Flat pseudo-matrix of input data (nPoints x nColsPoints)
 * @param nPoints       Number of observations (rows in points and F)
 * @param nColsPoints   Number of columns in points[]
 * @param dataCols      Array of column indices in points[]:
 *                       - dataCols[0] = measurement data column (e.g., VRAD or η)
 *                       - dataCols[1] = Nyquist velocity column (velocity inversion only)
 * @param factorArrays  Array of length nBlocks of pointers to geometry factor arrays:
 *                       - factorArrays[bi][row] = scaling factor for block bi at observation row
 *                       - Set pointer to NULL for factor=1 everywhere
 * @param outputs       Array of length nBlocks of pointers to double[m] arrays for solutions
 * @param k_vec         Folding counts per observation (nPoints length):
 *                       - Non-NULL => enable Nyquist folding update loop
 *                       - NULL     => disable folding (reflectivity)
 * @param m             Number of vertical layers
 * @param nBlocks       Number of profile blocks (1 for reflectivity, 3 for velocity, etc.)
 * @param max_iters     Max folding iterations (ignored if k_vec==NULL)
 * @param lambda        Regularization strength
 * @param regtype       Regularization type (REG_NONE, REG_L2, REG_CURVATURE)
 *
 * Notes:
 *  - Geometry factors and measurement data are extracted from points[] using dataCols
 *  - F is fixed during iteration; GTG is precomputed once
 *  - For velocity inversion, factorArrays contain UFactor,VFactor,WFactor
 *  - For reflectivity inversion, factorArrays=NULL and k_vec=NULL
 */
void solve_with_nyquist_reg_CSR_general(const CSRMatrix *F,
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
                                        RegularizationType regtype);

/* -------------------------------------------------------------------------
   Convenience wrapper: Velocity inversion with Nyquist folding
   ------------------------------------------------------------------------- */

/**
 * @brief Velocity inversion wrapper for estimating U, V, W profiles.
 *
 * @param F             CSR projection matrix (nPoints x m)
 * @param points        Flat pseudo-matrix of observations
 * @param nPoints       Number of observations
 * @param nColsPoints   Number of columns in points[]
 * @param colVRAD       Column index of measured radial velocity
 * @param colAzim       Column index of azimuth angle [radians]
 * @param colElev       Column index of elevation angle [radians]
 * @param colNyquist    Column index of Nyquist velocity
 * @param U             Output profile vector U (double[m])
 * @param V             Output profile vector V (double[m])
 * @param W             Output profile vector W (double[m])
 * @param k_vec         Output folding counts per observation (int[nPoints])
 * @param m             Number of vertical layers
 * @param max_iters     Max folding iterations
 * @param lambda        Regularization strength
 * @param regtype       Regularization type
 *
 * Computes geometry scaling factors from azimuth/elevation, enables
 * Nyquist folding correction loop.
 */
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
                                         RegularizationType regtype);

/* -------------------------------------------------------------------------
   Convenience wrapper: Reflectivity inversion (no folding)
   ------------------------------------------------------------------------- */

/**
 * @brief Reflectivity inversion wrapper for estimating η profile.
 *
 * @param F             CSR projection matrix (nPoints x m)
 * @param points        Flat pseudo-matrix of observations
 * @param nPoints       Number of observations
 * @param nColsPoints   Number of columns in points[]
 * @param colEta        Column index of measured reflectivity η
 * @param etaOut        Output profile vector η (double[m])
 * @param m             Number of vertical layers
 * @param lambda        Regularization strength
 * @param regtype       Regularization type
 *
 * Solves F * η ≈ measured_eta in least squares sense.
 * No geometry factors, no folding loop.
 */
void solve_reflectivity_CSR(const CSRMatrix *F,
                            const float *points,
                            size_t nPoints,
                            size_t nColsPoints,
                            size_t colEta,
                            double *etaOut,
                            size_t m,
                            double lambda,
                            RegularizationType regtype);

#ifdef __cplusplus
}
#endif

#endif /* RADAR_SOLVER_H */
