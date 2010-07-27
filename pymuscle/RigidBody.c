#include "PymPch.h"
#include "PymStruct.h"
#include "Config.h"
#include "RigidBody.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "CholmodMacro.h"

void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+3], const pym_config_t *const pymCfg) {
    pym_rb_named_t *rbn = &rb->b;
    assert(rbn->rotParam == RP_EXP);
    int i, j;
    FOR_0(i, 3) {
        rbn->p0[i] = rbn->p[i];
        rbn->p[i]  = Chi_1[i];
    }
    FOR_0(i, 3) {
        rbn->q0[i] = rbn->q[i];
        rbn->q[i]  = Chi_1[i+3];
    }
    /* update discrete velocity based on current and previous step */
    FOR_0(j, 3) {
        rbn->pd[j] = (rbn->p[j] - rbn->p0[j]) / pymCfg->h;
        rbn->qd[j] = (rbn->q[j] - rbn->q0[j]) / pymCfg->h;
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
    const int na = rbn->nAnchor;
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
    int i, j;

    /* c_{2,i} */
    FOR_0(i, nd) {
        double M_q_q0 = 0;
        FOR_0(j, nd) {
            M_q_q0 += (sd->M[i][j] * (2*chi_1[j] - chi_0[j]))/(h*h);
        }
        eta[sd->Ari[0] + i] = M_q_q0 + (-sd->Cqd[i] + sd->f_g[i] + sd->f_ext[i]);
    }
    /* p^(l)_{cfix,i}   and   (-V_ij)re_{j \in P} */
    FOR_0(i, np) {
        FOR_0(j, 4) {
            eta[sd->Ari[2] + 4*i + j] = sd->contactsFix_2[i][j];
            eta[sd->Ari[4] + 4*i + j] = -sd->V[i][j];
        }
    }
    /* chi^(l)_{i, ref} */
    FOR_0(i, nd) {
        eta[sd->Ari[5] + i] = rbn->chi_ref[i];

        if (i==2) {
            /* TODO Ground slant parameter - ground level calculation */
            const double ctY = rb->b.p[1];
            const double theta = pymCfg->slant;
            const double z = -ctY*tan(theta);
            eta[sd->Ari[5] + i] += z;
        }
    }
    /* (-V_ij)re_{j \in A} */
    FOR_0(i, na) {
        FOR_0(j, 4) {
            eta[sd->Ari[7] + 4*i + j] = -sd->Va[i][j];
        }
    }
    *_eta = eta;
    //__PRINT_VECTOR(eta, etaDim);
}
