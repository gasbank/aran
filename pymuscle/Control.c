/*
 * Control.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * PURPOSE
 * ---------------------------------------------------------
 * 1. Construction of the matrices which are used
 *    for calculating the optimal linear actuation forces
 *
 * 2. Solve the linear system using the matrix
 *    constructed above.
 *
 * Input list
 * ---------------------------------------------------------
 * m    :  the number of muscle fibers
 * nY   :  2*n*nd + m
 * w_y  :  nY x nY diagonal matrix's diagonal terms
 * w_u  :  nY x nY diagonal matrix's diagonal terms
 * D    :  nY x  m sparse matrix
 * E    :  nY x  1 vector
 *
 * (NOTE)
 * n    :  the number of rigid bodies
 * nd   :  degree-of-freedom for a rigid body (6 or 7)
 *
 */
#include <math.h>
#include <string.h>
#include "umfpack.h"
#include "cholmod.h"
#include "Control.h"
#define DEBUG
#include "DebugPrintDef.h"
void PrintUmfStatus(int status, const char *file, int line)
{
    if (status == UMFPACK_OK)
        return;

    printf("%s(%d) : ", file, line);

    if (status == UMFPACK_WARNING_singular_matrix)
        printf("UMFPACK_WARNING_singular_matrix\n");
    else if (status == UMFPACK_ERROR_out_of_memory)
        printf("UMFPACK_ERROR_out_of_memory\n");
    else if (status == UMFPACK_ERROR_argument_missing)
        printf("UMFPACK_ERROR_argument_missing\n");
    else if (status == UMFPACK_ERROR_invalid_Symbolic_object)
        printf("UMFPACK_ERROR_invalid_Symbolic_object\n");
    else if (status == UMFPACK_ERROR_different_pattern)
        printf("UMFPACK_ERROR_different_pattern\n");
    else if (status == UMFPACK_ERROR_invalid_matrix)
        printf("UMFPACK_ERROR_invalid_matrix\n");
    else
        printf("UMF......WTF..\n");
}
int control(const unsigned int nY, const unsigned int m,
            double ustar[m], double Dustar[nY],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_sparse *Dsp, cholmod_sparse *Fsp,
            const double E[nY], cholmod_common *c)
{
    double alpha[2]     = {  1,  0 };
    double alpha_neg[2] = { -1,  0 };
    double beta[2]      = {  1,  0 };
    double beta0[2]     = {  0,  0 };

    cholmod_sparse *DTsp     = cholmod_transpose(Dsp, 1, c);
    cholmod_sparse *FTsp     = cholmod_transpose(Fsp, 1, c);
    cholmod_sparse *DTWYsp   = cholmod_ssmult(DTsp, W_Ysp, 0, 1, 1, c);
    cholmod_sparse *FTWusp   = cholmod_ssmult(FTsp, W_usp, 0, 1, 1, c);
    cholmod_sparse *DTWYDsp  = cholmod_ssmult(DTWYsp, Dsp, 0, 1, 1, c);
    cholmod_sparse *FTWuFsp  = cholmod_ssmult(FTWusp, Fsp, 0, 1, 1, c);
    cholmod_sparse *A        = cholmod_add(DTWYDsp, FTWuFsp, alpha, beta, 1, 1, c);
    cholmod_dense  *Ed       = cholmod_allocate_dense(nY, 1, nY, CHOLMOD_REAL, c);
    cholmod_dense  *bd       = cholmod_allocate_dense(m , 1, m , CHOLMOD_REAL, c);
    cholmod_dense  *ustard   = cholmod_allocate_dense(m , 1, m , CHOLMOD_REAL, c);
    cholmod_dense  *Dustard  = cholmod_allocate_dense(nY, 1, nY, CHOLMOD_REAL, c);
    memcpy(Ed->x, E, sizeof(double)*nY);
    /* Set bd */
    cholmod_sdmult(DTWYsp, 0, alpha_neg, beta0, Ed, bd, c);

    cholmod_sparse *ATsp  = 0;
    cholmod_sparse *ATAsp = 0;
    cholmod_dense  *ATbd  = 0;
//    cholmod_print_sparse(Dsp,   "D",         c);
//    cholmod_print_sparse(W_Ysp, "W_Y",       c);
//    cholmod_print_sparse(WYDsp, "W_Y * D",   c);
//    cholmod_print_sparse(WuFsp, "W_u * F",   c);
//    cholmod_print_sparse(A,     "A",         c);
//    cholmod_print_sparse(AT,    "A^T",       c);
//    cholmod_print_sparse(ATA,   "A^T * A",   c);
//    cholmod_print_sparse(ATWY,  "A^T * W_Y", c);
//    cholmod_print_dense (Ed,    "E", c);
//    cholmod_print_dense (bd,    "b", c);

    double Ctrl[UMFPACK_CONTROL];
    Ctrl[UMFPACK_PRL] = 5;

    /*
     * Solve the linear system...
     *
     * A^T * A * ustar = b
     *
     */
    double *null = (double *) NULL ;
    void *Symbolic, *Numeric ;
    double *b = (double *)(bd->x);
    int *Ap = (int *)(A->p);
    int *Ai = (int *)(A->i);
    double *Ax = (double *)(A->x);
    int status;
    status = umfpack_di_symbolic (m, m, Ap, Ai, Ax, &Symbolic, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        //return (-10) ;
    }
    status = umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        if (status == UMFPACK_WARNING_singular_matrix)
        {
            printf("Control - A in Ax=b is singular. Try least squares...\n");
            umfpack_di_report_matrix(m, m, Ap, Ai, Ax, 1, Ctrl);
            //exit(-123);




            /* Transform AT*A*x = AT*b (least squares) */
            umfpack_di_free_symbolic (&Symbolic) ;
            umfpack_di_free_numeric(&Numeric);

            ATsp  = cholmod_transpose(A, 1, c);
            ATAsp = cholmod_aat(ATsp, 0, 0, 1, c);
            ATbd  = cholmod_allocate_dense(m, 1, m, CHOLMOD_REAL, c);
            /* Set ATb */
            cholmod_sdmult(ATsp, 0, alpha, beta0, bd, ATbd, c);

            b = (double *)(ATbd->x);
            Ap = (int *)(ATAsp->p);
            Ai = (int *)(ATAsp->i);
            Ax = (double *)(ATAsp->x);

            status = umfpack_di_symbolic (m, m, Ap, Ai, Ax, &Symbolic, null, null) ;
            if (status)
            {
                V_UMF_STATUS(status);
                printf("No way out.\n");
                exit(-99);
            }
            status = umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
            if (status)
            {
                V_UMF_STATUS(status);
                printf("No way out.\n");
                exit(-88);
            }
        }

    }
    umfpack_di_free_symbolic (&Symbolic) ;
    status = umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, ustar, b, Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        //return (-30) ;
    }
    umfpack_di_free_numeric(&Numeric);
    //PRINT_VECTOR(b, m);
    //PRINT_VECTOR(ustar, m);
    SANITY_VECTOR(b, m);
    SANITY_VECTOR(ustar, m);

    /* Copy the data from 'ustar' to the cholmod_dense variable 'ustard' */
    memcpy(ustard->x, ustar, sizeof(double)*m);
    /* Calculate ||A*ustar - b||^2 to check the correctness */
    int k;
    cholmod_dense *Austard = cholmod_allocate_dense(m, 1, m, CHOLMOD_REAL, c);
    cholmod_sdmult(A, 0, alpha, beta0, ustard, Austard, c);
    double Austar_b[m]; for (k=0; k<m; ++k) Austar_b[k]=((double *)(Austard->x))[k]-b[k];
    double Austar_b_norm_sq = 0; for (k=0; k<m; ++k) Austar_b_norm_sq += Austar_b[k]*Austar_b[k];
    //printf("Austar_b_norm_sq = %14lg\n", Austar_b_norm_sq);

    /* Calculate D*ustar */
    cholmod_sdmult(Dsp, 0, alpha, beta0, ustard, Dustard, c);
    /* Copy 'Dustard' to 'Dustar' */
    memcpy(Dustar, Dustard->x, sizeof(double)*nY);

    /* DEBUG */
    //umfpack_di_report_matrix(m, m, Ap, Ai, Ax, 1, Ctrl);
    //PRINT_VECTOR(b, m);

    /*
    printf("u* :");
    for (k = 0; k < m; ++k)
        printf("%14lg", ustar[k]);
    printf("\n");
    */

    cholmod_free_sparse(&DTsp,    c);
    cholmod_free_sparse(&FTsp,    c);
    cholmod_free_sparse(&DTWYsp,  c);
    cholmod_free_sparse(&FTWusp,  c);
    cholmod_free_sparse(&DTWYDsp, c);
    cholmod_free_sparse(&FTWuFsp, c);
    cholmod_free_sparse(&A,       c);
    cholmod_free_dense (&Ed,      c);
    cholmod_free_dense (&bd,      c);
    cholmod_free_dense (&ustard,  c);
    cholmod_free_dense (&Dustard, c);

    cholmod_free_sparse(&ATsp,    c);
    cholmod_free_sparse(&ATAsp,   c);
    cholmod_free_dense (&ATbd,    c);
    return 0;
}

cholmod_sparse *constructMatrixF(int nd, int n, int m, cholmod_common *c)
{
    int row = 2*nd*n+m;
    int col = m;
    cholmod_triplet *Ftr = cholmod_allocate_triplet(row, col, m, 0, CHOLMOD_REAL, c);
    int i;
    for (i = 0; i < m; ++i)
    {
        ((int*)(Ftr->i))[i] = 2*nd*n + i;
        ((int*)(Ftr->j))[i] = i;
        ((double*)(Ftr->x))[i] = 1;
    }
    Ftr->nnz = m;
    cholmod_sparse *Fsp = cholmod_triplet_to_sparse(Ftr, m, c);
    cholmod_free_triplet(&Ftr, c);
    return Fsp;
}

cholmod_sparse *constructSparseDiagonalMatrix(int n, double diag[n], cholmod_common *c)
{
    cholmod_triplet *Atr = cholmod_allocate_triplet(n, n, n, -1 /* symmetric lower part only */, CHOLMOD_REAL, c);
    int i, nnz;
    for (i = 0, nnz = 0; i < n; ++i)
    {
        if (diag[i] != 0)
        {
            ((int*)(Atr->i))[nnz] = i;
            ((int*)(Atr->j))[nnz] = i;
            ((double*)(Atr->x))[nnz] = diag[i];
            ++nnz;
        }
    }
    Atr->nnz = nnz;
    //cholmod_print_triplet(Atr, "Atr", c);
    cholmod_sparse *Asp = cholmod_triplet_to_sparse(Atr, n, c);
    cholmod_free_triplet(&Atr, c);
    return Asp;
}
