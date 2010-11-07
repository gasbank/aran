/*
 * ToSparse.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * PURPOSE
 * ---------------------------------------------------------
 * Make a dense matrix to a sparse form
 *
 */
#include "PymOptimizerPch.h"
#include "ToSparse.h"
#include "DebugPrintDef.h"
cholmod_sparse *ToSparseAndTranspose(int row, int col, double **mat /*[row][col]*/, cholmod_common *c)
{
    cholmod_sparse *Asp = ToSparse(row, col, mat, c);
    cholmod_sparse *ATsp = cholmod_transpose(Asp, 1, c);
    cholmod_free_sparse(&Asp, c);
    return ATsp;
}

cholmod_sparse *ToSparse(int row, int col, double **mat /*[row][col]*/, cholmod_common *c)
{
    int i, j;
    cholmod_dense *A = cholmod_allocate_dense(row, col, row, CHOLMOD_REAL, c);
    //SANITY_MATRIX(mat, row, col);
    for (i = 0; i < row; ++i)
    {
        for (j = 0; j < col; ++j)
        {
            /*
             * In CHOLMOD dense matrix, x_ij component (i-th row and j-th col)
             * of the matrix A is A->x[i + row*j] not A->x[row*i + j].
             */

//            const double eps = 1e-16;
//            if (-eps < mat[i][j] && mat[i][j] < eps)
//            {
//                /* If it is a very small value then truncate to 0! */
//                ((double *)(A->x))[i + row*j] = 0;
//            }
//            else
//            {
//                ((double *)(A->x))[i + row*j] = mat[i][j];
//            }

            /* NOTE:  No truncation... */
            ((double *)(A->x))[i + row*j] = mat[i][j];
        }
    }

    cholmod_sparse *Asp = cholmod_dense_to_sparse(A, 1, c);
    return Asp;
}
