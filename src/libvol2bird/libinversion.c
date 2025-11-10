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

/* ============================================================================
 * Folding update and residual stats
 * ==========================================================================*/

/* Update folding counts */
void update_k(const csr_matrix *F,
              const double *A1, const double *A2, const double *A3,
              const double *U, const double *V, const double *W,
              const double *M3, int *k) {
    size_t n = F->nrows;
    double *tmpU = malloc(n*sizeof(double));
    double *tmpV = malloc(n*sizeof(double));
    double *tmpW = malloc(n*sizeof(double));
    csr_matvec(F, U, tmpU);
    csr_matvec(F, V, tmpV);
    csr_matvec(F, W, tmpW);
    for (size_t i=0; i<n; i++) {
        double pred = A1[i]*tmpU[i] + A2[i]*tmpV[i] + A3[i]*tmpW[i];
        if (pred > M3[i]) k[i] = (int)floor((pred+M3[i])/(2*M3[i]));
        else if (pred < -M3[i]) k[i] = (int)ceil((pred-M3[i])/(2*M3[i]));
        else k[i] = 0;
    }
    free(tmpU); free(tmpV); free(tmpW);
}

/* Residuals */
void compute_residuals(const csr_matrix *F,
                       const double *A1,const double *A2,const double *A3,
                       const double *U,const double *V,const double *W,
                       const int *k,const double *M3,
                       const double *VRAD,double *residuals) {
    size_t n = F->nrows;
    double *tmpU = malloc(n*sizeof(double));
    double *tmpV = malloc(n*sizeof(double));
    double *tmpW = malloc(n*sizeof(double));
    csr_matvec(F,U,tmpU);
    csr_matvec(F,V,tmpV);
    csr_matvec(F,W,tmpW);
    for (size_t i=0;i<n;i++) {
        double pred = A1[i]*tmpU[i]+A2[i]*tmpV[i]+A3[i]*tmpW[i]+2.0*k[i]*M3[i];
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
int radar_inversion_full_reg(const csr_matrix *F,
                         const double *M1, const double *M2,
                         const double *M3, const double *VRAD,
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
    int *k=calloc(n,sizeof(int));
    double *VRAD_prime=malloc(n*sizeof(double));
    double *U_prev=calloc(m,sizeof(double));
    double *V_prev=calloc(m,sizeof(double));
    double *W_prev=calloc(m,sizeof(double));

    int stop_reason=2;
    int max_iter=20;
    for (int iter=0;iter<max_iter;iter++) {
        vol2bird_printf("dealiasing iter=%i\n",iter);
        for (size_t i=0;i<n;i++)
            VRAD_prime[i] = VRAD[i] - 2.0*k[i]*M3[i];
        compute_normal_eqs(F,A1,A2,A3,VRAD_prime,ATA,ATb);
        add_regularization_velocity(ATA, m, regtype, lambda_L2, lambda_smoothness);
        solve_normal_eqs(ATA,ATb,X);
        for (size_t j=0;j<m;j++) {
            U_out[j]=gsl_vector_get(X,j);
            V_out[j]=gsl_vector_get(X,m+j);
            W_out[j]=gsl_vector_get(X,2*m+j);
        }
        /* vel change */
        double max_vel_change=0.0;
        for (size_t j=0;j<m;j++) {
            double dU=fabs(U_out[j]-U_prev[j]);
            double dV=fabs(V_out[j]-V_prev[j]);
            double dW=fabs(W_out[j]-W_prev[j]);
            if (dU>max_vel_change) max_vel_change=dU;
            if (dV>max_vel_change) max_vel_change=dV;
            if (dW>max_vel_change) max_vel_change=dW;
        }
        memcpy(U_prev,U_out,m*sizeof(double));
        memcpy(V_prev,V_out,m*sizeof(double));
        memcpy(W_prev,W_out,m*sizeof(double));
        /* k change */
        int changed_k=0;
        int *k_old=malloc(n*sizeof(int));
        memcpy(k_old,k,n*sizeof(int));
        update_k(F,A1,A2,A3,U_out,V_out,W_out,M3,k);
        for (size_t i=0;i<n;i++) {
            if (k[i]!=k_old[i]) changed_k=1;
        }
        free(k_old);
        if (!changed_k) {stop_reason=0; break;}
        if (max_vel_change <= vel_tol) {stop_reason=1; break;}
    }
    /* outputs */
    compute_Neff(F,N_out);
    double *residuals=malloc(n*sizeof(double));
    compute_residuals(F,A1,A2,A3,U_out,V_out,W_out,k,M3,VRAD,residuals);
    compute_stddev_per_altitude(F,residuals,sigma_out);
    free(residuals);
    /* clean */
    free(A1); free(A2); free(A3);
    free(k); free(VRAD_prime);
    free(U_prev); free(V_prev); free(W_prev);
    gsl_matrix_free(ATA); gsl_vector_free(ATb); gsl_vector_free(X);
    return stop_reason;
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
    solve_normal_eqs(ATA,ATb,X);
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
