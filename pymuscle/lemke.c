/*
 * Optimization-based rigid body simulator
 * 2010 Geoyeob Kim
 *
 * Lemke's algorithm for solving LCP
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "cholmod.h"
#include "umfpack.h"
#include "ToSparse.h"
#include "Control.h"
//#define DEBUG
#include "DebugPrintDef.h"
#include "Config.h"

#define __FILE_AND_LINE__(file,line) file "(" #line ")"
#define FILE_AND_LINE __FILE_AND_LINE__(__FILE__,__LINE__)
#define __LINESTR__(line) "(" #line ")"
#define LINESTR __LINESTR__(__LINE__)


int allNonnegative(const unsigned int n, double v[n]) {
    unsigned int i;
    for (i = 0; i < n; ++i)
        if (v[i] < 0)
            return 0;
    return 1;
}

/*
 * Given v = [v0 v1 ... vn-1],
 * Find max(-v), i.e. max([-v0 -v1 ... -vn-1]) and the index of max(-v).
 */
void SeekMaxNeg(const unsigned int n, double v[n], double *m, unsigned int *idx)
{
    *m = -v[0]; *idx = 0;
    int i;
    for (i=0;i<n;++i) {
        if (*m < -v[i]) {
            *m = -v[i];
            *idx = i;
        }
    }
}
#define SEEK_MAX(n, v, m) { int i; m = v[0]; for (i=0;i<n;++i) if (v[i] > m) m = v[i]; }

void SeekMaxSub(const unsigned int n, double v[n],
                const unsigned int k, unsigned int subv[k],
                double *m, unsigned int *idx)
{
    assert(k>=1);
    *m = v[ subv[0] ]; *idx = subv[0];
    int i;
    for (i=0;i<k;++i) {
        const unsigned int vidx = subv[i];
        assert(vidx < n);
        if (*m < v[ vidx ]) {
            *m = v[ vidx ];
            *idx = vidx;
        }
    }
}
void SeekEqualSub(const unsigned int n, double v[n],
                  const unsigned int k, unsigned int subv[k],
                  const double vsame,
                  unsigned int *r, unsigned int *lenR)
{
    int i;
    *lenR = 0;
    for (i=0;i<k;++i) {
        const unsigned int vidx = subv[i];
        assert(vidx < n);
        if (vsame == v[ vidx ]) {
            r[*lenR] = vidx;
            ++(*lenR);
        }
    }
}
/*
 * Extend an array [a0, ..., an-1] with [b0 + C, ..., bm-1 + C].
 * Returns the extended length of the array.
 * Make sure that the array 'a' should have enough space to handle it.
 */
int ConcatenateWithAdding(const unsigned int n, unsigned int a[n], const unsigned int lenA,
                          const unsigned int m, unsigned int b[m], const unsigned int lenB,
                          const unsigned int C)
{
    int i;
    for (i=lenA;i<lenA+lenB;++i) {
        assert(i < n);
        a[i] = b[i-lenA] + C;
    }
    return lenA+lenB;
}

double Dot(const unsigned int n, double a[n], double b[n])
{
    int i; double r = 0;
    for (i=0;i<n;++i) r+=a[i]*b[i];
    return r;
}

int SolveLinearSystem(cholmod_sparse *A, cholmod_dense *bd, const unsigned int m, double x[m], cholmod_common *cc)
{
    /*
     * compact form of symmetric matrix (storing lower or upper part of matrix only)
     * must be converted to normal form before passing to this function.
     */
    assert(A->stype == 0);

    double *null = (double *) NULL ;
    void *Symbolic, *Numeric ;
    double *b = (double *)(bd->x);
    int *Ap = (int *)(A->p);
    int *Ai = (int *)(A->i);
    double *Ax = (double *)(A->x);
    int status;
    status = umfpack_di_symbolic (m, m, Ap, Ai, Ax, &Symbolic, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        //return (-10) ;
    }
    status = umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        if (status == UMFPACK_WARNING_singular_matrix)
        {
            double Ctrl[UMFPACK_CONTROL];
            Ctrl[UMFPACK_PRL] = 5;
            umfpack_di_report_matrix(m, m, Ap, Ai, Ax, 1, Ctrl);
            exit(-123);
        }

    }

    /*** DEBUG ***/
//    double Ctrl[UMFPACK_CONTROL];
//    Ctrl[UMFPACK_PRL] = 5;
//    umfpack_di_report_matrix(m, m, Ap, Ai, Ax, 1, Ctrl);
    /***       ***/

    umfpack_di_free_symbolic (&Symbolic) ;
    status = umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, null, null) ;
    if (status)
    {
        V_UMF_STATUS(status);
        //return (-30) ;
    }
    umfpack_di_free_numeric(&Numeric);
    return 0;
}

int lemke_internal(const unsigned int n, double zret[n], double *M, int Mdim, double q[n], double z0[n], cholmod_common *cc) {
    assert(Mdim == 1 || Mdim == 2);

    const double zer_tol = 1e-5;
    const double piv_tol = 1e-8;
    const unsigned int maxiter = (1000<50*n)?1000:50*n;
    int err = 0;
    int i;
    /* Trivial solution exists */
    if (allNonnegative(n, q)) {
        memset(zret, 0, sizeof(double)*n);
        return 0;
    }

    /*
     * Initializations
     * (Note: all variables are initialized to their appropriate sizes)
     */
    DECLARE_ZERO_VECTOR(double      , z, 2*n); unsigned int lenZ = 20;
    DECLARE_ZERO_VECTOR(unsigned int, j,   n); unsigned int lenJ = 0;
    double theta = 0;
    double ratio = 0;
    unsigned int leaving = 0;
    cholmod_dense *Be_c = cholmod_ones(n, 1, CHOLMOD_REAL, cc);
    double *Be = (double *)(Be_c->x);
    DECLARE_COPY_VECTOR(x, n, q);
    DECLARE_ZERO_VECTOR(unsigned int, bas,    n); unsigned int lenBas = 0;
    DECLARE_ZERO_VECTOR(unsigned int, nonbas, n); unsigned int lenNonbas = 0;
    const unsigned int t = 2*n; /* Artificial variable */
    unsigned int entering = t; /* is the first entering variable */

    PRINT_VECTOR(x, n);

    /* Determine initial basis */
    if (z0 == 0)
    {
        lenBas = 0;
        for (i=0;i<n;++i) nonbas[i] = i;
        lenNonbas = n;
    }
    else
    {
        assert(!FILE_AND_LINE " : z0 should be null for now..." );
    }

    /* allocate memory for B */
    cholmod_dense *B_c = cholmod_allocate_dense(n, n, n, CHOLMOD_REAL, cc);
    double *B = (double *)(B_c->x);
    memset(B, 0, sizeof(double)*n*n);
    for (i=0;i<n;++i) B[i + n*i] = -1.;

    /* Determine initial values */
    if (lenBas != 0)
        assert(!FILE_AND_LINE " : lenBas should be 0..." );

    /* Check if initial basis provides solution */
    if (allNonnegative(n, x))
    {
        for (i=0;i<lenBas;++i) z[ bas[i] ] = x[ i ];
        memcpy(zret, z, sizeof(double)*n);
        return err;
    }

    /* Determine initial leaving variable */
    double tval; unsigned int lvindex;
    SeekMaxNeg(n, x, &tval, &lvindex);
    lenBas = ConcatenateWithAdding(n, bas, lenBas, n, nonbas, lenNonbas, n);
    assert(lenBas <= n);
    assert(lvindex <= n);
    leaving = bas[lvindex];

    bas[lvindex] = t; /* pivot in the artificial variable */

    double U[n]; for(i=0;i<n;++i) U[i]=(x[i]<0)?1.0:0.;
    memcpy(Be, U, sizeof(double)*n); /* Because Be = -dot(B, U) = -dot(-I,U) = U */
    PRINT_VECTOR(x, n);
    VECTOR_ADD(n, x, U, tval);
    PRINT_VECTOR(x, n);
    x[lvindex] = tval;
    SET_COLUMN(B, n, lvindex, Be);

    PRINT_DENSE_MATRIX(B, n, n);

    PRINT_VECTOR(x, n);
    PRINT_VECTOR(U, n);
    PRINT_DBL(tval);
    PRINT_INT(lvindex);

    /* Main iterations begin here */
    int iterr = 0;
    for (iterr=0;iterr<maxiter;++iterr) {
        //printf("#######################################################\n");
        PRINT_INT(iterr);

        /* Check if done; if not, get new entering variable */
        //printf("ITERATION %d ---- leaving %d\n", iterr, leaving);
        PRINT_VECTOR_INT(bas, lenBas);
        if (leaving == t) {
            break;
        } else if(leaving < n) {
            entering = n + leaving;
            memset(Be, 0, sizeof(double)*n);
            Be[leaving] = -1.;
        } else {
            entering = leaving - n;
            for (i=0;i<n;++i) {
                if (Mdim == 1)
                    Be[i] = M[i + n*entering];
                else
                    Be[i] = M[n*i + entering];
            }
            PRINT_INT(leaving);
            PRINT_INT(n);
            PRINT_VECTOR(Be, n);
        }

        cholmod_sparse *Bsp_c = cholmod_dense_to_sparse(B_c, 1, cc);
        double d[n];
        SolveLinearSystem(Bsp_c, Be_c, n, d, cc);
        cholmod_free_sparse(&Bsp_c, cc);
        PRINT_VECTOR(d, n);

        /* Find new leaving variable */
        lenJ = 0;
        for (i=0;i<n;++i) {
            if (d[i] > piv_tol) {
                j[lenJ] = i;
                ++lenJ;
            }
        }

        PRINT_INT(lenJ);
        PRINT_VECTOR_INT(j, lenJ);

        if (lenJ == 0)
            break;

        theta = (x[ j[0] ]+zer_tol) / d[ j[0] ];
        for (i=0;i<lenJ;++i) {
            double theta2 = (x[ j[i] ] + zer_tol) / d[ j[i] ];
            if (theta > theta2) theta = theta2;
        }
        PRINT_DBL(theta);

        PRINT_INT(lenJ);
        /* In this scope, j2 and lenJ2 are temporary variables. */
        {
            unsigned int j2[n]; unsigned int lenJ2 = 0;
            for (i=0;i<lenJ;++i) {
                if (x[ j[i] ] / d[ j[i] ] <= theta) {
                    j2[lenJ2] = j[i];
                    ++lenJ2;
                }
            }
            memcpy(j, j2, sizeof(unsigned int)*lenJ2);
            lenJ = lenJ2;
        }
        assert(lenJ > 0);

        PRINT_INT(lenJ);
        PRINT_VECTOR_INT(j, lenJ);


        /* In this scope, lvindex2 and lenLvindex2 are temporary variables. */
        {

            unsigned int lvindex2[n]; unsigned int lenLvindex2 = 0;
            for (i=0;i<lenJ;++i) {
                if (bas[ j[i] ] == t) {
                    lvindex2[lenLvindex2] = i;
                    ++lenLvindex2;
                }
            }

            PRINT_INT(lenLvindex2);
            PRINT_VECTOR_INT(lvindex2, lenLvindex2);

            assert(lenLvindex2 == 0 || lenLvindex2 == 1);
            if (lenLvindex2 == 1) {
                lvindex = j[ lvindex2[0] ];
                //PRINT_VECTOR_INT(j, lenJ);
                //PRINT_INT(lvindex2[0]);
            } else {
                SeekMaxSub(n, d, lenJ, j, &theta, &lvindex);
                //printf("~~~~~~~~~!!!!!!!!!!!!!\n");
                PRINT_INT(lenJ);
                PRINT_VECTOR_INT(j, lenJ);
                PRINT_VECTOR(d, n);
                PRINT_INT(lvindex);
                PRINT_DBL(theta);


                SeekEqualSub(n, d, lenJ, j, theta, lvindex2, &lenLvindex2);
                PRINT_INT(lenLvindex2);
                PRINT_VECTOR_INT(lvindex2, lenLvindex2);

                /*lvindex = (int)(ceil((lenLvindex2-1)*(double)rand()/RAND_MAX));*/
                assert(lenLvindex2>=1);
                lvindex = (int)(ceil((lenLvindex2-1)*0.5));
                lvindex = j[lvindex];

                //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                PRINT_INT(lenLvindex2);
                PRINT_VECTOR_INT(lvindex2, lenLvindex2);
                //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            }
        }
        leaving = bas[lvindex];
        PRINT_INT(leaving);
        PRINT_VECTOR_INT(bas, lenBas);

        /* Perform pivot */
        ratio = x[lvindex] / d[lvindex];
        VECTOR_ADD(n, x, d, -ratio);
        x[lvindex] = ratio;
        SET_COLUMN(B, n, lvindex, Be);
        bas[lvindex] = entering;
        /* end of iterations */

        PRINT_DBL(ratio);
        PRINT_VECTOR(x, n);
        PRINT_DENSE_MATRIX(B, n, n);
        PRINT_VECTOR_INT(bas, lenBas);

    }
    if (iterr >= maxiter && leaving != t)
        err = 1;
    unsigned int maxBas;
    SEEK_MAX(lenBas, bas, maxBas);
    PRINT_INT(lenBas);
    PRINT_VECTOR_INT(bas, lenBas);
    PRINT_INT(maxBas);
    PRINT_INT(lenZ);
    if (maxBas >= 2*n) {
        //assert(!FILE_AND_LINE "Untested code");
        for (i=lenZ; i<lenZ+(maxBas-lenZ+1); ++i) z[i] = 0;
        lenZ += maxBas-lenZ+1;

    }
    PRINT_INT(lenZ);

    for (i=0;i<lenBas;++i)
        z[ bas[i] ] = x[ i ];
    PRINT_VECTOR(z, lenZ);
    memcpy(zret, z, sizeof(double)*n);

    cholmod_free_dense(&B_c, cc);
    cholmod_free_dense(&Be_c, cc);
    return err;
}

int lemke(const unsigned int n, double zret[n], double M[n][n], double q[n], double z0[n], cholmod_common *cc) {
    /* The matrix M's (i,j)-element is stored in M[i][j]. */
    return lemke_internal(n, zret, (double *)M, 2, q, z0, cc);
}

int lemke_1darray(const unsigned int n, double zret[n], double M[n*n], double q[n], double z0[n], cholmod_common *cc) {
    /* The matrix M's (i,j)-element is stored in M[i + n*j]. */
    return lemke_internal(n, zret, M, 1, q, z0, cc);
}

int lemke_Python(const unsigned int n, double zret[n], double M[n][n], double q[n])
{
    cholmod_common c ;
    cholmod_start (&c) ;
    int status = lemke(n, zret, M, q, 0, &c);
    cholmod_finish(&c);
    return status;
}

int lemkeTester(cholmod_common *cc)
{
    const unsigned int n = 2;
    double M[2][2] = { {10, 0}, {7, 30} };
    double q[2] = {100, -150};
    double zret[2] = {1234,5678};
    int status = lemke(n, zret, M, q, 0, cc);
    int i,j;
    printf("ZRET : ");
    for (i=0;i<n;++i)
        printf("%10.4lf", zret[i]);
    printf("\n");

    double score = 0;
    printf("WRET : ");
    for (i=0;i<n;++i) {
        double col = 0;
        for (j=0;j<n;++j) {
            col += M[i][j]*zret[j];
        }
        col += q[i];
        score += zret[i] * col;
        printf("%10.4lf", zret[i] * col);
    }
    printf("\n");
    printf("z^T * (Mz+q) : %10.4lf\n", score);
    return status;
}
