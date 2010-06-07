#ifndef DEBUGPRINTDEF_H_INCLUDED
#define DEBUGPRINTDEF_H_INCLUDED

#if defined(DEBUG)
    #define PRINT_FLH printf("%s (%d) : ", strrchr(__FILE__, '/')+1, __LINE__)

    #define PRINT_DENSE_VECTOR(vd) { PRINT_FLH;  double* v = (double *)(vd->x); int n = vd->nrow; printf("DenseVector " #vd); int i; for(i=0;i<n;++i) printf("%10.4lf", v[i]); printf("\n"); }
    #define PRINT_VECTOR(v, n)     { PRINT_FLH;  printf("Vector " #v " : "); int i; for(i=0;i<n;++i) printf("%14lg", v[i]); printf("\n"); }
    #define PRINT_VECTOR_INT(v, n) { PRINT_FLH;  printf("Vector " #v " : "); int i; for(i=0;i<n;++i) printf("%3d", v[i]); printf("\n"); }
    #define PRINT_INT(v)             PRINT_FLH;  printf(#v " : %d\n", v)
    #define PRINT_DBL(v)             PRINT_FLH;  printf(#v " : %lg\n", v)
    #define PRINT_MATRIX(M,r,c)    { PRINT_FLH;  printf("Matrix " #M " : \n"); int i,j; for(i=0;i<r;++i) { for(j=0;j<c;++j) printf("%10.4lf", M[i][j]); printf("\n"); } }
    #define PRINT_DENSE_MATRIX(M,r,c) \
        { \
            PRINT_FLH; \
            printf("DMatrix " #M " : \n"); \
            int i,j; \
            for(i=0;i<r;++i) { \
                for(j=0;j<c;++j) \
                    printf("%10.4lf", M[i + c*j]); \
                printf("\n"); \
            } \
        }
    #define SPARSE_CHECK(x) cholmod_print_sparse(x, #x, cc)
    #define DENSE_CHECK(x)  cholmod_print_dense(x, #x, cc)
    #define TRIPLET_CHECK(x)  cholmod_print_triplet(x, #x, cc)
    #define CHECK_SOURCE_LINE { PRINT_FLH;  printf("CHECK_SOURCE_LINE\n"); }

#else
    #define PRINT_FLH
    #define PRINT_DENSE_VECTOR(vd)
    #define PRINT_VECTOR(v,n)
    #define PRINT_VECTOR_INT(v,n)
    #define PRINT_INT(v)
    #define PRINT_DBL(v)
    #define PRINT_MATRIX(M, r, c)
    #define PRINT_DENSE_MATRIX(M,r,c)
    #define SPARSE_CHECK(x)
    #define DENSE_CHECK(x)
    #define TRIPLET_CHECK(x)
    #define CHECK_SOURCE_LINE
#endif


#if !defined(NOSANITYCHECK)
    #define SANITY_VECTOR(v, n) {\
        int i; for (i=0;i<n;++i) {\
            if (isinf(v[i]) || isnan(v[i])) {\
                printf(" ??? NAN or INFINITE ??? : ");\
                PRINT_VECTOR(v, n);\
                exit(-1985);\
            }\
        }\
    }
#else
    #define SANITY_VECTOR(v,n)
#endif

#endif // DEBUGPRINTDEF_H_INCLUDED
