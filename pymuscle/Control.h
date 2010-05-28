#ifndef CONTROL_H_
#define CONTROL_H_

cholmod_sparse *constructMatrixF(int nd, int n, int m, cholmod_common *c);
cholmod_sparse *constructSparseDiagonalMatrix(int n, double diag[n], cholmod_common *c);
int control(const unsigned int nY, const unsigned int m,
            double ustar[m], double Dustar[nY],
            cholmod_sparse *W_Ysp, cholmod_sparse *W_usp,
            cholmod_sparse *Dsp, cholmod_sparse *Fsp,
            double E[nY], cholmod_common *c);

void PrintUmfStatus(int status, const char *file, int line);
#define V_UMF_STATUS(status) PrintUmfStatus(status, __FILE__, __LINE__)

#endif
