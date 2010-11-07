#include "PymPch.h"
#include "TripletMatrix.h"

TripletMatrix *tm_allocate(int row, int col, int maxNz)
{
    assert(maxNz >= 1);

    TripletMatrix *mat = (TripletMatrix *)malloc(sizeof(TripletMatrix));
    mat->n_row = row;
    mat->n_col = col;
    mat->nz = 0;
    mat->maxNz = maxNz;
    mat->Ti = (int *)malloc(sizeof(int)*maxNz);
    mat->Tj = (int *)malloc(sizeof(int)*maxNz);
    mat->Tx = (double *)malloc(sizeof(double)*maxNz);
    return mat;
}

TripletMatrix *tm_free(TripletMatrix *mat)
{
    if (mat)
    {
        free(mat->Ti);
        free(mat->Tj);
        free(mat->Tx);
        mat->n_row = mat->n_col = mat->nz = mat->maxNz = -1;
    }
    return 0;
}

void tm_add_entry(TripletMatrix *mat, int i, int j, double v)
{
    if (v != 0)
    {
        int nz = mat->nz;
        assert(nz < mat->maxNz);
        assert(i >= 0 && i < mat->n_row);
        assert(j >= 0 && j < mat->n_col);

        mat->Ti[nz] = i;
        mat->Tj[nz] = j;
        mat->Tx[nz] = v;
        ++mat->nz;
    }
}

TripletMatrix *tm_merge(int n, TripletMatrix **mat /*[][n]*/)
{
    assert(n>=0);
    if (n==0)
        return 0;

    int i;
    int maxNz = 0;
    for (i = 0; i < n; ++i)
    {
        maxNz += mat[i]->nz;
    }
    TripletMatrix *ret;
    int row = mat[0]->n_row;
    int col = mat[0]->n_col;
    ret = tm_allocate(row, col, maxNz);

    int *retTi = ret->Ti;
    int *retTj = ret->Tj;
    double *retTx = ret->Tx;
    for (i = 0; i < n; ++i)
    {
        assert(row == mat[i]->n_row);
        assert(col == mat[i]->n_col);

        memcpy(retTi, mat[i]->Ti, sizeof(int)*mat[i]->nz);
        memcpy(retTj, mat[i]->Tj, sizeof(int)*mat[i]->nz);
        memcpy(retTx, mat[i]->Tx, sizeof(double)*mat[i]->nz);
        retTi += mat[i]->nz;
        retTj += mat[i]->nz;
        retTx += mat[i]->nz;
    }
    ret->nz = maxNz;
    return ret;
}
