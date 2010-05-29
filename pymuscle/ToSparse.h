#ifndef TOSPARSE_H_INCLUDED
#define TOSPARSE_H_INCLUDED

cholmod_sparse *ToSparseAndTranspose(int row, int col, double mat[row][col], cholmod_common *c);
cholmod_sparse *ToSparse(int row, int col, double mat[row][col], cholmod_common *c);
cholmod_sparse *constructMatrixF(int nd, int n, int m, cholmod_common *c);

#endif // TOSPARSE_H_INCLUDED
