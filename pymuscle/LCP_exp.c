/*
 * LCP_exp.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Linear complementarity problem for calculating contact forces
 * of multibody and multicontact cases
 *
 * Rotation is parameterized by an exponential map technique
 */
#include <string.h>
#include <assert.h>
#include <math.h>
#include <umfpack.h>
#include <cholmod.h>
#include "ExpBodyMoEq_real.h"
#include "dRdv_real.h"
#include "Control.h"
#include "LCP_exp.h"
#include "MathUtil.h"
#include "lemke.h"
#include "SymbolicTensor.h"
//#define DEBUG
#include "DebugPrintDef.h"


const double       ZERO3[3]   = {0,0,0};
const double       NORMAL3[3] = {0,0,1};
/* For sparse-dense multiplication of cholmod package */
const double alpha[2]  =  {  1,  0 };
const double beta[2]    = {  1,  0 };
const double beta0[2]    = {  0,  0 };

void PrintEntireSparseMatrix(cholmod_sparse *M_sp) {
    double Ctrl[UMFPACK_CONTROL];
    Ctrl[UMFPACK_PRL] = 5;
    umfpack_di_report_matrix(M_sp->nrow, M_sp->ncol, M_sp->p, M_sp->i, M_sp->x, 1, Ctrl);
}

void Invert6x6MassMatrix(double Minv[6][6], const double M[6][6])
{
    /*
     *        M                     M^-1 (Minv)
     *
     *   /         \          /              \
     *   |  mI   0 |          |  I/m    0    |
     *   |         |    ===>  |              |
     *   |  0    H |          |   0     H^-1 |
     *   \         /          \              /
     *
     */
    memset(Minv, 0, sizeof(double)*6*6);
    assert(M[0][0] == M[1][1]);
    assert(M[1][1] == M[2][2]);
    Minv[0][0] = 1./M[0][0];
    Minv[1][1] = 1./M[1][1];
    Minv[2][2] = 1./M[2][2];
    double determinant =+M[3][3]*(M[4][4]*M[5][5]-M[5][4]*M[4][5])
                        -M[3][4]*(M[4][3]*M[5][5]-M[4][5]*M[5][3])
                        +M[3][5]*(M[4][3]*M[5][4]-M[4][4]*M[5][3]);
    double invdet = 1./determinant;
    Minv[3][3] =  (M[4][4]*M[5][5]-M[5][4]*M[4][5])*invdet;
    Minv[4][3] = -(M[3][4]*M[5][5]-M[3][5]*M[5][4])*invdet;
    Minv[5][3] =  (M[3][4]*M[4][5]-M[3][5]*M[4][4])*invdet;
    Minv[3][4] = -(M[4][3]*M[5][5]-M[4][5]*M[5][3])*invdet;
    Minv[4][4] =  (M[3][3]*M[5][5]-M[3][5]*M[5][3])*invdet;
    Minv[5][4] = -(M[3][3]*M[4][5]-M[4][3]*M[3][5])*invdet;
    Minv[3][5] =  (M[4][3]*M[5][4]-M[5][3]*M[4][4])*invdet;
    Minv[4][5] = -(M[3][3]*M[5][4]-M[5][3]*M[3][4])*invdet;
    Minv[5][5] =  (M[3][3]*M[4][4]-M[4][3]*M[3][4])*invdet;
}

unsigned int Index(const unsigned int len, const unsigned array[len], unsigned int v)
{
    int i;
    for (i=0; i<len; ++i) {
        if (array[i] == v)
            return i;
    }
    return -1;
}

#define SET_TRIPLET_RCV(choltrip, _r, _c, _v) \
{\
    unsigned int *r = (unsigned int *)(choltrip->i) + choltrip->nnz; \
    unsigned int *c = (unsigned int *)(choltrip->j) + choltrip->nnz; \
    double       *v = (double       *)(choltrip->x) + choltrip->nnz; \
    *r = (_r); \
    *c = (_c); \
    *v = (_v); \
    ++choltrip->nnz; \
}

/*
 * Build the equations of motion coefficients considering no contact and no external force
 */
int BuildEquationsOfMotionCoefficients(cholmod_sparse **M_sp, cholmod_sparse **Minv_sp, cholmod_dense **_tau_den,
                        const unsigned int nd, const unsigned int n, const unsigned int m,
                        const unsigned int lenBodies, const unsigned int bodies[lenBodies],
                        const double Y [2*nd*n + m], const double extForces[n][nd],
                        const double I[n][4], const double mass[n],
                        cholmod_common *cc) {
    assert (*M_sp    == 0);
    assert (*_tau_den == 0);
    if (Minv_sp) {
        assert (*Minv_sp == 0);
    }
    /*
     *   Original form    :  M * qdd + Cqd  = fg
     *   Implemented form :  M * qdd        = tau
     *                                        tau = fg - Cqd
     */
    const unsigned int M_trip_maxnz = lenBodies*((nd-3)*(nd-3)+3);
    const unsigned int Minv_trip_maxnz = M_trip_maxnz;
    cholmod_triplet *M_trip = cholmod_allocate_triplet(lenBodies*nd,
                                                       lenBodies*nd,
                                                       M_trip_maxnz,
                                                       -1, /* symmetric: lower part only */
                                                       CHOLMOD_REAL, cc);
    cholmod_triplet *Minv_trip = 0;
    if (Minv_sp) {
        Minv_trip = cholmod_allocate_triplet(lenBodies*nd,
                                             lenBodies*nd,
                                             Minv_trip_maxnz,
                                             -1, /* symmetric: lower part only */
                                             CHOLMOD_REAL, cc);
    }
    cholmod_dense   *tau_den   = cholmod_allocate_dense(lenBodies*nd, 1, lenBodies*nd, CHOLMOD_REAL, cc);
    //memset(tau_a_den->x, 0, sizeof(double)*lenBodies*nd);
    int i, j, k;
    for (i=0; i<lenBodies; ++i) {
        /*
         *
         *     DO NOT USE 'i' as GLOBAL BODY INDEX ! ! !
         *     USE 'bodyidx' instead ! ! !
         *
         *     USE 'i' at LOCAL BODY INDEX ! ! !
         *
         */
        double M[nd][nd], Cqd[nd];
        const unsigned int bodyidx = bodies[i]; /* THIS IS GLOBAL BODY INDEX */
        const double *pi  = &Y[ bodyidx*2*nd + 0 ];
        const double *vi  = &Y[ bodyidx*2*nd + 3 ];
        const double *pdi = &Y[ bodyidx*2*nd + 6 ];
        const double *vdi = &Y[ bodyidx*2*nd + 9 ];
        const double *Ii  =  I[ bodyidx ];
        PRINT_VECTOR(pi, 3);
        PRINT_VECTOR(vi, 3);
        PRINT_VECTOR(pdi, 3);
        PRINT_VECTOR(vdi, 3);
        PRINT_VECTOR(Ii, 4);
        MassMatrixAndCqdVector(M, Cqd, pi, vi, pdi, vdi, Ii);
        PRINT_MATRIX(M, nd, nd);
        PRINT_VECTOR(Cqd, nd);
        double Minv[nd][nd];
        if (Minv_sp)
            Invert6x6MassMatrix(Minv, M);

        double *tau = (double *)(tau_den->x);
        /*
         * We treat the gravitational forces as external forces.
         * There are no separate term for the grav force.
         *
         * If you want the grav force is computed and added
         * in C side then use the following code snippet.
         *
         * const double gravForce[3] = { 0, 0, -9.81 * mass[bodyidx] };
         * GeneralizedForce(&tau[i*nd], vi, gravForce, ZERO3);
         */
        memcpy(tau + i*nd, extForces[bodyidx], sizeof(double)*nd);

        for (j=0; j<nd; ++j) {
            for (k=0; k<=j; ++k) { /* symmetric: lower part only */
                if (M[j][k] != 0) {
                    SET_TRIPLET_RCV(M_trip, i*nd + j, i*nd + k, M[j][k]);
                }
                if (Minv_sp && Minv[j][k] != 0) {
                    SET_TRIPLET_RCV(Minv_trip, i*nd + j, i*nd + k, Minv[j][k]);
                }
            }

            if (Cqd[j] != 0) {
                tau[ i*nd + j ] += -Cqd[j];
            }
        }

        PRINT_VECTOR(tau, nd);
    }
    assert(M_trip->nnz    <= M_trip_maxnz   );
    if (Minv_sp) {
        assert(Minv_trip->nnz <= Minv_trip_maxnz);
    }

    *M_sp        = cholmod_triplet_to_sparse(M_trip,    M_trip->nnz,    cc);
    //SPARSE_CHECK(*M_sp);
    if (Minv_sp)
        *Minv_sp = cholmod_triplet_to_sparse(Minv_trip, Minv_trip->nnz, cc);
    *_tau_den    = tau_den; /* RETURN THIS -- do not deallocate */

    cholmod_free_triplet(&M_trip, cc);
    if (Minv_sp)
        cholmod_free_triplet(&Minv_trip, cc);
    return 0;
}

int DetermineActiveness(const unsigned int nd, const unsigned int n, const unsigned int m,
                        unsigned int activeCorners[8*n][2], unsigned int *_lenActiveCorners,
                        unsigned int activeBodies[n],       unsigned int *_lenActiveBodies,
                        unsigned int inactiveBodies[n],     unsigned int *_lenInactiveBodies,
                        const double Y [2*nd*n + m], const double corners[n][8][3], const double penetration0) {
    unsigned int i, j;
    unsigned int lenActiveCorners  = 0;
    unsigned int lenActiveBodies   = 0;
    unsigned int lenInactiveBodies = 0;
    for (i=0; i<n; ++i) {
        int isActive = 0;
        for (j=0; j<8; ++j) {
            /* Calculate the i-th corner position in world coordinates */
            double q[4]; /* quaternion */
            double corners_wc[3];
            VtoQuat(q, &Y[2*nd*i + 3]); /* &Y[2*nd*i + 3] := start address of rotation vector */
            quat_rot(corners_wc, q, corners[i][j]);
            corners_wc[0] += Y[2*nd*i + 0];
            corners_wc[1] += Y[2*nd*i + 1];
            corners_wc[2] += Y[2*nd*i + 2];
            const double z_wc = corners_wc[2];
            if (z_wc <= penetration0) {
                activeCorners[lenActiveCorners][0] = i; /* body index */
                activeCorners[lenActiveCorners][1] = j; /* corner index */
                ++lenActiveCorners;
                isActive = 1;
            }
        }
        if (isActive == 0) {
            inactiveBodies[lenInactiveBodies] = i;
            ++lenInactiveBodies;
        } else {
            activeBodies[lenActiveBodies] = i;
            ++lenActiveBodies;
        }
    }
    *_lenActiveCorners  = lenActiveCorners;
    *_lenActiveBodies   = lenActiveBodies;
    *_lenInactiveBodies = lenInactiveBodies;
    return 0;
}



int BuildLCPSubmatrices(LcpSubmatrices *lcpSm,
                        const unsigned int nd, const unsigned int n, const unsigned int m,
                        const double Y [2*nd*n + m], const double I[n][4], const double mass[n],
                        const double corners[n][8][3],
                        const unsigned int NCONEBASIS, const double CONEBASIS[NCONEBASIS][3],
                        const double mu, const double h,
                        unsigned int lenActiveCorners,  unsigned int activeCorners [lenActiveCorners][2],
                        unsigned int lenActiveBodies,   unsigned int activeBodies  [lenActiveBodies],
                        cholmod_sparse *M_a_sp,
                        cholmod_sparse *Minv_a_sp,
                        cholmod_dense  *tau_a_den,
                        cholmod_common *cc) {
    assert(nd == 6);
    unsigned int i, j, k;

    /*
     *  Structure of the matrix N
     *
     *    lenActiveCorners  ==  the numer of contact points
     *  /                 \
     *  |  *              |
     *  |  *              |
     *  |  *              |
     *  | ................|
     *  |      *   *      |
     *  |      *   *      |  lenActiveBodies * nd
     *  |      *   *      |
     *  |.................|
     *  |                 |
     *  |             *   |        *
     *  |             *   |        *  --> 6x1 vector
     *  |             *   |        *
     *  \                 /
     */
    const unsigned int N_trip_maxnz = nd * lenActiveCorners;
    cholmod_triplet *N_trip = 0;
    const unsigned int D_trip_maxnz = NCONEBASIS * nd * lenActiveCorners;
    cholmod_triplet *D_trip = 0;
    if (lenActiveCorners) {
        N_trip = cholmod_allocate_triplet(lenActiveBodies*nd,
                                          lenActiveCorners,
                                          N_trip_maxnz,
                                          0, /* unsymmetric */
                                          CHOLMOD_REAL, cc);
        D_trip = cholmod_allocate_triplet(lenActiveBodies*nd,
                                          NCONEBASIS * lenActiveCorners,
                                          D_trip_maxnz,
                                          0, /* unsymmetric */
                                          CHOLMOD_REAL, cc);
        //TRIPLET_CHECK(N_trip);
        //TRIPLET_CHECK(D_trip);
    }
    for (i=0; i<lenActiveCorners; ++i) {
        const unsigned int bodyidx   = activeCorners[i][0];
        const unsigned int corneridx = activeCorners[i][1];
        /* local body index among active bodies */
        const unsigned int kp        = Index(lenActiveBodies, activeBodies, bodyidx);
        const double *vi     = &Y[ bodyidx*2*nd + 3 ];
        const double *corner = corners[bodyidx][corneridx];
        double Q[6]; /* Generalized force; mutable */

        /* Construct the matrix N */
        GeneralizedForce(Q, vi, NORMAL3, corner);
        for (j=0; j<nd; ++j) {
            if (Q[j] != 0) {
                SET_TRIPLET_RCV(N_trip, nd*kp + j, i, Q[j]);
            }
        }
        assert(N_trip->nnz <= N_trip_maxnz);

        /* Construct the matrix D */
        for (k=0; k<NCONEBASIS; ++k) {
            GeneralizedForce(Q, vi, CONEBASIS[k], corner);
            for (j=0; j<nd; ++j) {
                if (Q[j] != 0) {
                    SET_TRIPLET_RCV(D_trip, nd*kp + j, NCONEBASIS*i + k, Q[j]);
                }
            }
        }
        assert(D_trip->nnz <= D_trip_maxnz);
    }
    assert(N_trip->nnz <= N_trip_maxnz);
    assert(D_trip->nnz <= D_trip_maxnz);


    //TRIPLET_CHECK(N_trip);
    //TRIPLET_CHECK(D_trip);

    /* Construct the matrix E */
    cholmod_triplet *E_trip = cholmod_allocate_triplet(lenActiveCorners*NCONEBASIS,
                                                       lenActiveCorners,
                                                       lenActiveCorners*NCONEBASIS,
                                                       0, /* unsymmetric */
                                                       CHOLMOD_REAL, cc);
    for (i=0; i<lenActiveCorners; ++i) {
        for (j=0; j<NCONEBASIS; ++j) {
            SET_TRIPLET_RCV(E_trip, NCONEBASIS*i + j, i, 1.);
        }
    }
    assert(E_trip->nnz == lenActiveCorners*NCONEBASIS);

    double mus[lenActiveCorners];
    for (i=0; i<lenActiveCorners; ++i)
        mus[i] = mu;

    /*
     * The vector 'hMinvtau_Qd_den' represents h * Minv * tau + Qd
     * or  h * Minv * (fg - Cqd) + Qd.
     * This time we just set this value to Qd.
     */
    cholmod_dense *hMinvtau_Qd_den = cholmod_allocate_dense(lenActiveBodies*nd, 1, lenActiveBodies*nd, CHOLMOD_REAL, cc);
    for (i=0; i<lenActiveBodies; ++i) {
        const unsigned int bodyidx = activeBodies[i];
        const double *qdi = &Y[ bodyidx*2*nd + nd ]; /* qdi = [pdi] + [vdi] */
        double *x         = (double *)(hMinvtau_Qd_den->x) + nd*i;
        memcpy(x, qdi, sizeof(double)*nd);
    }




    cholmod_sparse *N_sp         = cholmod_triplet_to_sparse(N_trip,      N_trip->nnz,      cc);
    cholmod_sparse *D_sp         = cholmod_triplet_to_sparse(D_trip,      D_trip->nnz,      cc);
    cholmod_sparse *NT_sp        = cholmod_transpose(N_sp, 1, cc);
    cholmod_sparse *DT_sp        = cholmod_transpose(D_sp, 1, cc);
    cholmod_sparse *NTMinv_sp    = cholmod_ssmult(NT_sp, Minv_a_sp, 0, 1, 1, cc);
    cholmod_sparse *DTMinv_sp    = cholmod_ssmult(DT_sp, Minv_a_sp, 0, 1, 1, cc);
    cholmod_sparse *NTMinvN_sp   = cholmod_ssmult(NTMinv_sp, N_sp, 0, 1, 1, cc); /* RETURN THIS -- aka 'M00' */
    cholmod_sparse *DTMinvN_sp   = cholmod_ssmult(DTMinv_sp, N_sp, 0, 1, 1, cc); /* RETURN THIS -- aka 'M10' */
    cholmod_sparse *DTMinvN_T_sp = cholmod_transpose(DTMinvN_sp, 1, cc);         /* RETURN THIS -- aka 'M10.T' */
    cholmod_sparse *DTMinvD_sp   = cholmod_ssmult(DTMinv_sp, D_sp, 0, 1, 1, cc); /* RETURN THIS -- aka 'M11' */
    cholmod_sparse *Mu_sp        = constructSparseDiagonalMatrix(lenActiveCorners, mus, cc); /* RETURN THIS */
    cholmod_sparse *E_sp         = cholmod_triplet_to_sparse(E_trip,      E_trip->nnz,      cc); /* RETURN THIS */
    cholmod_sparse *NegET_sp     = cholmod_transpose(E_sp, 1, cc);               /* RETURN THIS -- aka '-E.T' */
    cholmod_dense  *Minus1_den   = cholmod_allocate_dense(1,1,1,CHOLMOD_REAL, cc);
    /*
    SPARSE_CHECK (N_sp);
    SPARSE_CHECK (D_sp);
    SPARSE_CHECK (NT_sp);
    SPARSE_CHECK (DT_sp);
    SPARSE_CHECK (NTMinv_sp);
    SPARSE_CHECK (DTMinv_sp);
    SPARSE_CHECK (NTMinvN_sp);
    SPARSE_CHECK (DTMinvN_sp);
    SPARSE_CHECK (DTMinvN_T_sp);
    SPARSE_CHECK (DTMinvD_sp);
    SPARSE_CHECK (Mu_sp);
    SPARSE_CHECK (E_sp);
    SPARSE_CHECK (NegET_sp);
    DENSE_CHECK  (Minus1_den);
    */
    ((double *)(Minus1_den->x))[0] = -1;
    cholmod_scale(Minus1_den, CHOLMOD_SCALAR, NegET_sp, cc);

    /* Calculate 'hMinvtau_Qd_den' */
    const double alphah[2]  = {  h,  0 };
    cholmod_sdmult(Minv_a_sp, 0, alphah, beta, tau_a_den, hMinvtau_Qd_den, cc);

    PRINT_DENSE_VECTOR(hMinvtau_Qd_den);

    /* RETURN THIS */
    cholmod_sparse *Z0_sp        = cholmod_allocate_sparse(DTMinvN_T_sp->nrow, E_sp->ncol,
                                                           0, /* nzmax */
                                                           0, /* sorted */
                                                           0, /* packed */
                                                           0, /* stype */
                                                           CHOLMOD_REAL, cc);

    /*
     * RETURN THIS -- aka LCP_q0
     * N^T * ( h M^-1 tau + Qd )
     */
    cholmod_dense *NThMinvtau_Qd_den = cholmod_allocate_dense(lenActiveCorners, 1, lenActiveCorners, CHOLMOD_REAL, cc);
    memset(NThMinvtau_Qd_den->x, 0, sizeof(double)*lenActiveCorners);
    cholmod_sdmult(NT_sp, 0, alpha, beta, hMinvtau_Qd_den, NThMinvtau_Qd_den, cc);
    /* RETURN THIS -- aka LCP_q1 */
    cholmod_dense *DThMinvtau_Qd_den = cholmod_allocate_dense(NCONEBASIS*lenActiveCorners, 1, NCONEBASIS*lenActiveCorners, CHOLMOD_REAL, cc);
    memset(DThMinvtau_Qd_den->x, 0, sizeof(double)*NCONEBASIS*lenActiveCorners);
    cholmod_sdmult(DT_sp, 0, alpha, beta, hMinvtau_Qd_den, DThMinvtau_Qd_den, cc);

    PRINT_DENSE_VECTOR(DThMinvtau_Qd_den);

    /* RETURN THIS -- aka LCP_q2 */
    cholmod_dense *Zerop_den = cholmod_zeros(lenActiveCorners, 1, CHOLMOD_REAL, cc);



    /*
     *  Return list : NTMinvN_sp (M00) , DTMinvN_sp (M10) , DTMinvN_T_sp (M10.T) ,
     *                DTMinvD_sp (M11) , Mu_sp (Mu) , E_sp (E) , NegET_sp (-E.T) ,
     *                Z0_sp (Z0) , NThMinvtau_Qd_sp (LCP_q0) , DThMinvtau_Qd_sp (LCP_q1) ,
     *                Zerop_den (LCP_q2)
     *
     *
     *     LCP_M  =  /                                            \
     *               |   NTMinvN_sp      DTMinvN_T_sp     Z0_sp   |
     *               |                                            |
     *               |   DTMinvN_sp      DTMinvD_sp       E_sp    |
     *               |                                            |
     *               |   Mu_sp           NegET_sp         Z0_sp   |
     *               \                                            /
     *
     *     LCP_q  =  /                       \
     *               |   NThMinvtau_Qd_den   |
     *               |                       |
     *               |   DThMinvtau_Qd_den   |
     *               |                       |
     *               |   Zerop_den           |
     *               \                       /
     */
    lcpSm->NTMinvN_sp        = NTMinvN_sp;
    lcpSm->DTMinvN_T_sp      = DTMinvN_T_sp;
    lcpSm->Z0_sp             = Z0_sp;
    lcpSm->DTMinvN_sp        = DTMinvN_sp;
    lcpSm->DTMinvD_sp        = DTMinvD_sp;
    lcpSm->E_sp              = E_sp;
    lcpSm->Mu_sp             = Mu_sp;
    lcpSm->NegET_sp          = NegET_sp;
    lcpSm->NThMinvtau_Qd_den = NThMinvtau_Qd_den;
    lcpSm->DThMinvtau_Qd_den = DThMinvtau_Qd_den;
    lcpSm->Zerop_den         = Zerop_den;

    lcpSm->N_sp              = N_sp;
    lcpSm->D_sp              = D_sp;
    lcpSm->NTMinv_sp         = NTMinv_sp;
    lcpSm->DTMinv_sp         = DTMinv_sp;
    lcpSm->hMinvtau_Qd_den   = hMinvtau_Qd_den;

    /* Release all intermediate matrices and vectors */
    cholmod_free_triplet(&N_trip, cc);
    cholmod_free_triplet(&D_trip, cc);
    cholmod_free_triplet(&E_trip, cc);
    cholmod_free_dense(&Minus1_den, cc);
    //cholmod_free_sparse(&N_sp, cc);
    //cholmod_free_sparse(&D_sp, cc);
    cholmod_free_sparse(&NT_sp, cc);
    cholmod_free_sparse(&DT_sp, cc);
    //cholmod_free_sparse(&NTMinv_sp, cc);
    //cholmod_free_sparse(&DTMinv_sp, cc);
    //cholmod_free_dense(&hMinvtau_Qd_den, cc);
    //cholmod_free_sparse(&NTMinvN_sp, cc);
    //cholmod_free_sparse(&DTMinvN_sp, cc);
    //cholmod_free_sparse(&DTMinvD_sp, cc);
    //cholmod_free_sparse(&Mu_sp, cc);
    //cholmod_free_sparse(&E_sp, cc);
    //cholmod_free_sparse(&Z0_sp, cc);

    return 0;
}

cholmod_sparse *AssembleLcpMatrix(LcpSubmatrices* lcpSm, cholmod_common *cc)
{
    /*
     *     LCP_M  =  /                                            \
     *               |   NTMinvN_sp      DTMinvN_T_sp     Z0_sp   |
     *               |                                            |
     *               |   DTMinvN_sp      DTMinvD_sp       E_sp    |
     *               |                                            |
     *               |   Mu_sp           NegET_sp         Z0_sp   |
     *               \                                            /
     *
     */
    cholmod_sparse *M_00_01 = cholmod_horzcat(lcpSm->NTMinvN_sp, lcpSm->DTMinvN_T_sp, 1, cc);
    /*
    SPARSE_CHECK(M_00_01);
    SPARSE_CHECK(lcpSm->NTMinvN_sp);
    SPARSE_CHECK(lcpSm->DTMinvN_T_sp);
    SPARSE_CHECK(lcpSm->Z0_sp);
    */
    cholmod_sparse *Row0    = cholmod_horzcat(M_00_01,           lcpSm->Z0_sp,        1, cc);
    cholmod_sparse *M_10_11 = cholmod_horzcat(lcpSm->DTMinvN_sp, lcpSm->DTMinvD_sp,   1, cc);
    cholmod_sparse *Row1    = cholmod_horzcat(M_10_11,           lcpSm->E_sp,         1, cc);
    /*
    SPARSE_CHECK(lcpSm->DTMinvN_sp);
    SPARSE_CHECK(lcpSm->DTMinvD_sp);
    SPARSE_CHECK(M_10_11);
    SPARSE_CHECK(lcpSm->E_sp);
    */
    cholmod_sparse *M_20_21 = cholmod_horzcat(lcpSm->Mu_sp,      lcpSm->NegET_sp,     1, cc);
    cholmod_sparse *Row2    = cholmod_horzcat(M_20_21,           lcpSm->Z0_sp,        1, cc);
    cholmod_sparse *Row_0_1 = cholmod_vertcat(Row0,    Row1, 1, cc);
    cholmod_sparse *LCP_M   = cholmod_vertcat(Row_0_1, Row2, 1, cc); /* RETURN THIS */
    //SPARSE_CHECK(LCP_M);
    cholmod_free_sparse(&M_00_01, cc);
    cholmod_free_sparse(&Row0, cc);
    cholmod_free_sparse(&M_10_11, cc);
    cholmod_free_sparse(&Row1, cc);
    cholmod_free_sparse(&M_20_21, cc);
    cholmod_free_sparse(&Row2, cc);
    cholmod_free_sparse(&Row_0_1, cc);
    return LCP_M;
}
cholmod_dense *AssembleLcpVector(LcpSubmatrices* lcpSm, cholmod_common *cc) {
    /*
     *  LCP_q0 = dot(N.T, h * Minv_fg_Cqd + Qd_a)
	 *  LCP_q1 = dot(D.T, h * Minv_fg_Cqd + Qd_a)
	 *  LCP_q2 = zeros((p))
	 */
    /*
     *     LCP_q  =  /                       \
     *               |   NThMinvtau_Qd_den   |
     *               |                       |
     *               |   DThMinvtau_Qd_den   |
     *               |                       |
     *               |   Zerop_den           |
     *               \                       /
     */
    const unsigned int lenNThMinvtau_Qd_den = lcpSm->NThMinvtau_Qd_den->nrow;
    const unsigned int lenDThMinvtau_Qd_den = lcpSm->DThMinvtau_Qd_den->nrow;
    const unsigned int lenZerop_den = lcpSm->Zerop_den->nrow;
    const unsigned int qrow = lenNThMinvtau_Qd_den + lenDThMinvtau_Qd_den + lenZerop_den;
    cholmod_dense  *LCP_q   = cholmod_allocate_dense(qrow, 1, qrow, CHOLMOD_REAL, cc);
    double *q = (double *)(LCP_q->x);
    memcpy(q,                                               lcpSm->NThMinvtau_Qd_den->x, sizeof(double)*lenNThMinvtau_Qd_den);
    memcpy(q + lenNThMinvtau_Qd_den,                        lcpSm->DThMinvtau_Qd_den->x, sizeof(double)*lenDThMinvtau_Qd_den);
    memcpy(q + lenNThMinvtau_Qd_den + lenDThMinvtau_Qd_den, lcpSm->Zerop_den->x,         sizeof(double)*lenZerop_den);
    return LCP_q;
}
void FreeLcpSubmatrices(LcpSubmatrices *lcpSm, cholmod_common *cc) {
    cholmod_free_sparse (&lcpSm->NTMinvN_sp,        cc);
    cholmod_free_sparse (&lcpSm->DTMinvN_T_sp,      cc);
    cholmod_free_sparse (&lcpSm->Z0_sp,             cc);
    cholmod_free_sparse (&lcpSm->DTMinvN_sp,        cc);
    cholmod_free_sparse (&lcpSm->DTMinvD_sp,        cc);
    cholmod_free_sparse (&lcpSm->E_sp,              cc);
    cholmod_free_sparse (&lcpSm->Mu_sp,             cc);
    cholmod_free_sparse (&lcpSm->NegET_sp,          cc);
    cholmod_free_dense  (&lcpSm->NThMinvtau_Qd_den, cc);
    cholmod_free_dense  (&lcpSm->DThMinvtau_Qd_den, cc);
    cholmod_free_dense  (&lcpSm->Zerop_den,         cc);

    cholmod_free_sparse (&lcpSm->N_sp,              cc);
    cholmod_free_sparse (&lcpSm->D_sp,              cc);
    cholmod_free_sparse (&lcpSm->NTMinv_sp,         cc);
    cholmod_free_sparse (&lcpSm->DTMinv_sp,         cc);
    cholmod_free_dense  (&lcpSm->hMinvtau_Qd_den,   cc);
}

int ReparameterizeExpRot(const unsigned int nd, const unsigned int n, const unsigned int m,
                         const double h, double Ynext[2*nd*n + m], const double Y [2*nd*n + m]) {
    /*
    Python code:

    th = linalg.norm(bodies[k].q[3:6])
    if th > pi:
        th = linalg.norm(bodies[k].q[3:6])
        bodies[k].q[3:6] = (1.-2*pi/th)*bodies[k].q[3:6]
        qprevmag = linalg.norm(qPrev[k][3:6])
        qPrev[k][3:6] = (1.-2*pi/qprevmag)*qPrev[k][3:6]
        bodies[k].qd[3:6] = (bodies[k].q[3:6] - qPrev[k][3:6])/h
        #print 'Body', k, ' reparmed at frame', gFrame
    */
	int i;
	for (i=0; i<n; ++i) {
	    double *v  = &Ynext[2*nd*i +      3];
	    double *vd = &Ynext[2*nd*i + nd + 3];
	    const double th = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
        if (th > M_PI) {
            const double rescale = 1. - 2*M_PI/th;
            v[0] *= rescale;
            v[1] *= rescale;
            v[2] *= rescale;
            const double *vprev = &Y[2*nd*i + 3];
            const double thprev = sqrt(vprev[0]*vprev[0]+vprev[1]*vprev[1]+vprev[2]*vprev[2]);
            const double rescaleprev = 1. - 2*M_PI/thprev;
            double nV[3] = { rescaleprev * vprev[0],
                             rescaleprev * vprev[1],
                             rescaleprev * vprev[2] };
            vd[0] = (v[0] - nV[0])/h;
            vd[1] = (v[1] - nV[1])/h;
            vd[2] = (v[2] - nV[2])/h;
        }
	}
	return 0;
}

int LCP_exp(const unsigned int nd, const unsigned int n, const unsigned int m,
            double Ynext[2*nd*n + m],
            double contactForces[n][8][3], unsigned int lenContactForces[n], unsigned int contactPoints[n][8],
            const double penetration0,
            const double Y [2*nd*n + m],
            const double extForces[n][nd],
            const double I[n][4], const double mass[n],
            const double corners[n][8][3],
            const unsigned int NCONEBASIS, const double CONEBASIS[NCONEBASIS][3],
            const double mu, const double h,
            /* if 0 explicit integration occurs, otherwise only contact information is calculated */
            const int contactForceInfoOnly,
            cholmod_common *cc) {
    //printf("LCP_exp () started.\n");
    assert(Y != Ynext);
    const double nY = 2*nd*n + m;
    assert ( nY > 0 );
    memset(Ynext, 0, sizeof(double)*nY);
    CHECK_SOURCE_LINE;
    /*
     * Determine which bodies are active or not.
     * Active bodies are sent to the LCP routine to compute contact forces.
     * Both active and inactive bodies next step state is calculated.
     */
    unsigned int activeCorners[8*n][2]; unsigned int lenActiveCorners  = 0; /* 8n is the maximum number of active corners */
    unsigned int activeBodies[n];       unsigned int lenActiveBodies   = 0;
    unsigned int inactiveBodies[n];     unsigned int lenInactiveBodies = 0;
    DetermineActiveness(nd, n, m,
                        activeCorners, &lenActiveCorners,
                        activeBodies, &lenActiveBodies,
                        inactiveBodies, &lenInactiveBodies,
                        Y, corners, penetration0);
    assert (lenActiveCorners <= 8*n) ;
    memset(lenContactForces, 0, sizeof(unsigned int)*n);
    if (lenActiveBodies == 0 && contactForceInfoOnly) {
        /* No needed to proceed further */
        return 0;
    }
    int i, j;
    /*********************************
     *  1. Active
     *********************************/
    if (lenActiveBodies) {
        /*
         * We can use one of two strategies to compute contact forces
         * on multibody-muticontact cases.
         *
         * Strategy A.
         *    Solve a single LCP by gathering all bodies to construct
         *    the matrix M and the vector q.
         *
         * Strategy B.
         *    Solve n LCPs by constructing an individual matrices and vectors,
         *    i.e. M1...Mn and q1...qn where n is the number of active bodies.
         *
         * We guess the second one (Strategy B) is more efficient than
         * the other, but not sure.
         * The size of the matrix M and the vector q will be 10*p x 10*p
         * respectively where p is the number of contact forces.
         *
         * Example: three rigid bodies with the following contact states
         *
         *            Body 1 -- 0 contacts
         *                 2 -- 3 contacts
         *                 3 -- 4 contacts
         *
         * Using the strategy A requires to construct a 70x70 matrix and
         * solve it. (single LCP)
         * Using the strategy B requires to construct a 30x30 matrix and
         * a 40x40 matrix and solve them. (two LCPs)
         */
        cholmod_sparse *M_a_sp    = 0;
        cholmod_sparse *Minv_a_sp = 0;
        cholmod_dense  *tau_a_den = 0;
        BuildEquationsOfMotionCoefficients(&M_a_sp, &Minv_a_sp, &tau_a_den,
                                           nd, n, m, lenActiveBodies, activeBodies,
                                           Y, extForces, I, mass, cc);
        LcpSubmatrices lcpSm;
        memset(&lcpSm, 0, sizeof(LcpSubmatrices));
        BuildLCPSubmatrices(&lcpSm, nd, n, m, Y, I, mass,
                            corners,
                            NCONEBASIS, CONEBASIS, mu, h,
                            lenActiveCorners, activeCorners,
                            lenActiveBodies, activeBodies,
                            M_a_sp, Minv_a_sp, tau_a_den,
                            cc);
        /* Not needed anymore */
        cholmod_free_sparse(&M_a_sp,    cc);
        cholmod_free_sparse(&Minv_a_sp, cc);
        cholmod_free_dense (&tau_a_den, cc);

        cholmod_sparse *LCP_M_sp = AssembleLcpMatrix(&lcpSm, cc);
        cholmod_dense  *LCP_q_den = AssembleLcpVector(&lcpSm, cc);
        cholmod_dense  *LCP_M_den = cholmod_sparse_to_dense(LCP_M_sp, cc);
        const unsigned int qsize = LCP_q_den->nrow;
        cholmod_dense  *x_opt_den = cholmod_allocate_dense(qsize, 1, qsize, CHOLMOD_REAL, cc);
        double *x_opt = (double *)(x_opt_den->x);
        assert ( lenActiveBodies  == lcpSm.N_sp->nrow / nd );
        assert ( lenActiveCorners == lcpSm.N_sp->ncol );

        /* Check the matrix LCP_M_den */
        //PrintEntireSparseMatrix(LCP_M_sp);
        double *LCP_q = (double *)(LCP_q_den->x);
        PRINT_VECTOR(LCP_q, qsize);
        int status = lemke_1darray(qsize, x_opt, (double *)(LCP_M_den->x), (double *)(LCP_q_den->x), 0, cc);
        assert(status == 0);
        /*
         * Verify the sanity of the solution
         * by checking these equations.
         * (w is calculated from Mx+q)
         *
         *    x_i * w_i   == 0
         *    x_i         >= 0
         *    w_i         >= 0
         */
        cholmod_dense *LCP_w = cholmod_copy_dense(LCP_q_den, cc);
        cholmod_sdmult(LCP_M_sp, 0, alpha, beta, x_opt_den, LCP_w, cc);
        int lcpFailed = 0;
        for (i=0; i<qsize; ++i) {
            const double xi = x_opt[i];
            const double wi = ((double *)(LCP_w->x))[i];
            const double xiwi = xi*wi;
            if (xiwi != 0) {
                //PRINT_FLH; printf("WARN ... x[%d]*w[%d] = %lg\n", i, i, xiwi);
                if (fabs(xiwi) > 1e-5)
                    lcpFailed = 1;
            }
            if (xi < 0) {
                //PRINT_FLH; printf("WARN ... x[%d] = %lg\n", i, xi);
                if (xi < -1e-5)
                    lcpFailed = 1;
            }
            if (wi < 0) {
                //PRINT_FLH; printf("WARN ... w[%d] = %lg\n", i, wi);
                if (wi < -1e-5)
                    lcpFailed = 1;
            }
        }
        if (lcpFailed) {
            PRINT_FLH; printf(" ************ LCP failed ************\n");
            print_trace();
            exit(-12345);
        }
        PRINT_DENSE_VECTOR(x_opt_den);

        cholmod_dense *cn_den   = cholmod_allocate_dense(           lenActiveCorners, 1,            lenActiveCorners, CHOLMOD_REAL, cc);
        cholmod_dense *beta_den = cholmod_allocate_dense(NCONEBASIS*lenActiveCorners, 1, NCONEBASIS*lenActiveCorners, CHOLMOD_REAL, cc);
        cholmod_dense *lamb_den = cholmod_allocate_dense(           lenActiveCorners, 1,            lenActiveCorners, CHOLMOD_REAL, cc);
        memcpy(cn_den->x  , x_opt + 0                                             , sizeof(double)*lenActiveCorners           );
        memcpy(beta_den->x, x_opt + lenActiveCorners                              , sizeof(double)*NCONEBASIS*lenActiveCorners);
        memcpy(lamb_den->x, x_opt + lenActiveCorners + NCONEBASIS*lenActiveCorners, sizeof(double)*lenActiveCorners           );

        double *cn = (double*)(cn_den->x);
        double *beta = (double*)(beta_den->x);
        PRINT_VECTOR(cn, lenActiveCorners);
        PRINT_VECTOR(beta, NCONEBASIS*lenActiveCorners);

        cholmod_dense  *MinvNcn_den   = cholmod_allocate_dense(nd*lenActiveBodies, 1, nd*lenActiveBodies, CHOLMOD_REAL, cc);
        cholmod_dense  *MinvDbeta_den = cholmod_allocate_dense(nd*lenActiveBodies, 1, nd*lenActiveBodies, CHOLMOD_REAL, cc);
        memset(MinvNcn_den->x,   0, sizeof(double)*nd*lenActiveBodies);
        memset(MinvDbeta_den->x, 0, sizeof(double)*nd*lenActiveBodies);
        cholmod_sdmult(lcpSm.NTMinv_sp, 1 /* transpose */, alpha, beta, cn_den,   MinvNcn_den,   cc);
        cholmod_sdmult(lcpSm.DTMinv_sp, 1 /* transpose */, alpha, beta, beta_den, MinvDbeta_den, cc);


        double Qimp_cont[nd*lenActiveBodies];
        double Qd_a_next[nd*lenActiveBodies];
        double Q_a_next[nd*lenActiveBodies];

        double *MinvNcn     = (double *)(MinvNcn_den->x);
        double *MinvDbeta   = (double *)(MinvDbeta_den->x);
        double *hMinvtau_Qd = (double *)(lcpSm.hMinvtau_Qd_den->x);

        for (i=0; i<lenActiveBodies; ++i) {
            const unsigned int bodyidx = activeBodies[i];
            for (j=0; j<nd; ++j) {
                const unsigned int idx = nd*i + j;
                Qimp_cont[idx] = MinvNcn[idx]     + MinvDbeta[idx];
                Qd_a_next[idx] = hMinvtau_Qd[idx] + Qimp_cont[idx];
                Q_a_next[idx]  = h*Qd_a_next[idx] + Y[ bodyidx*2*nd + j];  /* Y[bodyindex*2*nd + j] == Qa[idx] */
            }

            PRINT_VECTOR(Qimp_cont, nd*lenActiveBodies);
            PRINT_VECTOR(Qd_a_next, nd*lenActiveBodies);
            PRINT_VECTOR(Q_a_next, nd*lenActiveBodies);

            memcpy(&Ynext[bodyidx*2*nd +  0], &Q_a_next[nd*i],  sizeof(double)*nd);
            memcpy(&Ynext[bodyidx*2*nd + nd], &Qd_a_next[nd*i], sizeof(double)*nd);
        }

        PRINT_VECTOR(Ynext, nY);
        /*
        Python code:

        # For contact force visualization
		for b in bodies:
			b.cf = [] # Clear contact force visualization data
		for (kp, cp), k in zip(activeCorners, range(p)):
			# kp: Body index
			# cp: Corner index
			kk = activeBodies.index(kp)
			fric = dot(D[6*kk:6*(kk+1),8*k:8*(k+1)], beta[8*k:8*(k+1)])
			nor  = dot(N[6*kk:6*(kk+1),k], cn[k])
			cf   = (fric[0:3] + nor[0:3]) / h
			bodies[kp].cf.append(cf)
        */
        /* Calculate contact forces in Cartesian coordinates */
        for (i=0; i<lenActiveCorners; ++i) {

            const unsigned int kp = activeCorners[i][0]; /* body index */
            const unsigned int cp = activeCorners[i][1]; /* corner index */
            const unsigned int kk = Index(lenActiveBodies, activeBodies, kp); /* body index among active bodies */

            int rsetD[nd];         for (j=0; j<nd;         ++j) rsetD[j] = nd*kk        + j;
            int csetD[NCONEBASIS]; for (j=0; j<NCONEBASIS; ++j) csetD[j] = NCONEBASIS*i + j;
            cholmod_sparse *Dsub_sp     = cholmod_submatrix(lcpSm.D_sp, rsetD, nd, csetD, NCONEBASIS, 1, 1, cc);
            cholmod_dense  *betasub_den = cholmod_allocate_dense(NCONEBASIS, 1, NCONEBASIS, CHOLMOD_REAL, cc);
            cholmod_dense  *fric_den    = cholmod_allocate_dense(nd,         1, nd,         CHOLMOD_REAL, cc);
            memcpy(betasub_den->x, beta + NCONEBASIS*i, sizeof(double)*NCONEBASIS);
            cholmod_sdmult(Dsub_sp, 0, alpha, beta0, betasub_den, fric_den, cc);
            cholmod_free_sparse(&Dsub_sp, cc);
            cholmod_free_dense(&betasub_den, cc);

            int rsetN[nd];         for (j=0; j<nd;         ++j) rsetN[j] = nd*kk        + j;
            int csetN[1];          csetN[0] = i;
            cholmod_sparse *Nsub_sp     = cholmod_submatrix(lcpSm.N_sp, rsetN, nd, csetN, 1, 1, 1, cc);
            cholmod_dense  *cnsub_den   = cholmod_allocate_dense(1, 1, 1, CHOLMOD_REAL, cc);
            ((double *)(cnsub_den->x))[0] = cn[i];
            cholmod_scale(cnsub_den, CHOLMOD_SCALAR, Nsub_sp, cc);
            cholmod_dense  *nor_den     = cholmod_sparse_to_dense(Nsub_sp, cc);
            cholmod_free_sparse(&Nsub_sp, cc);
            cholmod_free_dense(&cnsub_den, cc);

            double *fric = (double *)(fric_den->x);
            double *nor  = (double *)(nor_den->x);
            double cf[3];
            cf[0] = (fric[0]+nor[0])/h;
            cf[1] = (fric[1]+nor[1])/h;
            cf[2] = (fric[2]+nor[2])/h;
            cholmod_free_dense(&fric_den, cc);
            cholmod_free_dense(&nor_den, cc);

            contactPoints[kp][ lenContactForces[kp] ] = cp;
            memcpy(contactForces[kp][ lenContactForces[kp] ], cf, sizeof(double)*3);
            ++lenContactForces[kp];
        }
        cholmod_free_sparse(&LCP_M_sp,     cc);
        cholmod_free_dense(&LCP_q_den,     cc);
        cholmod_free_dense(&LCP_M_den,     cc);
        cholmod_free_dense(&cn_den,        cc);
        cholmod_free_dense(&beta_den,      cc);
        cholmod_free_dense(&lamb_den,      cc);
        cholmod_free_dense(&MinvNcn_den,   cc);
        cholmod_free_dense(&MinvDbeta_den, cc);
        cholmod_free_dense(&LCP_w, cc);
        cholmod_free_dense(&x_opt_den, cc);
        FreeLcpSubmatrices(&lcpSm, cc);
    }
    if (contactForceInfoOnly) {
        /* No needed to proceed further */
        CHECK_SOURCE_LINE;
        return 0;
    }

    /*********************************
     *  2. Inactive
     *********************************/
    if (lenInactiveBodies) {
        cholmod_sparse *M_i_sp    = 0;
        cholmod_dense  *tau_i_den = 0;
        BuildEquationsOfMotionCoefficients(&M_i_sp, 0, &tau_i_den,
                                           nd, n, m, lenInactiveBodies, inactiveBodies,
                                           Y, extForces, I, mass, cc);
        SPARSE_CHECK(M_i_sp);
        DENSE_CHECK(tau_i_den);
        //PrintEntireSparseMatrix(M_i_sp);
        PRINT_DENSE_VECTOR(tau_i_den);
        PRINT_VECTOR(I[0], 4);
        PRINT_VECTOR(Y, nY);

        double Qdd[nd*lenInactiveBodies];
        double Qd_i_next[nd*lenInactiveBodies];
        double Q_i_next[nd*lenInactiveBodies];
        /* hard copy the symmetric part of M_i_sp since SolveLinearSystem does not recognize compact form */
        cholmod_sparse *noCompact_M_i_sp = cholmod_copy(M_i_sp, 0, 1, cc);
        SolveLinearSystem(noCompact_M_i_sp, tau_i_den, nd*lenInactiveBodies, Qdd, cc);
        //PRINT_VECTOR(Qdd, nd*lenInactiveBodies);
        for (i=0; i<lenInactiveBodies; ++i) {
            const unsigned int bodyidx = inactiveBodies[i];
            for (j=0; j<nd; ++j) {
                const unsigned int idx = nd*i + j;
                Qd_i_next[idx] = Y[bodyidx*2*nd + nd + j] + h*Qdd[idx];
                Q_i_next[idx]  = Y[bodyidx*2*nd +      j] + h*Qd_i_next[idx];  /* Y[bodyindex*2*nd + j] == Qi[idx] */
            }
            memcpy(&Ynext[bodyidx*2*nd +  0], &Q_i_next[nd*i],  sizeof(double)*nd);
            memcpy(&Ynext[bodyidx*2*nd + nd], &Qd_i_next[nd*i], sizeof(double)*nd);
        }
        /* Not needed anymore */
        cholmod_free_sparse(&M_i_sp,           cc);
        cholmod_free_sparse(&noCompact_M_i_sp, cc);
        cholmod_free_dense (&tau_i_den,        cc);
    }

    /*
     * Exponential map rotation value should be reparameterized
     * if the length of vector is close to 2n*pi.
     */
    ReparameterizeExpRot(nd, n, m, h, Ynext, Y);

    return 0;
}

int LCP_exp_Python(const unsigned int nd, const unsigned int n, const unsigned int m,
            double Ynext[2*nd*n + m],
            double contactForces[n][8][3], unsigned int lenContactForces[n], unsigned int contactPoints[n][8],
            const double penetration0,
            const double Y [2*nd*n + m], const double extForces[n][nd],
            const double I[n][4], const double mass[n],
            const double corners[n][8][3],
            const unsigned int NCONEBASIS, const double CONEBASIS[NCONEBASIS][3],
            const double mu, const double h, const int contactForceInfoOnly) {
    //printf("LCP_exp_Python() started.\n");
    cholmod_common c ;
    cholmod_start (&c) ;
    CHECK_SOURCE_LINE;
    int status = LCP_exp(nd, n, m, Ynext, contactForces, lenContactForces, contactPoints, penetration0, Y, extForces, I, mass, corners, NCONEBASIS, CONEBASIS, mu, h, contactForceInfoOnly, &c);
    CHECK_SOURCE_LINE;
    cholmod_finish(&c);
    CHECK_SOURCE_LINE;
    PRINT_VECTOR_INT(lenContactForces, n);
    return status;
}

int LCP_exp_test() {
    const unsigned int nd = 6;
    const unsigned int n = 1;
    const unsigned int m = 0;
    const unsigned int nY = 2*nd*n+m;
    double Y[nY], I[n][4], corners[n][8][3], mass[n];
    double extForces[n][nd];
    double Ynext[nY];
    const unsigned int NCONEBASIS = 8;
    const double pi = 4*atan(1.);
    const double CONEBASIS[8][3] =
           { {        1.,         0., 0},
             { cos(pi/4),  sin(pi/4), 0},
             {        0.,         1., 0},
             {-cos(pi/4),  sin(pi/4), 0},
             {       -1.,         0., 0},
             {-cos(pi/4), -sin(pi/4), 0},
             {         0,        -1., 0},
             { cos(pi/4), -sin(pi/4), 0} };
    memset(Y, 0, sizeof(double)*nY);
    memset(mass, 0, sizeof(double)*n);
    memset(extForces, 0, sizeof(double)*n*nd);
    Y[2] = 5; /* start from the sky */
    /* slightly rotated */
    Y[3] = 0.3;
    Y[4] = 0.2;
    Y[5] = 0.1;
    /* have angular velocity */
    Y[9] = 10;
    Y[10] = 10;
    Y[11] = 10;
    mass[0] = 1;
    double sx = 0.5, sy = 0.9, sz = 3.5;
    SymbolicTensor(I[0], sx, sy, sz, mass[0]/(sx*sy*sz));
    BoxCorners(corners[0], sx, sy, sz);

//    Y[12 + 0] = 0;
//    Y[12 + 1] = 0;
//    Y[12 + 2] = 10; /* start from the sky */
//    mass[1] = 2;
//    SymbolicTensor(I[1], sx, sy, sz, mass[1]/(sx*sy*sz));
//    BoxCorners(corners[1], sx, sy, sz);

    const double mu = 1.5;
    const double h = 0.0025;
    const double penetration0 = 0.;

    cholmod_common c ;
    cholmod_start (&c) ;
    int status = 0;

    double contactForces[n][8][3];
    unsigned int lenContactForces[n];
    unsigned int contactPoints[n][8];

    const unsigned int iter = 100000;
    int it;
    for (it = 0; it < iter; ++it) {
        status = LCP_exp(nd, n, m, Ynext, contactForces, lenContactForces, contactPoints,
                         penetration0, Y, extForces, I, mass, corners, NCONEBASIS, CONEBASIS, mu, h, 0, &c);
        if (it % 1000 == 0)
            printf("== Frame %5d ==\n", it);
        PRINT_VECTOR(Y, nY);
        //PRINT_VECTOR(Ynext, nY);
        memcpy(Y, Ynext, sizeof(double)*nY);

        //printf("%5d Body0 : %lg %lg %lg , %lg %lg %lg\n", it, Y[0     ], Y[1     ], Y[2     ], Y[3     ], Y[4     ], Y[5     ]);
    }
    printf("== Frame %5d ==\n", iter);
    printf("Body0 : %lg %lg %lg , %lg %lg %lg\n", Y[0     ], Y[1     ], Y[2     ], Y[3     ], Y[4     ], Y[5     ]);
    //printf("Body1 : %lg %lg %lg , %lg %lg %lg\n", Y[12 + 0], Y[12 + 1], Y[12 + 2], Y[12 + 3], Y[12 + 4], Y[12 + 5]);
    cholmod_finish(&c);
    return status;
}
