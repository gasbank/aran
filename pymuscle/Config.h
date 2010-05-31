#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define THETA (1e-3)        /* Exp rot vector Near 0 threshold */

#define DECLARE_ZERO_VECTOR(type, valName, size) type valName[(size)]; memset(valName, 0, sizeof(type)*(size))
#define DECLARE_ONE_VECTOR(valName, size) double valName[(size)]; { int i; for(i=0;i<(size);++i) valName[i]=1.0; }
#define DECLARE_COPY_VECTOR(valName, size, srcVec) double valName[(size)]; memcpy(valName, srcVec, sizeof(double)*(size))
#define DECLARE_IDENTITY_MATRIX(valName, size, factor) double valName[size][size]; memset(valName,0,sizeof(double)*(size)*(size)); {int i; for(i=0;i<(size);++i) valName[i][i] = 1.0*factor; }
#define VECTOR_ADD(n, a, b, factor) {int i; for (i=0;i<n;++i) a[i]+=factor*b[i]; }
#define SET_COLUMN(A, n, c, b) memcpy(A+n*c, b, sizeof(double)*n)


#endif // CONFIG_H_INCLUDED
