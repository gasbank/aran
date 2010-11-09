#ifndef _LCP_EXP_H_
#define _LCP_EXP_H_

typedef struct _LcpSubmatrices {
    cholmod_sparse *NTMinvN_sp, *DTMinvN_T_sp, *Z0_sp;
    cholmod_sparse *DTMinvN_sp, *DTMinvD_sp,   *E_sp;
    cholmod_sparse *Mu_sp,      *NegET_sp  /*, *Z0_sp  */ ;
    cholmod_dense *NThMinvtau_Qd_den, *DThMinvtau_Qd_den, *Zerop_den;

    cholmod_sparse *N_sp, *D_sp;
    cholmod_sparse *NTMinv_sp, *DTMinv_sp;
    cholmod_dense  *hMinvtau_Qd_den;
} LcpSubmatrices;

int LCP_exp_test();
void PrintEntireSparseMatrix(cholmod_sparse *M_sp);

#endif
