#include "PymCorePch.h"
#include "PymStruct.h"
#include "Config.h"
#include "RigidBody.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "CholmodMacro.h"
#include "MathUtil.h"

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795029)
#endif

static const double SPRING_K = 5000; /* Ground spring constant (penalty method) */
static const double SPRING_C = 100;    /* Ground damping constant (penalty method) */


std::ostream &operator << (std::ostream &s, const pym_rb_t &rb) {
  if (rb.b.rotParam == RP_EXP) {
    double th = PymNorm(3, rb.b.q);
    s << rb.b.name << " p [" << rb.b.p[0] << ", " << rb.b.p[1] << ", " << rb.b.p[2]
    << "] / q [" << rb.b.q[0] << ", " << rb.b.q[1] << ", " << rb.b.q[2] << "] th=" << th << " (RP_EXP)";
  } else if (rb.b.rotParam == RP_QUAT_WFIRST) {
    s << rb.b.name << " p [" << rb.b.p[0] << ", " << rb.b.p[1] << ", " << rb.b.p[2]
    << "] / q " << rb.b.q[0] << " [" << rb.b.q[1] << ", " << rb.b.q[2] << ", " << rb.b.q[3] << "] (RP_QUAT_WFIRST)";
  }
  return s;
}

void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+4],
		       const pym_config_t *const pymCfg) {
  pym_rb_named_t *rbn = &rb->b;
  assert(rbn->rotParam == RP_EXP);
  int i, j;
  double p0[3], q0[4];
  FOR_0(i, 3) {
    p0[i] = rbn->p[i];
    rbn->p[i]  = Chi_1[i];
  }
  FOR_0(i, 4) {
    q0[i] = rbn->q[i];
    rbn->q[i]  = Chi_1[i+3];
  }
  /* update discrete velocity based on current and previous step */
  FOR_0(j, 3) {
    rbn->pd[j] = (rbn->p[j] - p0[j]) / pymCfg->h;
  }
  FOR_0(j, 4) {
    rbn->qd[j] = (rbn->q[j] - q0[j]) / pymCfg->h;
  }
}

void GetAMatrix(cholmod_triplet **AMatrix, const pym_rb_statedep_t *const sd,
		const pym_rb_t *rb, const pym_config_t *pymCfg,
		cholmod_common *cc) {
  const pym_rb_named_t *rbn = &rb->b;
  /* We can calculate optimal estimate for the number of nonzero elements */
  size_t nzmax = 0;
  const int nd = 3 + pymCfg->nrp;              /* DOF of this body */
  const int np = sd->nContacts_1;/* # of contact points of this body */
  const int na = rbn->nAnchor;   /* # of joint anchor points of this body */
  const int nmi = rbn->nFiber;   /* # of muscle fibers connected to this body */
  //printf("   %s has %d fibers.\n", rbn->name, nmi);

  nzmax += 3+4*4;     /* Subblock 01 : M/h^2 */
  nzmax += nd*np;     /* Subblock 02 : (-1)^ce_{j in P} */
  nzmax += nd*nmi;    /* Subblock 03 : (-1)^ce_{j in M} */
  nzmax += nd;        /* Subblock 04 : (-1) */
  nzmax += nd;        /* Subblock 04a: (-1) */
  nzmax += nd*np;     /* Subblock 05 : -1 */
  nzmax += (nd*5)*np; /* Subblock 06 : (Q_j)^de_{j in P} */
  nzmax += 4*np;      /* Subblock 07 : 1 */
  nzmax += 4*np;      /* Subblock 08 : 1 */
  nzmax += np;        /* Subblock 09 : (c_1)^de_{j in P} */
  nzmax += np;        /* Subblock 10 :-1 */
  nzmax += (4*nd)*np; /* Subblock 11 : (Z_j)^re_{j in P} */
  nzmax += 4*np;      /* Subblock 12 :-1 */
  nzmax += nd;        /* Subblock 13 : 1 */
  nzmax += nd;        /* Subblock 14 : 1 */
  nzmax += 3*np;      /* Subblock 15 : (C_n)^de_{j in P} */
  nzmax += (4*nd)*na; /* Subblock 16 : (Z_j)^re_{j in A} */
  nzmax += 4*na;      /* Subblock 17 :-1 */
  nzmax += nd;        /* Subblock 18 : 1 */
  nzmax += nd;        /* Subblock 19 :-1 */
  nzmax += np;        /* Subblock 20 : (C_ns)^de_{j in P} */
  nzmax += np;        /* Subblock 21 : (C_ns2)^de_{j in P} */
  nzmax += np;        /* Subblock 22 : -1 */
  nzmax += np;        /* Subblock 23 : (C_tfx)^de_{j in P} */
  nzmax += np;        /* Subblock 24 : (C_tfy)^de_{j in P} */
  nzmax += (4*nd)*MAX_CONTACTS;        /* Subblock 25 : (Z_j)^re_{j in Pll} */
  nzmax += 4*MAX_CONTACTS;      /* Subblock 26 :-1 */
  


  double tot_mass = 0;
  for (int j = 0; j < pymCfg->nBody; ++j) {
    pym_rb_named_t *rbn2 = &pymCfg->body[j].b;
    tot_mass += rbn2->m;
  }
  //    printf("    A matrix body name           : %s\n", rbn->name);
  //    printf("    A matrix constants (np)      : %d\n", np);
  //    printf("    A matrix constants (nmi)     : %d\n", nmi);
  //    printf("    A matrix subblock dimension  : %d x %d\n", sd->Asubrows, sd->Asubcols);
  //    printf("    A matrix size                : %d x %d\n", sd->Ari[sd->Asubrows], sd->Aci[sd->Asubcols]);
  cholmod_triplet *AMatrix_trip =
    cholmod_allocate_triplet(sd->Ari[sd->Asubrows],
			     sd->Aci[sd->Asubcols],
			     nzmax, 0, CHOLMOD_REAL, cc);
  assert(AMatrix_trip->nnz == 0);
  int i, j;
  const double h = pymCfg->h;
  const double mu = pymCfg->mu;
  /* Sub-block 01 : M/(h*h) */
  for (i=0;i<3;++i) {
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 0, i, i,
			     sd->M[i][i] / (h*h));
    for (j=0;j<3;++j) {
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 0, i+3, j+3,
			       sd->M[i+3][j+3] / (h*h));
    }
  }
  /* Sub-block 02 : -1 */
  for (i=0;i<nd*np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 1, i%nd, i, -1);
  /* Sub-block 03 : -1 */
  for (i=0;i<nd*nmi;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 9, i%nd, i, -1);
  /* Sub-block 04  : -1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 16, i, i, -1);
  /* Sub-block 04a  : -1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 0, 19, i, i, -1);
  /* Sub-block 05 : -1 */
  for (i=0;i<nd*np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 1, 1, i, i, -1);
  /* Sub-block 06 : Q_j */
  FOR_0(j, np) {
    cholmod_sparse *Qj = sd->Q[j];
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
  /* Sub-block 07 : 1 */
  for (i=0;i<4*np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 2, 3, i, i, 1);
  /* Sub-block 08 : -1 */
  for (i=0;i<4*np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 2, 4, i, i, -1);
  /* Sub-block 09 : C_mu */
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
  /* Sub-block 10 : -1 */
  for (i=0;i<np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 3, 6, i, i, -1);
  /* Sub-block 11 : Z_j */
  FOR_0(j, np) {
    cholmod_sparse *Zj = sd->Z[j];
    cholmod_triplet *Zj_trip = cholmod_sparse_to_triplet(Zj, cc);
    FOR_0(i, Zj_trip->nnz) {
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 4, 0,
			       4*j  + ((int *)(Zj_trip->i))[i],
			       0    + ((int *)(Zj_trip->j))[i],
			       ((double *)(Zj_trip->x))[i] );
    }
    cholmod_free_triplet(&Zj_trip, cc);
  }
  /* Sub-block 12 : -1 */
  for (i=0;i<4*np;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 4, 3, i, i, -1);
  /* Sub-block 13 : 1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 5, 0, i, i, 1);
  /* Sub-block 14 : -1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 5, 7, i, i, -1);
  /* Sub-block 15 : C_n */
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
  /* Sub-block 16 (similar to sub-block 10) : Z_j */
  FOR_0(j, na) {
    cholmod_sparse *Zj = sd->Za[j];
    cholmod_triplet *Zj_trip = cholmod_sparse_to_triplet(Zj, cc);
    FOR_0(i, Zj_trip->nnz) {
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 7, 0,
			       4*j  + ((int *)(Zj_trip->i))[i],
			       0    + ((int *)(Zj_trip->j))[i],
			       ((double *)(Zj_trip->x))[i] );
    }
    cholmod_free_triplet(&Zj_trip, cc);
  }
  /* Sub-block 17 (similar to sub-block 11) : -1 */
  for (i=0;i<4*na;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 7, 10, i, i, -1);
  /* Sub-block 18 : 1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 8, 0, i, i, 1);
  /* Sub-block 19 : -1 */
  for (i=0;i<nd;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 8, 12, i, i, -1);
  /* Sub-block 20 : C_ns */
  for (i=0;i<np;++i) {
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 9, 2, i, 5*i + 4, pymCfg->h);
  }
  /* Sub-block 21 : C_ns2 */
  for (i=0;i<np;++i) {
    //const double pendep_coeff = pymCfg->h*SPRING_K + 2*sqrt(rbn->m/np * SPRING_K);
    const double pendep_coeff = pymCfg->h*SPRING_K + 2*SPRING_C;
    //printf("pendep_coeff = %lf\n", pendep_coeff);
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 9, 3, i, 4*i + 2, pendep_coeff);
  }
  /* Sub-block 22 : -1 */
  for (i=0;i<np;++i) {
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 9, 15, i, i, -1);
  }
  /* Sub-block 23 : C_tfx */
  for (i=0;i<np;++i) {
    if (sd->contactTypes_1[i] == dynamic_contact) {
      const int conidx = sd->contactIndices_1[i];
      const double cx0 = sd->contacts_0[conidx][0];
      const double cx1 = sd->contacts_1[conidx][0];
      double coeff = -pymCfg->mu * (cx1-cx0) / h;
      //printf("dx_coeff = %lf\n", coeff);
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 10, 2, i, 5*i + 0, -1);
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 10, 2, i, 5*i + 4, coeff);
    }
  }
  /* Sub-block 24 : C_tfy */
  for (i=0;i<np;++i) {
    if (sd->contactTypes_1[i] == dynamic_contact) {
      const int conidx = sd->contactIndices_1[i];
      const double cy0 = sd->contacts_0[conidx][1];
      const double cy1 = sd->contacts_1[conidx][1];
      double coeff = -pymCfg->mu * (cy1-cy0) / h;
      //printf("dy_coeff = %lf\n", coeff);
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 11, 2, i, 5*i + 1, -1);
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 11, 2, i, 5*i + 4, coeff);
    }
  }
  /* Sub-block 25 (similar to sub-block 10) : Z_j */
  FOR_0(j, MAX_CONTACTS) {
    cholmod_sparse *Zj = sd->Zall[j];
    cholmod_triplet *Zj_trip = cholmod_sparse_to_triplet(Zj, cc);
    FOR_0(i, Zj_trip->nnz) {
      SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 13, 0,
        4*j  + ((int *)(Zj_trip->i))[i],
        0    + ((int *)(Zj_trip->j))[i],
        ((double *)(Zj_trip->x))[i] );
    }
    cholmod_free_triplet(&Zj_trip, cc);
  }
  /* Sub-block 26 : -1 */
  for (i=0;i<4*MAX_CONTACTS;++i)
    SET_TRIPLET_RCV_SUBBLOCK(AMatrix_trip, sd, 13, 17, i, i, -1);
  *AMatrix = AMatrix_trip;
  // cholmod_print_triplet(AMatrix_trip, rbn->name, cc);
}

void GetEta(double **_eta, const pym_rb_statedep_t *sd,
	    const pym_rb_t *rb, const pym_config_t *pymCfg,
	    cholmod_common *cc) {
  const pym_rb_named_t *rbn = &rb->b;
  const double h = pymCfg->h;
  /* We can calculate optimal estimate for the number of nonzero elements */
  const int nd = 3 + pymCfg->nrp;
  const int np = sd->nContacts_1;
  const int na = rbn->nAnchor;
  size_t etaDim = sd->Ari[sd->Asubrows];
  //printf("RB %s etaDim = %d\n", rbn->name, etaDim);
  double *eta = (double *)malloc(sizeof(double) * etaDim);
  memset(eta, 0, sizeof(double) * etaDim);

  double chi_1[3+4], chi_0[3+4];
  // chi_1: current frame
  for (int i = 0; i < 3; ++i) chi_1[i] = rbn->p[i];
  for (int i = 0; i < 4; ++i) chi_1[i + 3] = rbn->q[i];
  // chi_0: previous frame
  for (int i = 0; i < 3; ++i) chi_0[i] = rbn->p[i] - h*rbn->pd[i];
  for (int i = 0; i < 4; ++i) chi_0[i + 3] = rbn->q[i] - h*rbn->qd[i];


  double tot_mass = 0;
  for (int j = 0; j < pymCfg->nBody; ++j) {
    pym_rb_named_t *rbn2 = &pymCfg->body[j].b;
    tot_mass += rbn2->m;
  }

  int i, j;

  /* c_{2,i} */
  FOR_0(i, nd) {
    double M_q_q0 = 0;
    FOR_0(j, nd) {
      M_q_q0 += (sd->M[i][j] * (2*chi_1[j] - chi_0[j]))/(h*h);
    }
    eta[sd->Ari[0] + i] = M_q_q0 + (-sd->Cqd[i] + /*sd->f_g[i] +*/ sd->f_ext[i]);
  }
  /* p^(l)_{cfix,i}   and   (-V_ij)re_{j \in P} */
  FOR_0(i, np) {
    FOR_0(j, 4) {
      eta[sd->Ari[2] + 4*i + j] = sd->contactsFix_1[i][j];
      eta[sd->Ari[4] + 4*i + j] = -sd->V[i][j];
    }
  }
  /* chi^(l)_{i, ref} */
  FOR_0(i, nd) {
    eta[sd->Ari[5] + i] = rbn->chi_ref[i];
    /* if (i==2) { */
    /*   /\* TODO Ground slant parameter - ground level calculation *\/ */
    /*   const double ctY = rb->b.p[1]; */
    /*   const double theta = pymCfg->slant; */
    /*   const double z = -ctY*tan(theta); */
    /*   eta[sd->Ari[5] + i] += z; */
    /* } */
  }
  /* (-V_ij)re_{j \in A} */
  FOR_0(i, na) {
    FOR_0(j, 4) {
      eta[sd->Ari[7] + 4*i + j] = -sd->Va[i][j];
    }
  }
  /* chi^(l)_{i} */
  FOR_0(i, nd) {
    if (i < 3)
      eta[sd->Ari[8] + i] = rbn->p[i];
    else
      eta[sd->Ari[8] + i] = rbn->q[i-3];
  }
  double bipTotMass = 0;
  for (j=0; j < pymCfg->nBody; ++j) {
    pym_rb_named_t *rbn2 = &pymCfg->body[j].b;
    bipTotMass += rbn2->m;
  }
  /* ns */
  FOR_0(i, np) {
    const int conidx = sd->contactIndices_1[i];
    const double pendep = sd->contacts_1[conidx][2]; // penetration depth at current time step
    //printf("pendep = %lf\n", pendep);
    assert(pendep <= 0);
    //double v = 2.0*sqrt(rbn->m/np * SPRING_K)*pendep;
    double v = 2.0*SPRING_C*pendep;
    eta[sd->Ari[9] + i] = v;
  }
  /* (-V_ij)re_{j \in Pall} */
  FOR_0(i, MAX_CONTACTS) {
    FOR_0(j, 4) {
      eta[sd->Ari[13] + 4*i + j] = -sd->Vall[i][j];
    }
  }
  *_eta = eta;
  //__PRINT_VECTOR(eta, etaDim);
}

static inline double Norm3(const double *const v) {
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

bool check_near_2pi(double v, double how_near) {
  assert(v >= 0 && how_near >= 0);
  int n_2pi = (int)(v / (2*M_PI));
  if (n_2pi > 0) {
    double dist1 = v - n_2pi*(2*M_PI);
    double dist2 = 2*M_PI - dist1;
    assert(dist1 >= 0 && dist2 >= 0);
    if (dist1 < how_near/2 || dist2 < how_near/2)
      return true;
  } else {
    double dist2 = 2*M_PI - v;
    //assert(dist2 >= 0);
    if (dist2 < how_near/2 || dist2 < 0)
      return true;
  }
  return false;
}

int PymCheckRotParam(pym_config_t *pymCfg) {
  int j;
  const int nb = pymCfg->nBody;
  FOR_0(j, nb) {
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    double th_1 = Norm3(rbn->q);
    bool th_1_near = check_near_2pi(th_1, 1e-2);
    if (th_1_near) {
      printf("Error - %s rotation parameterization failure:\n", rbn->name);
      printf("        th_1 = %e (%e, %e, %e)\n",
        th_1, rbn->q[0], rbn->q[1], rbn->q[2]);
      return -1;
    }
  }
  return 0;
}

std::ostream & pym_print_detailed_rb_state( std::ostream &s, const pym_rb_t &rb )
{
  const pym_rb_named_t &rbn = rb.b;
  s << rb << std::endl;
  if (rb.b.rotParam == RP_EXP) {
    double th = PymNorm(3, rbn.q);
    s << rbn.name << " p [" << rbn.p[0] << ", " << rbn.p[1] << ", " << rbn.p[2]
    << "] / q [" << rbn.q[0] << ", " << rbn.q[1] << ", " << rbn.q[2] << "] th0=" << th << " (RP_EXP)";
  } else if (rb.b.rotParam == RP_QUAT_WFIRST) {
    assert(!"Not implemented yet.");
  }
  return s;
}
