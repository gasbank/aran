/*
 * SimCore.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Implicit integration core routine
 */
#include <string.h>
#include <assert.h>
#include <math.h>

#include "TripletMatrix.h"
#include "FiberEffectImpAll.h"
#include "umfpack.h"
#include "cholmod.h"
#include "MathUtil.h"
#include "ToSparse.h"
#include "Control.h"
#include "SimCore.h"

int SimCore_Python(const double h, const int nBody, const int nMuscle,
            const int nd, const int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][6],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nY])
{
    //printf("Hello world %d\n", nY);
    cholmod_common c ;
    cholmod_start (&c) ;
    cholmod_sparse *Fsp = constructMatrixF(nd, nBody, nMuscle, &c);
    cholmod_sparse *W_Ysp = constructSparseDiagonalMatrix(nY, w_y, &c);
    cholmod_sparse *W_usp = constructSparseDiagonalMatrix(nY, w_u, &c);
    int status = SimCore(h, nBody, nMuscle, nd, nY, body, extForce, muscle, musclePair, cost, ustar, Ydesired, w_y, w_u, W_Ysp, W_usp, Fsp, &c);
    cholmod_free_sparse(&Fsp, &c);
    cholmod_free_sparse(&W_Ysp, &c);
    cholmod_free_sparse(&W_usp, &c);
    cholmod_finish(&c);
    return status;
}

int SimCore(const double h, const int nBody, const int nMuscle,
            const int nd, const int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][6],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nY],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_sparse *Fsp,cholmod_common *c)
{
    assert( nY == 2*nd*nBody + nMuscle );
    const unsigned int matSize = nY;

    TripletMatrix *dfdY_R[nBody]; /* be allocated by the function 'ImpAll()' */
    TripletMatrix *dfdY_Q[nMuscle]; /* be allocated by the function 'ImpAll()' */
    double f[nBody*14 + nMuscle];

    int i, j, k;
    for (j = 0; j < nBody; ++j)
    {
        /* Renormalize quaternions for each body */
        NormalizeVector(4, &body[j][3]);
    }

    ImpAll(nBody, nMuscle, dfdY_R, dfdY_Q, f, body, extForce, muscle, musclePair, h);

    /*
    for (j = 0; j < dfdY_Q[0]->nz; ++j)
    {
        printf("dfdY_Q[0] (%d, %d) = %lf\n", dfdY_Q[0]->Ti[j], dfdY_Q[0]->Tj[j], dfdY_Q[0]->Tx[j]);
    }
    */

    TripletMatrix *dfdY_RQ_merged[2] = { 0, 0 };
    dfdY_RQ_merged[0] = tm_merge(nBody, dfdY_R);
    dfdY_RQ_merged[1] = tm_merge(nMuscle, dfdY_Q);
    /* dfdY_R and dfdY_Q are unnecessary */
    for (j = 0; j < nBody; ++j)
        dfdY_R[j] = tm_free(dfdY_R[j]);
    for (j = 0; j < nMuscle; ++j)
        dfdY_Q[j] = tm_free(dfdY_Q[j]);
    TripletMatrix *dfdY_merged;
    if (nMuscle)
        dfdY_merged = tm_merge(2, dfdY_RQ_merged);
    else
        dfdY_merged = tm_merge(1, dfdY_RQ_merged);
    /* dfdY_RQ_merged is unnecessary */
    dfdY_RQ_merged[0] = tm_free(dfdY_RQ_merged[0]);
    dfdY_RQ_merged[1] = tm_free(dfdY_RQ_merged[1]);

    int Ap[dfdY_merged->n_col + 1];
    int Ai[dfdY_merged->nz];
    int status;
    int *Map = 0;
    double Ax[dfdY_merged->nz];
    status = umfpack_di_triplet_to_col (dfdY_merged->n_row,
                                        dfdY_merged->n_col,
                                        dfdY_merged->nz,
                                        dfdY_merged->Ti,
                                        dfdY_merged->Tj,
                                        dfdY_merged->Tx,
                                        Ap, Ai, Ax, Map);
    assert(status == UMFPACK_OK);
    /* Ap[n_col] or Ap[matSize] is the number of nonzeros in the sparse matrix. */
    //printf("dfdY_merged nz = %d\n", Ap[matSize]);

    double Control [UMFPACK_CONTROL];
    Control[UMFPACK_PRL] = 5;
    /*
    umfpack_di_report_matrix(matSize, matSize, Ap, Ai, Ax, 1, Control);
    */


    /*
     * Now we have f, dfdY.
     * Let's start implicit integration step.
     * Build the following matrix.
     *
     *               (identity matrix)        d f
     * tripletA  =  -------------------  -  ------
     *                      h                 d Y
     */
    int tripletA_nzMax = matSize + Ap[matSize];
    TripletMatrix *tripletA = tm_allocate(matSize, matSize, tripletA_nzMax);
    for (j = 0; j < matSize; ++j)
    {
        tm_add_entry(tripletA, j, j, 1./h);
    }
    int Tj[Ap[matSize]];
    status = umfpack_di_col_to_triplet (matSize, Ap, Tj) ;
    assert(status == UMFPACK_OK);
    /* We can access dfdY by using Ai (row indices), Tj (column indices) and Ax (numerical values). */
    /* Number of nonzero values is Ap[matSize]. */
    for (j = 0; j < Ap[matSize]; ++j)
    {
        tm_add_entry(tripletA, Ai[j], Tj[j], - Ax[j]);
    }
    /* tripletA is constructed. Convert it to column-compressed format. */
    int tripAp[matSize + 1];
    int tripAi[tripletA_nzMax];
    int *tripMap = 0;
    double tripAx[tripletA_nzMax];
    status = umfpack_di_triplet_to_col (tripletA->n_row,
                                        tripletA->n_col,
                                        tripletA->nz,
                                        tripletA->Ti,
                                        tripletA->Tj,
                                        tripletA->Tx,
                                        tripAp, tripAi, tripAx, tripMap);
    assert(status == UMFPACK_OK);
    //printf("tripletA nonzeros = %d\n", tripAp[matSize]);

    //umfpack_di_report_matrix(matSize, matSize, tripAp, tripAi, tripAx, 1, Control);

    tripletA = tm_free(tripletA);
    dfdY_merged = tm_free(dfdY_merged);

    /*
     * Linear system solving
     *
     * A x = b
     *
     * A := tripletA  ...  'P(Y^l)' matrix in the thesis
     * x := delta Y
     * b := f(Y0)     ...  'f(Y^l)' vector in the thesis
     */
    double *null = (double *) NULL ;
    void *Symbolic, *Numeric ;
    int n = matSize;
    double *b = f;
    double x[matSize];
    double DT[nMuscle][matSize]; /* Transpose of D */

    status = umfpack_di_symbolic (n, n, tripAp, tripAi, tripAx, &Symbolic, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        return -10;
    }
    status = umfpack_di_numeric (tripAp, tripAi, tripAx, Symbolic, &Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        return -20;
    }
    umfpack_di_free_symbolic (&Symbolic) ;
    /*
     * Solve for delta Y when there is no actuation forces exist.
     * The solution 'x' is the same as the vector 'C' in the thesis.
     */
    status = umfpack_di_solve (UMFPACK_A, tripAp, tripAi, tripAx, x, b, Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        return -30;
    }
    /*
     * Calculate the vector E = Y^l - Ydesired + C
     */
    double E[nY];
    for (k = 0, i = 0; k < nBody; ++k)
    {
        for (j = 0; j < 2*nd; ++j, ++i)
        {
            double Yl_i = body[k][j];
            double C_i  = x[i];
            E[i] = Yl_i - Ydesired[i] + C_i;
        }
    }
    /*
     * Calculate the sparse matrix D = P^-1 G
     * Result: Dsp
     */
    double g[matSize];
    for (k = 0; k < nMuscle; ++k)
    {
        memset(g, 0, sizeof(double) * matSize);

        double k_se = muscle[k][0 /* k_se */];
        double b    = muscle[k][2 /*   b  */];
        //g[2*nd*nBody + k] = k_se / b;
        g[2*nd*nBody + k] = 1;
        umfpack_di_solve (UMFPACK_A, tripAp, tripAi, tripAx, DT[k], g, Numeric, null, null) ;
    }

    cholmod_sparse *Dsp = ToSparseAndTranspose(nMuscle, matSize, DT, c);

    double Dustar[nY];
    control(nY, nMuscle, ustar, Dustar, W_Ysp, W_usp, Dsp, Fsp, E, c);

    printf("u* :");
    for (k = 0; k < nMuscle; ++k)
        printf("%12.4lf", ustar[k]);
    printf("\n");

    /*
     * Evaluate the cost function
     */
    double ustar_extended[nY];
    memset(ustar_extended, 0, sizeof(double) * 2*nd*nBody);
    memcpy(ustar_extended+2*nd*nBody, ustar, sizeof(double) * nMuscle);
    double _cost = 0;
    for (k = 0; k < nY; ++k)
    {
        double duek = Dustar[k] + E[k];
        _cost += w_y[k] * duek * duek;
        _cost += w_u[k] * ustar_extended[k] * ustar_extended[k];
    }
    *cost = _cost;

    cholmod_free_sparse(&Dsp, c);

    umfpack_di_free_numeric (&Numeric) ;
    //for (j = 0 ; j < 3 ; j++) printf ("x [%d] = %g  ", j, x [j]) ;

    /* Since x := deltaY, we should update our state vector accordingly. */
    for (k = 0; k < nBody; ++k)
    {
        /* Update p(3), q(4), pd(3), qd(4) for each body */
        for (j = 0; j < 14; ++j)
        {
            body[k][j] += x[k*14 + j];
            body[k][j] += Dustar[k*14 + j];
        }
    }
    for (k = 0; k < nMuscle; ++k)
    {
        muscle[k][4 /* watch out! */] += x[nBody*14 + k];
        muscle[k][4 /* watch out! */] += Dustar[nBody*14 + k];
    }
    return 0; /* GOOD */
}
