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
#include <math.h>
#include "cholmod.h"

cholmod_sparse *ToSparseAndTranspose(int row, int col, double mat[row][col], cholmod_common *c)
{
    const double eps = 1e-16;
    int i, j;
    cholmod_dense *A = cholmod_allocate_dense(row, col, row, CHOLMOD_REAL, c);
    for (i = 0; i < row; ++i)
    {
        for (j = 0; j < col; ++j)
        {
            if (!isfinite(mat[i][j]) || isinf(mat[i][j]))
            {
                printf("mat[%d][%d] = %lf\n", i, j, mat[i][j]);
            }

            /* In CHOLMOD dense matrix, x_ij component of the matrix A is
             * A->x[i + row*j] not A->x[row*i + j].
             */
            if (-eps < mat[i][j] && mat[i][j] < eps)
            {
                /* If it is a very small value then truncate to 0! */
                ((double *)(A->x))[i + row*j] = 0;
            }
            else
            {
                ((double *)(A->x))[i + row*j] = mat[i][j];
            }

            /* No truncation... */
            ((double *)(A->x))[i + row*j] = mat[i][j];
        }
    }

    cholmod_sparse *Asp = cholmod_dense_to_sparse(A, 1, c);
    cholmod_sparse *ATsp = cholmod_transpose(Asp, 1, c);

//    cholmod_print_sparse(Asp, "Asp", c);
//    cholmod_print_sparse(ATsp, "ATsp", c);

    cholmod_free_sparse(&Asp, c);
    return ATsp;


//    const double eps = 1e-16;
//    int i, j;
//    cholmod_dense *A = cholmod_allocate_dense(col, row, col, CHOLMOD_REAL, c);
//    for (i = 0; i < row; ++i)
//    {
//        for (j = 0; j < col; ++j)
//        {
//            if (!isfinite(mat[i][j]) || isinf(mat[i][j]))
//            {
//                printf("mat[%d][%d] = %lf\n", i, j, mat[i][j]);
//            }
//
//            if (-eps < mat[i][j] && mat[i][j] < eps)
//            {
//                /* If it is a very small value then truncate to 0! */
//                ((double *)(A->x))[j*row + i] = 0;
//            }
//            else
//            {
//                ((double *)(A->x))[j*row + i] = mat[i][j];
//            }
//        }
//    }
//
//    cholmod_sparse *Asp = cholmod_dense_to_sparse(A, 1, c);
//    cholmod_print_sparse(Asp, "Asp", c);
//    exit(0);
//    return Asp;


    /*
    for (i = 0; i < row; ++i)
    {
        for (j = 0; j < col; ++j)
        {
            printf("%12.4lf", ((double *)(A->x))[i*row + j]);
        }
        printf("\n");
    }
    */





    /*
    cholmod_triplet *Atr = cholmod_sparse_to_triplet(Asp, c);



    for (i = 0; i < Atr->nnz; ++i)
    {
        int r = ((int*)(Atr->i))[i];
        int c = ((int*)(Atr->j))[i];
        double v = ((double*)(Atr->x))[i];
        if (!isfinite(v))
            printf("(%d, %d) %12.4lf\n", r, c, v);
    }
    cholmod_print_dense(A, "A", c);
    cholmod_print_sparse(Asp, "Asp", c);
    cholmod_print_triplet(Atr, "Atr", c);

    cholmod_free_sparse(&Asp, c);
    cholmod_free_dense(&A, c);
    cholmod_free_triplet(&Atr, c);
    cholmod_finish (c) ;
    exit(-100);
    */
}
