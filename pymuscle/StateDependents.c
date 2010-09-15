#include "PymPch.h"
#include "PymStruct.h"
#include "MathUtil.h"
#include "ExpBodyMoEq_real.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "dRdv_real.h"
#include "Config.h"
#include "CholmodMacro.h"
#include "DebugPrintDef.h"
#include "StateDependents.h"
#include "PymDebugMessageFlags.h"

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

    if (Q && normal) {
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
        cholmod_free_dense(&qij_2nd_col_T_den, cc);
        cholmod_free_sparse(&qij_2nd_col_T, cc);
        cholmod_free_sparse(&Q_T, cc);
    } else if (Q == 0 && normal == 0) {

    } else {
        assert(!"Invalid usage");
    }

    cholmod_free_triplet(&Ztrip, cc);
    cholmod_free_dense(&chi_den, cc);
    cholmod_free_dense(&Z_chi_1, cc);

}

int PymConstructRbStatedep(pym_rb_statedep_t *sd, const pym_rb_t *rb,
                           FILE *dmstreams[],
                           const pym_config_t *pymCfg, cholmod_common *cc) {
    const pym_rb_named_t* rbn = &rb->b;
    assert(rbn->rotParam == RP_EXP);
    MassMatrixAndCqdVector(sd->M, sd->Cqd, rbn->p, rbn->q, rbn->pd, rbn->qd, rbn->Ixyzw);
    Invert6x6MassMatrix(sd->Minv, sd->M);
    double grav_force[3] = { 0, 0, -9.81 * rbn->m };
    double ZERO3[3] = {0,0,0};
    GeneralizedForce(sd->f_g, rbn->q, grav_force, ZERO3);
    /* TODO External force on each rigid body */
    GeneralizedForce(sd->f_ext, rbn->q, rbn->extForce, rbn->extForcePos);
    double dRdvn[3][3][3];
    dRdv(dRdvn[0], dRdvn[1], dRdvn[2], rbn->q);
    int i,j,k;
    memset(sd->dWdchi_tensor, 0, sizeof(sd->dWdchi_tensor));
    for (i=0;i<3;++i)
        for (j=0;j<3;++j)
            for (k=0;k<3;++k)
                sd->dWdchi_tensor[i][j][k] = dRdvn[i][j][k];
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
    sd->nContacts_1 = 0;
    sd->nContacts_2 = 0;
    for (j=0; j<8; ++j) {
        double pcj_2_nocf_W[3], pcj_1_W[3], pcj_0_W[3];
        AffineTransformPoint(pcj_2_nocf_W, W_2_nocf, rbn->corners[j]);
        AffineTransformPoint(pcj_1_W, sd->W_1, rbn->corners[j]);
        AffineTransformPoint(pcj_0_W, sd->W_0, rbn->corners[j]);

        const double ctY = pcj_2_nocf_W[1];
        const double theta = pymCfg->slant;
        const double z = -ctY*tan(theta); /* ground level */
        /*
         * TODO [TUNE] Contact point level threshold
         * Optimal value table
         *    Walk0 -> 0.050
         *    Nav0  -> 0.050
         *    Exer0 -> 0.050 (unknown)
         */
        //if (pcj_2_nocf_W[2] <= -0.004) {
        //if (pcj_1_W[2] <= 0 && pcj_0_W[2] > pcj_1_W[2]) {
        if (pcj_2_nocf_W[2] <= 0.100) {
            sd->contactIndices_2[ sd->nContacts_2 ] = j;
            double *pcj_fix = sd->contactsFix_2[ sd->nContacts_2 ];
            for (k=0;k<3;++k) {
                pcj_fix[k] = (pcj_1_W[k] + pcj_2_nocf_W[k]) / 2.0;
                //pcj_fix[k] = pcj_1_W[k];
            }
            pcj_fix[2] = 0; /* fix contact points Z axis to 0 (flat ground assumption) */
            pcj_fix[3] = 1; /* homogeneous component*/
            ++sd->nContacts_2;
            FILE *dmst = dmstreams[PDMTE_FBYF_ACTIVE_CORNER_POINTS];
            fprintf(dmst, "   ACP : %s (cornerid=%d) %lf %lf %lf\n",
                    rbn->name, j,
                    pcj_2_nocf_W[0], pcj_2_nocf_W[1], pcj_2_nocf_W[2]);
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
                                4*na,        // \tilde{p}_A^{(l+1)}
                                1,           // \epsilon_rotparam
                                 };
    //__PRINT_VECTOR_INT(Asubcolsizes, 10);
    sd->Asubrows = sizeof(Asubrowsizes)/sizeof(int);
    sd->Asubcols = sizeof(Asubcolsizes)/sizeof(int);
    assert(sizeof(int)*(1+sd->Asubrows) == sizeof(sd->Ari));
    assert(sizeof(int)*(1+sd->Asubcols) == sizeof(sd->Aci));
    sd->Ari[0] = 0;
    sd->Aci[0] = 0;
    for (i=0; i<sd->Asubrows; ++i) sd->Ari[i+1] = sd->Ari[i] + Asubrowsizes[i];
    for (i=0; i<sd->Asubcols; ++i) sd->Aci[i+1] = sd->Aci[i] + Asubcolsizes[i];

    FOR_0(i, np) {
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
    FOR_0(i, na) {
        const double *pcj = rbn->jointAnchors[i];
        ZVQ(&sd->Za[i], sd->Va[i], 0, chi_1, pcj, 0, sd->W_1, sd->dWdchi_tensor, cc);
    }

    return 0;
}

void PymDestroyRbStatedep(pym_rb_statedep_t *sd, pym_rb_named_t *rbn, cholmod_common *cc) {
    int j;
    FOR_0(j, sd->nContacts_2) {
        cholmod_free_sparse(&sd->Z[j], cc);
        cholmod_free_sparse(&sd->Q[j], cc);
    }
    FOR_0(j, rbn->nAnchor) {
        cholmod_free_sparse(&sd->Za[j], cc);
    }
}

