#include <string.h>
#include <assert.h>
#include <cholmod.h>
#include "PymStruct.h"
#include "MathUtil.h"
#include "ExpBodyMoEq_real.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "dRdv_real.h"
#include "Config.h"
#include "CholmodMacro.h"
#include "DebugPrintDef.h"
#include "StateDependents.h"

void ZVQ(cholmod_sparse **Z, double V[4], cholmod_sparse **Q,
         const double chi[6], const double corner[3], const double normal[3], const double W[4][4], const double dRdv_tensor[3][4][4],
         cholmod_common *cc) {
    /*
     *
     *        [   d   W                          d  W              ]
     *  Z  =  [   -------- . corner    . . .    -------- . corner  ]
     *        [   d chi_0                       d chi_5            ]
     *
     */
    double r[3][3];
    int i, j;
    for (i=0;i<3;++i) TransformPoint(r[i], dRdv_tensor[i], corner);
    cholmod_triplet *Ztrip = cholmod_allocate_triplet(4, 6, 4*6, 0, CHOLMOD_REAL, cc); /* i, j, x, nnz */
    assert(Ztrip->nnz == 0);
    FOR_0(i, 3) SET_TRIPLET_RCV(Ztrip, i, i, 1.0);
    FOR_0(i, 3) { /* column index */
        FOR_0(j, 3) { /* row index */
            const double rij = r[i][j];
            SET_TRIPLET_RCV(Ztrip, j, 3+i, rij);
        }
    }
    *Z = cholmod_triplet_to_sparse(Ztrip, Ztrip->nnz, cc);

    //printf("Z\n");
    //PrintEntireSparseMatrix(*Z);

    /*
     *   V  =  W corner - Z chi
     */
    double corner_W[3];
    AffineTransformPoint(corner_W, W, corner);
    cholmod_dense *chi_den = cholmod_allocate_dense(6, 1, 6, CHOLMOD_REAL, cc);
    memcpy(chi_den->x, chi, sizeof(double)*6);
    double alpha[2] = {1, 0};
    double beta0[2] = {0, 0};
    cholmod_dense *Z_chi_1 = cholmod_allocate_dense(4, 1, 4, CHOLMOD_REAL, cc);
    cholmod_sdmult(*Z, 0, alpha, beta0, chi_den, Z_chi_1, cc);
    V[0] = corner_W[0] - ((double *)(Z_chi_1->x))[0];
    V[1] = corner_W[1] - ((double *)(Z_chi_1->x))[1];
    V[2] = corner_W[2] - ((double *)(Z_chi_1->x))[2];
    V[3] =     1       - ((double *)(Z_chi_1->x))[3]; /* corner_W[3] is 1.0 since it is a point */

    //printf("V\n");
    //__PRINT_VECTOR(V, 4);

    /*
     *          [                                        Z                                             ]^T
     *          [                                                                                      ]
     *   Q  =   [ [  d   W                                       d   W                             ]   ]
     *          [ [ --------- . corner <dot> normal     . . .   --------- . corner <dot> normal    ]   ]
     *          [ [  d chi_0                                     d chi_0                           ]   ]
     */
    cholmod_dense *qij_2nd_col_T_den = cholmod_allocate_dense(1, 6, 1, CHOLMOD_REAL, cc);
    ((double *)(qij_2nd_col_T_den->x))[0] = normal[0];
    ((double *)(qij_2nd_col_T_den->x))[1] = normal[1];
    ((double *)(qij_2nd_col_T_den->x))[2] = normal[2];
    ((double *)(qij_2nd_col_T_den->x))[3] = Dot33(r[0], normal);
    ((double *)(qij_2nd_col_T_den->x))[4] = Dot33(r[1], normal);
    ((double *)(qij_2nd_col_T_den->x))[5] = Dot33(r[2], normal);
    cholmod_sparse *qij_2nd_col_T = cholmod_dense_to_sparse(qij_2nd_col_T_den, 1, cc);
    cholmod_sparse *Q_T = cholmod_vertcat(*Z, qij_2nd_col_T, 1, cc);
    *Q = cholmod_transpose(Q_T, 1, cc);

    cholmod_free_triplet(&Ztrip, cc);
    cholmod_free_dense(&chi_den, cc);
    cholmod_free_dense(&Z_chi_1, cc);
    cholmod_free_dense(&qij_2nd_col_T_den, cc);
    cholmod_free_sparse(&qij_2nd_col_T, cc);
    cholmod_free_sparse(&Q_T, cc);
}

int PymConstructRbStatedep(pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc) {
    const pym_rb_named_t* rbn = &rb->b;
    assert(rbn->rotParam == RP_EXP);
    MassMatrixAndCqdVector(sd->M, sd->Cqd, rbn->p, rbn->q, rbn->pd, rbn->qd, rbn->Ixyzw);
    Invert6x6MassMatrix(sd->Minv, sd->M);
    double grav_force[3] = { 0, 0, -9.81 * rbn->m };
    double ZERO3[3] = {0,0,0};
    GeneralizedForce(sd->f_g, rbn->q, grav_force, ZERO3);
    double dRdvn[3][3][3];
    dRdv(dRdvn[0], dRdvn[1], dRdvn[2], rbn->q);
    int i,j,k;
    memset(sd->dWdchi_tensor, 0, sizeof(sd->dWdchi_tensor));
    for (i=0;i<3;++i) for (j=0;j<3;++j) for (k=0;k<3;++k) sd->dWdchi_tensor[i][j][k] = dRdvn[i][j][k];
    double chi_1[6], chi_0[6];
    chi_1[0] = rbn->p[0]; chi_1[1] = rbn->p[1]; chi_1[2] = rbn->p[2];
    chi_1[3] = rbn->q[0]; chi_1[4] = rbn->q[1]; chi_1[5] = rbn->q[2];
    chi_0[0] = rbn->p0[0]; chi_0[1] = rbn->p0[1]; chi_0[2] = rbn->p0[2];
    chi_0[3] = rbn->q0[0]; chi_0[4] = rbn->q0[1]; chi_0[5] = rbn->q0[2];
    GetWFrom6Dof(sd->W_0, chi_0);
    GetWFrom6Dof(sd->W_1, chi_1);
    double chi_2_nocf[6];
    const double h = pymCfg->h;
    for (i=0; i<6; ++i) {
        double Minv_h2_C_fg = 0;
        for (j=0; j<6; ++j) {
            Minv_h2_C_fg += sd->Minv[i][j]*(h*h)*(-sd->Cqd[j] + sd->f_g[j]);
        }
        chi_2_nocf[i] = 2*chi_1[i] - chi_0[i] + Minv_h2_C_fg;
    }
    double W_2_nocf[4][4]; GetWFrom6Dof(W_2_nocf, chi_2_nocf);
    sd->nContacts_2 = 0;
    for (j=0; j<8; ++j) {
        double pcj_2_nocf_W[3], pcj_1_W[3];
        AffineTransformPoint(pcj_2_nocf_W, W_2_nocf, rbn->corners[j]);
        AffineTransformPoint(pcj_1_W, sd->W_1, rbn->corners[j]);

        const double ctY = pcj_2_nocf_W[1];
        const double theta = pymCfg->slant;
        const double z = -ctY*tan(theta); /* ground level */
        if (pcj_2_nocf_W[2] <= z+50.0) { /* 0.030 optimal */
            sd->contactIndices_2[ sd->nContacts_2 ] = j;
            double *pcj_fix = sd->contactsFix_2[ sd->nContacts_2 ];
            for (k=0;k<3;++k) pcj_fix[k] = (pcj_1_W[k] + pcj_2_nocf_W[k]) / 2.0;
            pcj_fix[2] = 0; /* fix contact points Z axis to 0 (flat ground assumption) */
            pcj_fix[3] = 1; /* homogeneous component*/
            ++sd->nContacts_2;
            printf("   ACP : %s (cornerid=%d) %lf %lf %lf\n",
                   rbn->name, j, pcj_2_nocf_W[0], pcj_2_nocf_W[1], pcj_2_nocf_W[2]);
        }
    }
    const int nd = NUM_DOF;
    const int np = sd->nContacts_2;
    const int na = rbn->nAnchor;
    const int nm = rbn->nFiber;
    const int Asubrowsizes[] = {nd,         // inertial constraints
                                nd*np,      // generalized contact forces <--> contact forces relationship
                                4*np,       // compose contact point non-moving variable
                                np,         // compose friction cone constraints variable (mu*normal_force)
                                4*np,       // next state <--> next CP position relationship
                                nd,         // next state <--> trajectory relationship
                                np,         // tangential/normal contact force constraint (tan dot nor == 0)
                                4*na };     // next state <--> next JA position relationship

    const int Asubcolsizes[] = {nd,          // chi^{(l+1)}
                                nd*np,       // f_c
                                5*np,        // c_c
                                4*np,        // \tilde{p}_c^{(l+1)}
                                4*np,        // \Delta \tilde{p}_c
                                np,          // \epsilon_c
                                np,          // \mu f_{c,z}
                                nd,          // \Delta \chi
                                1,           // \epsilon_\Delta
                                nd*nm,       // f_T
                                4*na };      // \tilde{p}_A^{(l+1)}
    //__PRINT_VECTOR_INT(Asubcolsizes, 10);
    sd->Asubrows = sizeof(Asubrowsizes)/sizeof(int);
    sd->Asubcols = sizeof(Asubcolsizes)/sizeof(int);
    assert(sizeof(int)*(1+sd->Asubrows) == sizeof(sd->Ari));
    assert(sizeof(int)*(1+sd->Asubcols) == sizeof(sd->Aci));
    sd->Ari[0] = 0;
    sd->Aci[0] = 0;
    for (i=0; i<sd->Asubrows; ++i) sd->Ari[i+1] = sd->Ari[i] + Asubrowsizes[i];
    for (i=0; i<sd->Asubcols; ++i) sd->Aci[i+1] = sd->Aci[i] + Asubcolsizes[i];

    for (i = 0; i < np; ++i) {
        const double *pcj = rbn->corners[ sd->contactIndices_2[i] ];
        double normal[4] = {0,0,1,0};
        const double theta = pymCfg->slant;
        const double cth = cos(-theta);
        const double sth = sin(-theta);
        const double ny = cth*normal[1] - sth*normal[2];
        const double nz = sth*normal[1] + cth*normal[2];
        normal[1] = ny;
        normal[2] = nz;
        memcpy(sd->contactsNormal_1[i], normal, sizeof(double)*4);
        ZVQ(&sd->Z[i], sd->V[i], &sd->Q[i], chi_1, pcj, normal, sd->W_1, sd->dWdchi_tensor, cc);
    }

    return 0;
}

void PymDestroyRbStatedep(pym_rb_statedep_t *sd, cholmod_common *cc) {
    int j;
    FOR_0(j, sd->nContacts_2) {
        cholmod_free_sparse(&sd->Z[j], cc);
        cholmod_free_sparse(&sd->Q[j], cc);
    }
}

void GetAMatrix(cholmod_triplet **AMatrix, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc) {
    const pym_rb_named_t *rbn = &rb->b;
    /* We can calculate optimal estimate for the number of nonzero elements */
    size_t nzmax = 0;
    const int nd = 6;                  /* DOF of this body */
    const int np = sd->nContacts_2;    /* # of contact points of this body */
    const int na = rbn->nAnchor;       /* # of joint anchor points of this body */
    const int nmi = rbn->nFiber;       /* # of muscle fibers connected to this body */

    //printf("   %s has %d fibers.\n", rbn->name, nmi);

    nzmax += 3+9;       /* Subblock 01 : M/h^2 */
    nzmax += nd*np;     /* Subblock 02 : (-1)^ce_{j in P} */
    nzmax += nd*nmi;    /* Subblock 03 : (-1)^ce_{j in M(i)} */
    nzmax += nd*np;     /* Subblock 04 : -1 */
    nzmax += (nd*5)*np; /* Subblock 05 : (Q_j)^de_{j in P} */
    nzmax += 4*np;      /* Subblock 06 : 1 */
    nzmax += 4*np;      /* Subblock 07 : 1 */
    nzmax += np;        /* Subblock 08 : (c_1)^de_{j in P} */
    nzmax += np;        /* Subblock 09 :-1 */
    nzmax += (4*nd)*np; /* Subblock 10 : (Z_j)^re_{j in P} */
    nzmax += 4*np;      /* Subblock 11 :-1 */
    nzmax += nd;        /* Subblock 12 : 1 */
    nzmax += nd;        /* Subblock 13 : 1 */
    nzmax += 3*np;      /* Subblock 14 : (C_n)^de_{j in P} */
    nzmax += (4*nd)*na; /* Subblock 15 : (Z_j)^re_{j in A} */
    nzmax += 4*na;      /* Subblock 16 :-1 */
//    printf("    A matrix body name           : %s\n", rbn->name);
//    printf("    A matrix constants (np)      : %d\n", np);
//    printf("    A matrix constants (nmi)     : %d\n", nmi);
//    printf("    A matrix subblock dimension  : %d x %d\n", sd->Asubrows, sd->Asubcols);
//    printf("    A matrix size                : %d x %d\n", sd->Ari[sd->Asubrows], sd->Aci[sd->Asubcols]);
    cholmod_triplet *AMatrix_trip = cholmod_allocate_triplet(sd->Ari[sd->Asubrows], sd->Aci[sd->Asubcols], nzmax, 0, CHOLMOD_REAL, cc);
    assert(AMatrix_trip->nnz == 0);
    int i, j;
    const double h = pymCfg->h;
    const double mu = pymCfg->mu;
    /* Sub-block 01 */
    for (i=0;i<3;++i) {
        SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 0, i, i, sd->M[i][i] / (h*h));
        for (j=0;j<3;++j) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 0, i+3, j+3, sd->M[i+3][j+3] / (h*h));
    }
    /* Sub-block 02 */
    for (i=0;i<nd*np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 1, i%nd, i, -1);
    /* Sub-block 03 */
    for (i=0;i<nd*nmi;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 9, i%nd, i, -1);
    /* Sub-block 04 */
    for (i=0;i<nd*np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 1, 1, i, i, -1);
    /* Sub-block 05 */
    FOR_0(j, np) {
        const cholmod_sparse *Qj = sd->Q[j];
        cholmod_triplet *Qj_trip = cholmod_sparse_to_triplet(Qj, cc);

        //printf("Q_%d\n", j);
        //PrintEntireSparseMatrix(Qj);

        FOR_0(i, Qj_trip->nnz) {
            SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 1, 2,
                                     nd*j + ((int *)(Qj_trip->i))[i],
                                     5*j  + ((int *)(Qj_trip->j))[i],
                                     ((double *)(Qj_trip->x))[i] );
        }
        cholmod_free_triplet(&Qj_trip, cc);
    }
    /* Sub-block 06 */
    for (i=0;i<4*np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 2, 3, i, i, 1);
    /* Sub-block 07 */
    for (i=0;i<4*np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 2, 4, i, i, -1);
    /* Sub-block 08 */
    /*
     *  ctx  cty  ctz ctw  cn
     * [ 0,   0,   0,  0,  mu ]
     *                     .
     *                        .
     *                           .
     *                            [ 0,   0,   0,  0,  mu ]
     */
    FOR_0(j, np) {
        SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 3, 2,
                                 1*j  + 0,
                                 5*j  + 4,
                                 mu );
    }
    /* Sub-block 09 */
    for (i=0;i<np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 3, 6, i, i, -1);
    /* Sub-block 10 */
    FOR_0(j, np) {
        const cholmod_sparse *Zj = sd->Z[j];
        cholmod_triplet *Zj_trip = cholmod_sparse_to_triplet(Zj, cc);
        FOR_0(i, Zj_trip->nnz) {
            SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 4, 0,
                                     4*j  + ((int *)(Zj_trip->i))[i],
                                     0    + ((int *)(Zj_trip->j))[i],
                                     ((double *)(Zj_trip->x))[i] );
        }
        cholmod_free_triplet(&Zj_trip, cc);
    }
    /* Sub-block 11 */
    for (i=0;i<4*np;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 4, 3, i, i, -1);
    /* Sub-block 12 */
    for (i=0;i<nd;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 5, 0, i, i, 1);
    /* Sub-block 13 */
    for (i=0;i<nd;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 5, 7, i, i, -1);
    /* Sub-block 14 */
    for (i=0;i<np;++i) {
        const double nx = sd->contactsNormal_1[i][0];
        const double ny = sd->contactsNormal_1[i][1];
        const double nz = sd->contactsNormal_1[i][2];
        const double nw = sd->contactsNormal_1[i][3];
        assert (nw == 0);
        SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 6, 2, i, 5*i + 0, nx);
        SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 6, 2, i, 5*i + 1, ny);
        SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 6, 2, i, 5*i + 2, nz);
    }
    /* Sub-block 15 (similar to sub-block 10) */
    FOR_0(j, na) {
        const cholmod_sparse *Zj = sd->Za[j];
        cholmod_triplet *Zj_trip = cholmod_sparse_to_triplet(Zj, cc);
        FOR_0(i, Zj_trip->nnz) {
            SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 7, 0,
                    4*j  + ((int *)(Zj_trip->i))[i],
                    0    + ((int *)(Zj_trip->j))[i],
                    ((double *)(Zj_trip->x))[i] );
        }
        cholmod_free_triplet(&Zj_trip, cc);
    }
    /* Sub-block 16 (similar to sub-block 11) */
    for (i=0;i<4*na;++i) SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 7, 10, i, i, -1);

    *AMatrix = AMatrix_trip;
//    cholmod_print_triplet(AMatrix_trip, rbn->name, cc);
}

void GetEta(double **_eta, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc) {
    const pym_rb_named_t *rbn = &rb->b;
    /* We can calculate optimal estimate for the number of nonzero elements */
    const int nd = 6;
    const int np = sd->nContacts_2;
    size_t etaDim = sd->Ari[sd->Asubrows];
    //printf("RB %s etaDim = %d\n", rbn->name, etaDim);
    double *eta = (double *)malloc(sizeof(double) * etaDim);
    memset(eta, 0, sizeof(double) * etaDim);

    double chi_1[6], chi_0[6];
    chi_1[0] = rbn->p[0]; chi_1[1] = rbn->p[1]; chi_1[2] = rbn->p[2];
    chi_1[3] = rbn->q[0]; chi_1[4] = rbn->q[1]; chi_1[5] = rbn->q[2];
    chi_0[0] = rbn->p0[0]; chi_0[1] = rbn->p0[1]; chi_0[2] = rbn->p0[2];
    chi_0[3] = rbn->q0[0]; chi_0[4] = rbn->q0[1]; chi_0[5] = rbn->q0[2];

    const double h = pymCfg->h;
    int i,j;
    FOR_0(i, nd) {
        double M_q_q0 = 0;
        FOR_0(j, nd) {
            M_q_q0 += (sd->M[i][j] * (2*chi_1[j] - chi_0[j]))/(h*h);
        }
        eta[sd->Ari[0] + i] = M_q_q0 + (-sd->Cqd[i] + sd->f_g[i]);
    }

    FOR_0(i, np) {
        FOR_0(j, 4) {
            eta[sd->Ari[2] + 4*i + j] = sd->contactsFix_2[i][j];
            eta[sd->Ari[4] + 4*i + j] = -sd->V[i][j];
        }
    }

    FOR_0(i, nd) {
        eta[sd->Ari[5] + i] = rbn->chi_ref[i];

        if (i==2) {
            /* DEBUG */
            const double ctY = rb->b.p[1];
            const double theta = pymCfg->slant;
            const double z = -ctY*tan(theta);
            eta[sd->Ari[5] + i] += -ctY * tan(theta);
        }
    }



    *_eta = eta;
    //__PRINT_VECTOR(eta, etaDim);
}
