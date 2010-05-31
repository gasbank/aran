#ifndef LEMKE_H_INCLUDED
#define LEMKE_H_INCLUDED

int lemke(const unsigned int n, double zret[n], double M[n][n], double q[n], double z0[n], cholmod_common *cc);
int lemke_1darray(const unsigned int n, double zret[n], double M[n*n], double q[n], double z0[n], cholmod_common *cc);
int lemkeTester(cholmod_common *cc);
int SolveLinearSystem(cholmod_sparse *A, cholmod_dense *bd, const unsigned int m, double x[m], cholmod_common *cc);

#endif // LEMKE_H_INCLUDED
