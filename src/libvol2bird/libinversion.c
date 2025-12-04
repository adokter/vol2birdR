#include "libinversion.h"
#include "librender.h"
#include "libvol2bird.h" // FOR DEBUGGING, vol2bird_printf
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ============================================================================
 * CSR helpers
 * ==========================================================================*/

/* Allocate CSR matrix with given dimensions and nnz count.
 * Arrays are zeroed initially.
 */
csr_matrix *csr_alloc(size_t nrows, size_t ncols, size_t nnz) {
    csr_matrix *mat = malloc(sizeof(csr_matrix));
    mat->nrows = nrows;
    mat->ncols = ncols;
    mat->nnz   = nnz;
    mat->values  = calloc(nnz, sizeof(double));
    mat->col_idx = calloc(nnz, sizeof(size_t));
    mat->row_ptr = calloc(nrows+1, sizeof(size_t));
    return mat;
}

/* Free all arrays inside CSR matrix */
void csr_free(csr_matrix *mat) {
    if (!mat) return;
    free(mat->values);
    free(mat->col_idx);
    free(mat->row_ptr);
    free(mat);
}

/* Multiply CSR matrix (n x m) by dense vector x (length m) -> y (length n) */
void csr_matvec(const csr_matrix *mat, const double *x, double *y) {
    for (size_t i = 0; i < mat->nrows; i++) {
        double sum = 0.0;
        for (size_t jj = mat->row_ptr[i]; jj < mat->row_ptr[i+1]; jj++) {
            sum += mat->values[jj] * x[mat->col_idx[jj]];
        }
        y[i] = sum;
    }
}

/* Weight and renormalize CSR matrix (n x m) by dense vector x (length m) */
void csr_reweight(csr_matrix *mat, const double *x) {
  for (size_t i = 0; i < mat->nrows; i++) {
    double norm = 0.0;
    for (size_t jj = mat->row_ptr[i]; jj < mat->row_ptr[i+1]; jj++) {
      // apply weighting
      mat->values[jj] = mat->values[jj] * x[mat->col_idx[jj]];
      // track row sum
      norm += mat->values[jj];
    }
    // renormalize by row sum
    for (size_t jj = mat->row_ptr[i]; jj < mat->row_ptr[i+1]; jj++) {
      mat->values[jj] /= norm;
    }
  }
}


/**
 * Build the CSR matrix F from points/layers using threshold
 */
csr_matrix *build_F_csr(size_t nPoints,
                        double* refHeight,
                        double* range,
                        double* elev,
                        const double layerThickness,
                        size_t nLayer,
                        const double antennaHeight,
                        const double beamAngle,
                        double cutoff)
{
    size_t i, j;
    size_t nnz_count = 0;

    /* First pass: count nnz above cutoff */
    for (i = 0; i < nPoints; i++) {
        for (j = 0; j < nLayer; j++) {
            double height = j*layerThickness+layerThickness/2;
            double val = beamProfile(height+refHeight[i], elev[i], range[i],
                                     antennaHeight, beamAngle);
            // calculate beam_sd to account for normalization
            // cutoff is defined for a gaussian that has amplitude 1 at peak
            // so need to account for ('undo') the normalization constant:
            double beam_sd = beamWidth(range[i], beamAngle) * cos(elev[i]) / (2 * sqrt(log(2)));
            if (val > cutoff/(beam_sd*sqrt(2*PI))) {
                nnz_count++;
            }
        }
    }

    /* Allocate matrix with counted nnz */
    csr_matrix *F = csr_alloc(nPoints, nLayer, nnz_count);

    /* Second pass: fill CSR arrays */
    size_t pos = 0;
    F->row_ptr[0] = 0;

    for (i = 0; i < nPoints; i++) {
        double norm = 0;
        for (j = 0; j < nLayer; j++) {
            double height = j*layerThickness+layerThickness/2;
            double val = beamProfile(height+refHeight[i], elev[i], range[i],
                                     antennaHeight, beamAngle);
            double beam_sd = beamWidth(range[i], beamAngle) * cos(elev[i]) / (2 * sqrt(log(2)));
            if (val > cutoff/(beam_sd*sqrt(2*PI))) {
                F->values[pos] = val;
                norm += val;
                F->col_idx[pos] = j;
                pos++;
            }
            //if(i==1000){
            //  double gateheight = range2height(range[i], elev[i]);
            //  vol2bird_printf("hght = %f | gateheight = %f | range = %f | val = %f | beam_sd = %f | elev = %f | keep = %i\n", height, gateheight, range[i], val, beam_sd, elev[i], val > cutoff/(beam_sd*sqrt(2*PI)));
            //}
        }
        // renormalize
        for (j = F->row_ptr[i]; j < pos; j++) {
          F->values[j] /= norm;
        }

        F->row_ptr[i+1] = pos;  // cumulative count
    }

    vol2bird_printf("CSR matrix has nnz_count=%i out of max %i\n",pos,nLayer*nPoints);

    return F;
}


/* ============================================================================
 * Adding regularization terms
 * ==========================================================================*/

/* Add regularization to ATA in-place */
static void add_regularization(gsl_matrix *ATA, regularization_type regtype, double lambda_L2, double lambda_smoothness) {
    if (regtype == REG_NONE) return;
    size_t m = ATA->size1;
    if ((regtype == REG_L2 || regtype == REG_MIXED) && lambda_L2 > 0.0) {
        for (size_t i=0; i<m; i++)
            gsl_matrix_set(ATA, i, i, gsl_matrix_get(ATA, i, i) + lambda_L2);
    }
    if ((regtype == REG_SMOOTHNESS || regtype == REG_MIXED) && lambda_smoothness > 0.0) {
        for (size_t i=0; i<m; i++) {
            for (size_t j=0; j<m; j++) {
                double reg = 0;
                int d = (int)j - (int)i;
                if (i==j) {
                    if (i==0 || i==m-1) reg=1.0;
                    else if (i==1 || i==m-2) reg=5.0;
                    else reg=6.0;
                } else if (abs(d) == 1) {
                    if (i==0 || i==m-1 || j==0 || j==m-1) reg=-2.0;
                    else reg=-4.0;
                } else if (abs(d) == 2) {
                    reg=1.0;
                }
                if (reg != 0.0) {
                    gsl_matrix_set(ATA, i, j, gsl_matrix_get(ATA,i,j) + lambda_smoothness*reg);
                }
            }
        }
    }
}

/* For velocity, apply blockwise on 3m x 3m ATA */
static void add_regularization_velocity(gsl_matrix *ATA, size_t m,
                                        regularization_type regtype, double lambda_L2, double lambda_smoothness) {
    if (regtype == REG_NONE) return;
    for (int block=0; block<3; block++) {
        gsl_matrix_view sub = gsl_matrix_submatrix(ATA, block*m, block*m, m, m);
        add_regularization(&sub.matrix, regtype, lambda_L2, lambda_smoothness);
    }
}


/* ============================================================================
 * Compute effective sample size, simple sum over columns F
 * ==========================================================================*/
void compute_Neff(const csr_matrix *F, double *Neff) {
  size_t m = F->ncols;
  for(size_t j=0; j<m; j++) Neff[j] = 0.0;
  for (size_t i = 0; i < F->nrows; i++) {
    for (size_t jj = F->row_ptr[i]; jj < F->row_ptr[i+1]; jj++) {
      size_t col = F->col_idx[jj];
      Neff[col] += F->values[jj];
    }
  }
}

/* ============================================================================
 * Normal equations for velocity inversion
 * ==========================================================================*/

/* Construct normal equations for stacked [D1F | D2F | D3F] system */
void compute_normal_eqs(const csr_matrix *F,
                        const double *A1, const double *A2, const double *A3,
                        const double *VRAD_prime,
                        gsl_matrix *ATA, gsl_vector *ATb) {
    size_t m = F->ncols;
    gsl_matrix_set_zero(ATA);
    gsl_vector_set_zero(ATb);

    double *fi_vals = malloc(m*sizeof(double));
    size_t *fi_idx  = malloc(m*sizeof(size_t));

    for (size_t i = 0; i < F->nrows; i++) {
        double a1 = A1[i], a2 = A2[i], a3 = A3[i];
        double v  = VRAD_prime[i];
        size_t start = F->row_ptr[i];
        size_t end = F->row_ptr[i+1];
        size_t count = end - start;
        for (size_t jj = start; jj < end; jj++) {
            fi_idx[jj-start] = F->col_idx[jj];
            fi_vals[jj-start] = F->values[jj];
        }
        /* ATb contributions */
        for (size_t p=0; p<count; p++) {
            size_t col = fi_idx[p]; double fval = fi_vals[p];
            gsl_vector_set(ATb, col, gsl_vector_get(ATb, col) + a1*fval*v);
            gsl_vector_set(ATb, m+col, gsl_vector_get(ATb, m+col) + a2*fval*v);
            gsl_vector_set(ATb, 2*m+col, gsl_vector_get(ATb, 2*m+col) + a3*fval*v);
        }
        /* ATA contributions */
        for (size_t p=0; p<count; p++) {
            size_t ip = fi_idx[p]; double fv_p = fi_vals[p];
            for (size_t q=0; q<count; q++) {
                size_t iq = fi_idx[q]; double fv_q = fi_vals[q];
                /* 3x3 block */
                gsl_matrix_set(ATA, ip, iq, gsl_matrix_get(ATA, ip, iq) + a1*a1*fv_p*fv_q);
                gsl_matrix_set(ATA, ip, m+iq, gsl_matrix_get(ATA, ip, m+iq) + a1*a2*fv_p*fv_q);
                gsl_matrix_set(ATA, ip, 2*m+iq, gsl_matrix_get(ATA, ip, 2*m+iq) + a1*a3*fv_p*fv_q);
                gsl_matrix_set(ATA, m+ip, m+iq, gsl_matrix_get(ATA, m+ip, m+iq) + a2*a2*fv_p*fv_q);
                gsl_matrix_set(ATA, m+ip, 2*m+iq, gsl_matrix_get(ATA, m+ip, 2*m+iq) + a2*a3*fv_p*fv_q);
                gsl_matrix_set(ATA, 2*m+ip, 2*m+iq, gsl_matrix_get(ATA, 2*m+ip, 2*m+iq) + a3*a3*fv_p*fv_q);
            }
        }
    }
    free(fi_vals); free(fi_idx);
}

/* Solve ATA * X = ATb using LU decomposition */
int solve_normal_eqs(gsl_matrix *ATA, gsl_vector *ATb, gsl_vector *X) {
    gsl_permutation *perm = gsl_permutation_alloc(X->size);
    int signum;
    gsl_linalg_LU_decomp(ATA, perm, &signum);
    gsl_linalg_LU_solve(ATA, perm, ATb, X);
    gsl_permutation_free(perm);
    return 0;
}

/*
 Non-negative least-squares solver
 Solve: min_x 0.5 x^T Q x - c^T x   s.t. x >= 0
 Q: m×m symmetric positive definite (includes regularization)
 c: length m
 Result in x. Returns 0 for success.
 Includes:
 - Warm start from unconstrained ridge LS
 - Forced release of at least one var if first iteration has no release
 - Diagnostic printing if verbose!=0
 */
int solve_normal_eqs_nonneg_qp(
    const gsl_matrix *Q,
    const gsl_vector *c,
    gsl_vector *x,
    double tol_grad,
    double tol_zero,
    int max_iter,
    int verbose)
{
  size_t m = Q->size1;
  gsl_vector_set_zero(x);

  // --- Warm start: unconstrained solve Q x_ls = c ---
  {
    gsl_permutation *perm = gsl_permutation_alloc(m);
    gsl_matrix *Qcopy = gsl_matrix_alloc(m, m);
    gsl_matrix_memcpy(Qcopy, Q);
    gsl_vector *x_ls = gsl_vector_alloc(m);
    int signum;
    gsl_linalg_LU_decomp(Qcopy, perm, &signum);
    gsl_linalg_LU_solve(Qcopy, perm, c, x_ls);
    // Clip negatives to zero
    for (size_t i = 0; i < m; ++i) {
      double val = gsl_vector_get(x_ls, i);
      gsl_vector_set(x, i, (val > 0.0) ? val : 0.0);
    }
    gsl_vector_free(x_ls);
    gsl_permutation_free(perm);
    gsl_matrix_free(Qcopy);
    if (verbose) {
      printf("Warm start from unconstrained ridge LS.\n");
    }
  }

  // Active set mask: 1 = fixed at 0, 0 = free
  int *active = (int *) malloc(m * sizeof(int));
  for (size_t i = 0; i < m; ++i)
    active[i] = (gsl_vector_get(x, i) <= tol_zero) ? 1 : 0;

  size_t iter = 0;
  int changed = 1;

  while (changed && iter <= max_iter-1) {
    iter++;
    changed = 0;

    // Compute gradient g = Q x - c
    gsl_vector *g = gsl_vector_alloc(m);
    gsl_blas_dgemv(CblasNoTrans, 1.0, Q, x, -1.0, g);

    // Release rule
    size_t released_this_iter = 0;
    for (size_t i = 0; i < m; ++i) {
      if (active[i] && gsl_vector_get(g, i) < -tol_grad) {
        active[i] = 0;
        changed = 1;
        released_this_iter++;
      }
    }

    // Force release at least one var if first iteration has no release
    if (released_this_iter == 0 && iter == 1) {
      double min_g = 0.0;
      size_t min_idx = m;
      for (size_t i = 0; i < m; ++i) {
        if (active[i]) {
          double gi = gsl_vector_get(g, i);
          if (min_idx == m || gi < min_g) {
            min_g = gi;
            min_idx = i;
          }
        }
      }
      if (min_idx < m) {
        active[min_idx] = 0;
        changed = 1;
        released_this_iter++;
        if (verbose) {
          printf("Forced release of var %zu at iter 1 (g=%g)\n", min_idx, min_g);
        }
      }
    }

    gsl_vector_free(g);

    // Count free vars
    size_t free_count = 0;
    for (size_t i = 0; i < m; ++i)
      if (!active[i]) free_count++;

      if (verbose) {
        size_t active_count = m - free_count;
        printf("Iter %zu: free=%zu, active=%zu, released=%zu\n",
               iter, free_count, active_count, released_this_iter);
      }

      if (free_count == 0) break;

      // Map free-set indices
      size_t *fmap = (size_t *) malloc(free_count * sizeof(size_t));
      {
        size_t fi = 0;
        for (size_t i = 0; i < m; ++i)
          if (!active[i]) fmap[fi++] = i;
      }

      // Build Qf / cf for free set
      gsl_matrix *Qf = gsl_matrix_alloc(free_count, free_count);
      gsl_vector *cf = gsl_vector_alloc(free_count);

      for (size_t i = 0; i < free_count; ++i) {
        gsl_vector_set(cf, i, gsl_vector_get(c, fmap[i]));
        for (size_t j = 0; j < free_count; ++j) {
          gsl_matrix_set(Qf, i, j, gsl_matrix_get(Q, fmap[i], fmap[j]));
        }
      }

      // Solve Qf * xf = cf
      gsl_permutation *perm = gsl_permutation_alloc(free_count);
      int signum;
      gsl_linalg_LU_decomp(Qf, perm, &signum);
      gsl_vector *xf = gsl_vector_alloc(free_count);
      gsl_linalg_LU_solve(Qf, perm, cf, xf);
      gsl_permutation_free(perm);

      // Assign back, clamp negatives
      for (size_t i = 0; i < free_count; ++i) {
        double val = gsl_vector_get(xf, i);
        if (val <= tol_zero) {
          gsl_vector_set(x, fmap[i], 0.0);
          active[fmap[i]] = 1;
          changed = 1;
        } else {
          gsl_vector_set(x, fmap[i], val);
        }
      }

      gsl_vector_free(xf);
      gsl_matrix_free(Qf);
      gsl_vector_free(cf);
      free(fmap);

      if (verbose && !changed) {
        printf("No changes in iteration %zu -> stopping.\n", iter);
      }
  }

  free(active);

  if(iter > max_iter){
    if (verbose) {
      printf("Failed to converge in %zu iterations.\n", iter);
    }
    return 1;
  }

  if (verbose) {
    printf("Converged in %zu iterations.\n", iter);
  }
  return 0;
}

/* ============================================================================
 * Folding update and residual stats
 * ==========================================================================*/

/* Residuals */
void compute_residuals(const csr_matrix *F,
                       const double *A1,const double *A2,const double *A3,
                       const double *U,const double *V,const double *W,
                       const double *VRAD,double *residuals) {
    size_t n = F->nrows;
    double *tmpU = malloc(n*sizeof(double));
    double *tmpV = malloc(n*sizeof(double));
    double *tmpW = malloc(n*sizeof(double));
    csr_matvec(F,U,tmpU);
    csr_matvec(F,V,tmpV);
    csr_matvec(F,W,tmpW);
    for (size_t i=0;i<n;i++) {
        double pred = A1[i]*tmpU[i]+A2[i]*tmpV[i]+A3[i]*tmpW[i];
        residuals[i] = VRAD[i] - pred;
    }
    free(tmpU); free(tmpV); free(tmpW);
}

/* Stddev per altitude bin */
void compute_stddev_per_altitude(const csr_matrix *F,
                                 const double *residuals,
                                 double *stddev) {
    size_t m = F->ncols;
    double *sum = calloc(m,sizeof(double));
    double *sum_sq = calloc(m,sizeof(double));
    size_t *count = calloc(m,sizeof(size_t));
    for (size_t i=0;i<F->nrows;i++) {
        for (size_t jj=F->row_ptr[i]; jj<F->row_ptr[i+1]; jj++) {
            size_t col=F->col_idx[jj];
            sum[col]+=residuals[i];
            sum_sq[col]+=residuals[i]*residuals[i];
            count[col]++;
        }
    }
    for (size_t j=0;j<m;j++) {
        if (count[j]>1) {
            double mean = sum[j]/count[j];
            stddev[j] = sqrt((sum_sq[j]/count[j]) - mean*mean);
        } else stddev[j] = NAN;
    }
    free(sum); free(sum_sq); free(count);
}

/* ============================================================================
 * High-level velocity inversion driver
 * ==========================================================================*/
int radar_inversion_full_reg(csr_matrix *F,
                         const double *M1, const double *M2,
                         const double *VRAD, const double *Z,
                         double *U_out, double *V_out, double *W_out,
                         double *N_out, double *sigma_out,
                         double vel_tol,
                         regularization_type regtype,
                         double lambda_L2,
                         double lambda_smoothness)
{
    size_t n=F->nrows, m=F->ncols;
    double *A1=malloc(n*sizeof(double)), *A2=malloc(n*sizeof(double)), *A3=malloc(n*sizeof(double));
    for (size_t i=0;i<n;i++) {
        A1[i]=sin(M1[i])*cos(M2[i]);
        A2[i]=cos(M1[i])*cos(M2[i]);
        A3[i]=sin(M2[i]);
    }
    gsl_matrix *ATA=gsl_matrix_alloc(3*m,3*m);
    gsl_vector *ATb=gsl_vector_alloc(3*m);
    gsl_vector *X=gsl_vector_alloc(3*m);

    // compute effective sample size on original F-matrix
    compute_Neff(F,N_out);

    // re-weight F-matrix by the inverse solution to the reflectivity,
    // effectively a weighted averaged of speed by reflectivity.
    csr_reweight(F, Z);

    compute_normal_eqs(F,A1,A2,A3,VRAD,ATA,ATb);
    add_regularization_velocity(ATA, m, regtype, lambda_L2, lambda_smoothness);
    solve_normal_eqs(ATA,ATb,X);
    for (size_t j=0;j<m;j++) {
        U_out[j]=gsl_vector_get(X,j);
        V_out[j]=gsl_vector_get(X,m+j);
        W_out[j]=gsl_vector_get(X,2*m+j);
    }

    /* outputs */
    double *residuals=malloc(n*sizeof(double));
    compute_residuals(F,A1,A2,A3,U_out,V_out,W_out,VRAD,residuals);
    compute_stddev_per_altitude(F,residuals,sigma_out);
    free(residuals);
    /* clean */
    free(A1); free(A2); free(A3);
    gsl_matrix_free(ATA); gsl_vector_free(ATb); gsl_vector_free(X);
    return 0;
}

/* ============================================================================
 * Simple reflectivity inversion
 * ==========================================================================*/
static void compute_normal_eqs_simple(const csr_matrix *F,
                                      const double *ETA,
                                      gsl_matrix *ATA,
                                      gsl_vector *ATb) {
    gsl_matrix_set_zero(ATA);
    gsl_vector_set_zero(ATb);
    for (size_t i=0;i<F->nrows;i++) {
        size_t start=F->row_ptr[i];
        size_t end=F->row_ptr[i+1];
        for (size_t p=start; p<end; p++) {
            size_t cp=F->col_idx[p];
            double vp=F->values[p];
            gsl_vector_set(ATb, cp, gsl_vector_get(ATb,cp) + vp*ETA[i]);
            for (size_t q=start;q<end;q++) {
                size_t cq=F->col_idx[q];
                double vq=F->values[q];
                double old=gsl_matrix_get(ATA,cp,cq);
                gsl_matrix_set(ATA,cp,cq, old + vp*vq);
            }
        }
    }
}

/* Main wrapper for reflectivity inversion */
int reflectivity_inversion_reg(const csr_matrix *F,
                           const double *ETA,
                           double *x_out,
                           double *N_out,
                           double *sigma_out,
                           regularization_type regtype,
                           double lambda_L2,
                           double lambda_smoothness)
{
    size_t m=F->ncols, n=F->nrows;
    gsl_matrix *ATA=gsl_matrix_alloc(m,m);
    gsl_vector *ATb=gsl_vector_alloc(m);
    gsl_vector *X=gsl_vector_alloc(m);
    compute_normal_eqs_simple(F,ETA,ATA,ATb);
    add_regularization(ATA, regtype, lambda_L2, lambda_smoothness);
    solve_normal_eqs_nonneg_qp(ATA,ATb,X, 1e-10, 1e-14, 2, 1);
    //solve_normal_eqs(ATA,ATb,X);
    for (size_t j=0;j<m;j++) x_out[j]=gsl_vector_get(X,j);
    compute_Neff(F,N_out);
    double *residuals=malloc(n*sizeof(double));
    csr_matvec(F,x_out,residuals);
    for (size_t i=0;i<n;i++)
        residuals[i]=ETA[i]-residuals[i];
    compute_stddev_per_altitude(F,residuals,sigma_out);
    free(residuals);
    gsl_matrix_free(ATA); gsl_vector_free(ATb); gsl_vector_free(X);
    return 0;
}
