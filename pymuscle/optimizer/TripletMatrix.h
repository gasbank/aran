#ifndef TRIPLETMATRIX_H_INCLUDED
#define TRIPLETMATRIX_H_INCLUDED

typedef struct _TripletMatrix
{
    int maxNz;
    int n_row, n_col, nz, *Ti, *Tj;
    double *Tx;
} TripletMatrix;

TripletMatrix *tm_allocate(int row, int col, int maxNz);
TripletMatrix *tm_free(TripletMatrix *mat);
TripletMatrix *tm_merge(int n, TripletMatrix **mat /*[][n]*/);
void tm_add_entry(TripletMatrix *mat, int i, int j, double v);
void tm_to_umfsparse(TripletMatrix *mat);

#endif // TRIPLETMATRIX_H_INCLUDED
