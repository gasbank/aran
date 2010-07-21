/*
 * SimCore.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Implicit integration core routine
 */
#include "PymPch.h"
#include "TripletMatrix.h"
#include "FiberEffectImpAll.h"
#include "MathUtil.h"
#include "ToSparse.h"
#include "Control.h"
#include "SimCore.h"
#define DEBUG
#include "DebugPrintDef.h"
#include "LCP_exp.h"
int SimCore_Python(const double h, const int nBody, const int nMuscle,
            const int nd, const int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][nd],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double *cost2, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nMuscle])
{
    //printf("Hello world %d\n", nY);
    cholmod_common c ;
    cholmod_start (&c) ;
    cholmod_sparse *W_Ysp = constructSparseDiagonalMatrix(nY, w_y, &c);
    cholmod_sparse *W_usp = constructSparseDiagonalMatrix(nMuscle, w_u, &c);
    //PrintEntireSparseMatrix(W_Ysp);
    //PrintEntireSparseMatrix(W_usp);
    int status = SimCore(h, nBody, nMuscle, nd, nY, body, extForce, muscle, musclePair, cost, cost2, ustar, Ydesired, w_y, w_u, W_Ysp, W_usp, &c);
    cholmod_free_sparse(&W_Ysp, &c);
    cholmod_free_sparse(&W_usp, &c);
    cholmod_finish(&c);
    return status;
}

int SimCore(const double h, const unsigned int nBody, const unsigned int nMuscle,
            const unsigned int nd, const unsigned int nY,
            double body[nBody][2*nd + 4], double extForce[nBody][nd],
            double muscle[nMuscle][1 + 11], unsigned int musclePair[nMuscle][2],
            double *cost, double *cost2, double ustar[nMuscle],
            double Ydesired[nY], double w_y[nY], double w_u[nMuscle],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_common *c)
{
    assert( nY == 2*nd*nBody + nMuscle );
    assert( nBody > 0 );
    assert( nd == 6 || nd == 7 );

    TripletMatrix *dfdY_R[nBody]; /* be allocated by the function 'ImpAll()' */
    TripletMatrix *dfdY_Q[nMuscle + 1]; /* be allocated by the function 'ImpAll()'. +1 added just for preventing a zero-sized array */
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
    /* Ap[n_col] or Ap[nY] is the number of nonzeros in the sparse matrix. */
    //printf("dfdY_merged nz = %d\n", Ap[nY]);

    double Control [UMFPACK_CONTROL];
    Control[UMFPACK_PRL] = 5;
    /*
    umfpack_di_report_matrix(nY, nY, Ap, Ai, Ax, 1, Control);
    */


    /*
     * Now we have f, dfdY.
     * Let's start implicit integration step.
     * Build the following matrix.
     *
     *               (identity matrix)        d f
     * tripletA  =  -------------------  -  ------
     *                      h                 d Y
     *
     *  tripletA is the matrix P in the thesis.
     */
    int tripletA_nzMax = nY + Ap[nY];
    TripletMatrix *tripletA = tm_allocate(nY, nY, tripletA_nzMax);
    for (j = 0; j < nY; ++j)
    {
        tm_add_entry(tripletA, j, j, 1./h);
    }
    int Tj[Ap[nY]];
    status = umfpack_di_col_to_triplet (nY, Ap, Tj) ;
    assert(status == UMFPACK_OK);
    /* We can access dfdY by using Ai (row indices), Tj (column indices) and Ax (numerical values). */
    /* Number of nonzero values is Ap[nY]. */
    for (j = 0; j < Ap[nY]; ++j)
    {
        tm_add_entry(tripletA, Ai[j], Tj[j], - Ax[j]);
    }
    /* tripletA is constructed. Convert it to column-compressed format. */
    int tripAp[nY + 1];
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
    //printf("tripletA nonzeros = %d\n", tripAp[nY]);

    //umfpack_di_report_matrix(nY, nY, tripAp, tripAi, tripAx, 1, Control);

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
    int n = nY;
    double *b = f;
    double x[nY];

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
        if (status == UMFPACK_WARNING_singular_matrix)
        {
            printf("    ERROR: Implicit integration matrix singular.\n");
            printf("         : What happened?\n");
            /*
            double Ctrl[UMFPACK_CONTROL];
            Ctrl[UMFPACK_PRL] = 5;
            printf("====== Matrix P =======\n");
            umfpack_di_report_matrix(nY, nY, tripAp, tripAi, tripAx, 1, Ctrl);
            printf("====== Matrix dfdY =======\n");
            umfpack_di_report_matrix(nY, nY, Ap, Ai, Ax, 1, Ctrl);
            */
            exit(-123);
        }
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
    SANITY_VECTOR(x, nY);
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
    assert(i == 2*nd*nBody); /* i start from 2*nd*nBody */
    for (k = 0; k < nMuscle; ++k, ++i)
    {
        double C_i  = x[i];
        E[i] = - Ydesired[i] + C_i;
    }
    SANITY_VECTOR(E, nY);
    //__PRINT_VECTOR(E, nY);
    assert(i == nY);
    /*
     * Calculate the sparse matrix D = P^-1 G
     * by solving the sequence of linear system
     * P di = gi. (i=1..m)
     *
     * Result: Dsp
     *
     * Dcol;
     * Transpose of D, i.e. Dcol[i] is i-th 'column' of the matrix D.
     * +1 is added just for preventing a zero-sized array.
     */
    double Dcol[nMuscle + 1][nY];
    double g[nY];
    for (k = 0; k < nMuscle; ++k)
    {
        memset(g, 0, sizeof(double) * nY);
        const double k_se = muscle[k][0 /* k_se */];
        const double b    = muscle[k][2 /*   b  */];
        g[2*nd*nBody + k] = k_se / b;
        //printf("g[%d] = %lf\n", 2*nd*nBody + k, g[2*nd*nBody + k]);
        umfpack_di_solve (UMFPACK_A, tripAp, tripAi, tripAx, Dcol[k], g, Numeric, null, null) ;
    }
    SANITY_MATRIX(Dcol, nMuscle, nY);
    cholmod_sparse *Dsp = ToSparseAndTranspose(nMuscle, nY, Dcol, c);
    cholmod_print_sparse(Dsp, "Dsp", c);
    //PrintEntireSparseMatrix(Dsp);

    double Dustar[nY];
    memset(Dustar, 0, sizeof(double)*nY);
    if (nMuscle) {
        control(nY, nMuscle, ustar, Dustar, W_Ysp, W_usp, Dsp, E, c);
    }
    SANITY_VECTOR(Dustar, nY);

    /*
     * Evaluate the cost function
     * 1) _cost  : normal cost
     * 2) _cost2 : cost if there were no control (i.e. ustar=0)
     */
    double _cost = 0, _cost2 = 0;
    for (k = 0; k < nY; ++k) {
        const double duek = Dustar[k] + E[k];
        _cost += w_y[k] * duek * duek;
        _cost2 += w_y[k] * E[k] * E[k];
    }
    for (k = 0; k < nMuscle; ++k) {
        _cost += w_u[k] * ustar[k] * ustar[k];
    }
    printf("******************************************************\n");
    printf("          Cost without control = %20lg\n", _cost2);
    printf(" <minus>  Cost with control    = %20lg\n", _cost);
    printf("                                 %20lg\n", _cost2-_cost);
    printf("******************************************************\n");
    if (_cost > _cost2) {
        /* Something goes wrong definitely... */
        printf("Exit...\n");
        exit(-9);
    }

    //printf("                     C COST   %llx\n", *((unsigned long long *)((double*)&_cost)));
    *cost  = _cost;
    *cost2 = _cost2;


    cholmod_free_sparse(&Dsp,     c);

    umfpack_di_free_numeric (&Numeric) ;
    //for (j = 0 ; j < 3 ; j++) printf ("x [%d] = %g  ", j, x [j]) ;

    /* Since x := deltaY, we should update our state vector accordingly. */
    for (k = 0; k < nBody; ++k)
    {
        /* Update p(3), q(4), pd(3), qd(4) for each body */
        for (j = 0; j < 2*nd; ++j)
        {
            body[k][j] += x[2*nd*k + j];
            body[k][j] += Dustar[2*nd*k + j];
        }
    }
    for (k = 0; k < nMuscle; ++k)
    {
        muscle[k][4 /* T ... watch out! */] += x[2*nd*nBody + k];
        muscle[k][4 /* T ... watch out! */] += Dustar[2*nd*nBody + k];
    }

    return 0; /* GOOD */
}
