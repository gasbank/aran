#include "PymCorePch.h"
#include "PymStruct.h"
#include "MathUtil.h"
#include "ExpBodyMoEq_real.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "dRdv_real.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "StateDependents.h"
#include "CholmodMacro.h"
#include "PymDebugMessageFlags.h"

void ZVQ(cholmod_sparse **Z, double V[4], cholmod_sparse **Q,
  const double chi[7], const double corner[3], const double normal[3], const double W[4][4], const double dRdv_tensor[4][4][4],
  pym_rot_param_t rp, cholmod_common *cc) {
    int rpn = -1;
    if (rp == RP_EXP) rpn = 3;
    else if (rp == RP_QUAT_WFIRST) rpn = 4;
    else assert(0);
    const int nd = 3 + rpn;
    /*
    *
    *        [   d   W                             d  W               ]
    *  Z  =  [   -------- . corner    . . .    ------------ . corner  ]
    *        [   d chi_0                       d chi_(nd-1)           ]
    *
    */
    double r[4][3];
    int i, j;
    for (i = 0; i < rpn; ++i)
      TransformPoint(r[i], dRdv_tensor[i], corner);
    cholmod_triplet *Ztrip = cholmod_allocate_triplet(4, nd, 4*nd, 0, CHOLMOD_REAL, cc); /* i, j, x, nnz */
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
      *          [                                        Z                                                  ]^T
      *          [                                                                                           ]
      *   Q  =   [ [  d   W                                          d   W                               ]   ]
      *          [ [ --------- . corner <dot> normal     . . .   -------------- . corner <dot> normal    ]   ]
      *          [ [  d chi_0                                     d chi_(nd-1)                           ]   ]
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

int PymConstructRbStatedep(pym_rb_statedep_t *sd_all, pym_rb_statedep_t *sd,
  const pym_rb_t *rb, FILE *dmstreams[], const pym_config_t *pymCfg, cholmod_common *cc)
{
  const double h = pymCfg->h;
  const pym_rb_named_t *const rbn = &rb->b;
  if (rbn->rotParam == RP_EXP) {
    MassMatrixAndCqdVector(sd->M, sd->Cqd, rbn->p, rbn->q, rbn->pd, rbn->qd, rbn->Ixyzw);
    Invert6x6MassMatrix(sd->Minv, sd->M);
  }
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
  const double chi_1[7] = {
    rbn->p[0],  rbn->p[1],  rbn->p[2],
    rbn->q[0],  rbn->q[1],  rbn->q[2], rbn->q[3]
  };
  const double chi_0[7] = {
    rbn->p[0] - h*rbn->pd[0],  rbn->p[1] - h*rbn->pd[1],  rbn->p[2] - h*rbn->pd[2],
    rbn->q[0] - h*rbn->qd[0],  rbn->q[1] - h*rbn->qd[1],  rbn->q[2] - h*rbn->qd[2], rbn->q[3] - h*rbn->qd[3]
  };
  GetWFrom6Dof(sd->W_0, chi_0);
  GetWFrom6Dof(sd->W_1, chi_1);
  double chi_2_nocf[6];
  for (i=0; i<6; ++i) {
    double Minv_h2_C_fg = 0;
    for (j=0; j<6; ++j) {
      Minv_h2_C_fg += sd->Minv[i][j]*(h*h)*(-sd->Cqd[j] + sd->f_g[j]);
    }
    chi_2_nocf[i] = 2*chi_1[i] - chi_0[i] + Minv_h2_C_fg;
  }
  double W_2_nocf[4][4]; GetWFrom6Dof(W_2_nocf, chi_2_nocf);
  double W_ref[4][4]; GetWFrom6Dof(W_ref, rbn->chi_ref);
  sd->nContacts_0 = sd->nContacts_1;
  sd->nContacts_1 = 0;
  //sd->nContacts_2 = 0;
  for (j=0; j<8; ++j) {
    double pcj_1_W[3], pcj_0_W[3], pcj_ref[3];
    AffineTransformPoint(pcj_1_W, sd->W_1, rbn->corners[j]);
    AffineTransformPoint(pcj_0_W, sd->W_0, rbn->corners[j]);
    AffineTransformPoint(pcj_ref, W_ref, rbn->corners[j]);

    memcpy(sd->contacts_0[j], pcj_0_W, sizeof(double)*3);
    memcpy(sd->contacts_1[j], pcj_1_W, sizeof(double)*3);

    double groundLevel = 0.0;
    if (strcmp(pymCfg->trajName, "Walk1") == 0) {
      if (pymCfg->curFrame > 180) 
        groundLevel += 0.630;
      else if (pymCfg->curFrame > 150)
        groundLevel += 0.413;
      else if (pymCfg->curFrame > 120)
        groundLevel += 0.210;
    }
    if (pcj_1_W[2] <= groundLevel) {
      sd->contactIndices_1[ sd->nContacts_1 ] = j;
      const double unit_sliding_distance = PymDist(2, pcj_1_W, pcj_0_W)/pymCfg->h;
      if (unit_sliding_distance < 1e-4)
        sd->contactTypes_1[ sd->nContacts_1 ] = static_contact;
      else
        sd->contactTypes_1[ sd->nContacts_1 ] = dynamic_contact;
      double *pcj_fix = sd->contactsFix_1[ sd->nContacts_1 ];
      pcj_fix[0] = sd->contacts_1[j][0];
      pcj_fix[1] = sd->contacts_1[j][1];
      pcj_fix[2] = 0; /* fix contact points Z axis to 0 (flat ground assumption) */
      pcj_fix[3] = 0; /* homogeneous component*/
      ++sd->nContacts_1;
      FILE *dmst = dmstreams[PDMTE_FBYF_ACTIVE_CORNER_POINTS];
      fprintf(dmst, "   ACP : %s (cornerid=%d) %lf %lf %lf\n",
        rbn->name, j,
        pcj_1_W[0], pcj_1_W[1], pcj_1_W[2]);
    }
  }

  const int nd = 3 + pymCfg->nrp;
  const int np = sd->nContacts_1;
  const int na = rbn->nAnchor;
  const int nm = rbn->nFiber;
  const int Asubrowsizes[] = {nd,         // 0:inertial constraints
    nd*np,      // 1:generalized contact forces <--> contact forces relationship
    4*np,       // 2:compose contact point non-moving variable
    np,         // 3:compose friction cone constraints variable (mu*normal_force)
    4*np,       // 4:next state <--> next CP position relationship
    nd,         // 5:next state <--> trajectory relationship
    np,         // 6:tangential/normal contact force constraint (tan dot nor == 0)
    4*na,       // 7:next state <--> next JA position relationship
    nd,         // 8:next state <--> current state relationship
    np,         // 9:contact normal force deternimed by penalty term
    np,         //10:contact tangential force (x)
    np,         //11:contact tangential force (y)
  };
  const int Asubcolsizes[] = {nd,          // 0:chi^{(l+1)}
    nd*np,       // 1:f_c
    5*np,        // 2:c_c
    4*np,        // 3:\tilde{p}_c^{(l+1)}
    4*np,        // 4:\Delta \tilde{p}_c
    np,          // 5:\epsilon_c
    np,          // 6:\mu f_{c,z}
    nd,          // 7:\Delta \chi ref
    1,           // 8:\epsilon_\Delta ref
    nd*nm,       // 9:f_T
    4*na,        // 10:\tilde{p}_A^{(l+1)}
    1,           // 11:\epsilon_rotparam
    nd,          // 12:\Delta \chi prv
    1,           // 13:\epsilon_\Delta \chi prv
    np,          // 14:epsilon_cp_|z|
    np,          // 15:kappa_cp_|z| (penalty method nonnegativity compensation variable)
    nd,          // 16:generalized gravitational force
  };

  BOOST_STATIC_ASSERT(sizeof(int) + sizeof(Asubrowsizes) == sizeof(sd->Ari));
  BOOST_STATIC_ASSERT(sizeof(int) + sizeof(Asubcolsizes) == sizeof(sd->Aci));

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
    const double *pcj = rbn->corners[ sd->contactIndices_1[i] ];
    double normal[4] = {0,0,1,0};
    const double theta = pymCfg->slant;
    const double cth = cos(-theta);
    const double sth = sin(-theta);
    const double ny = cth*normal[1] - sth*normal[2];
    const double nz = sth*normal[1] + cth*normal[2];
    normal[1] = ny;
    normal[2] = nz;
    memcpy(sd->contactsNormal_1[i], normal, sizeof(double)*4);
    ZVQ(&sd->Z[i], sd->V[i], &sd->Q[i], chi_1, pcj, normal, sd->W_1, sd->dWdchi_tensor, rbn->rotParam, cc);
  }
  FOR_0(i, na) {
    const double *pcj = rbn->jointAnchors[i];
    ZVQ(&sd->Za[i], sd->Va[i], 0, chi_1, pcj, 0, sd->W_1, sd->dWdchi_tensor, rbn->rotParam, cc);
  }

  return 0;
}

void PymDestroyRbStatedep(pym_rb_statedep_t *sd, pym_rb_named_t *rbn, cholmod_common *cc) {
  int j;
  FOR_0(j, sd->nContacts_1) {
    cholmod_free_sparse(&sd->Z[j], cc);
    cholmod_free_sparse(&sd->Q[j], cc);
  }
  FOR_0(j, rbn->nAnchor) {
    cholmod_free_sparse(&sd->Za[j], cc);
  }
}

void pym_init_statedep( pym_rb_statedep_t &sd ) 
{
  sd.nContacts_0 = 0;
  sd.nContacts_1 = 0;
  //sd.nContacts_2 = 0;
}
