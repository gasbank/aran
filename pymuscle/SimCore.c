/*
 * SimCore.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Implicit integration core routine
 */
#include <assert.h>
#include <math.h>

#include "TripletMatrix.h"
#include "FiberEffectImpAll.h"
#include "umfpack.h"
#include "MathUtil.h"

void SimCore(const double h, const int nBody, const int nMuscle,
             double body[nBody][18], double extForce[nBody][6],
             double muscle[nMuscle][12], unsigned int musclePair[nMuscle][2])
{
    const unsigned int matSize = 14*nBody + nMuscle;

    TripletMatrix *dfdY_R[nBody]; /* be allocated by the function 'ImpAll()' */
    TripletMatrix *dfdY_Q[nMuscle]; /* be allocated by the function 'ImpAll()' */
    double f[nBody*14 + nMuscle];

    int j, k;
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
     * A := tripletA
     * x := delta Y
     * b := f(Y0)
     */
    double *null = (double *) NULL ;

    void *Symbolic, *Numeric ;
    int n = matSize;
    double *b = f;
    double x[matSize];
    (void) umfpack_di_symbolic (n, n, tripAp, tripAi, tripAx, &Symbolic, null, null) ;
    (void) umfpack_di_numeric (tripAp, tripAi, tripAx, Symbolic, &Numeric, null, null) ;
    umfpack_di_free_symbolic (&Symbolic) ;
    (void) umfpack_di_solve (UMFPACK_A, tripAp, tripAi, tripAx, x, b, Numeric, null, null) ;
    umfpack_di_free_numeric (&Numeric) ;
    //for (j = 0 ; j < 3 ; j++) printf ("x [%d] = %g  ", j, x [j]) ;

    /* Since x := deltaY, we should update our state vector accordingly. */
    for (k = 0; k < nBody; ++k)
    {
        /* Update p(3), q(4), pd(3), qd(4) for each body */
        for (j = 0; j < 14; ++j)
        {
            body[k][j] += x[k*14 + j];
        }
    }
    for (k = 0; k < nMuscle; ++k)
    {
        muscle[k][4 /* watch out! */] += x[nBody*14 + k];
    }
}
