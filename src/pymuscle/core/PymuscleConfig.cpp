#include "PymCorePch.h"
#include "PymStruct.h"
#include "MathUtil.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "StateDependents.h"
#include "SymbolicTensor.h"
#include "Config.h"
#include "CholmodMacro.h"
#include "dRdv_real.h"
#include "PymuscleConfig.h"
#include "PymPodUtil.h"

void PrintRigidBody(const pym_rb_t *rb) {
  int i; for (i=0; i<18; ++i) printf("%2d : %20lf\n", i, rb->a[i]);
}

int PymDestoryConfig(pym_config_t *pymCfg) {
  free(pymCfg->body) ;
  free(pymCfg->fiber) ;
  pymCfg->body = 0 ;
  pymCfg->fiber = 0 ;
  pymCfg->nBody = 0 ;
  pymCfg->nFiber = 0 ;
  return 0;
}

void pym_rotparam_from_string(pym_rot_param_t &rp, int &nrp, const char *rotParamStr)
{
  if (strcmp(rotParamStr, "QUAT_WFIRST") == 0)    { rp = RP_QUAT_WFIRST; nrp = 4; }
  else if (strcmp(rotParamStr, "EXP") == 0)       { rp = RP_EXP;         nrp = 3; }
  else if (strcmp(rotParamStr, "EULER_XYZ") == 0) { rp = RP_EULER_XYZ;   nrp = 3; }
  else if (strcmp(rotParamStr, "EULER_ZXZ") == 0) { rp = RP_EULER_ZXZ;   nrp = 3; }
  else assert(0);
}

int PymConstructConfig(const char *fnConf, pym_config_t *pymCfg, FILE *verbosestream) {
#define csgfe(a,b) config_setting_get_float_elem(a,b)
#define BODY(j,ai) body[j].a[ai]
#define FIBER(j,ai) fiber[j].a[ai]
  int j, k;
  config_t conf;
  config_init(&conf);
  config_set_auto_convert(&conf, 1);
  printf("Opening %s...\n", fnConf);
  int confret = config_read_file(&conf, fnConf);
  if (confret != CONFIG_TRUE)
  {
    const char *errText = config_error_text(&conf);
    const int errLine = config_error_line(&conf);
    /*
    * TODO libconfig version mismatch
    *
    * config_error_file() function available on
    * libconfig-1.3.2 but available in 1.4.5.
    * Since Ubuntu 10.04 (Lucid) supports 1.3.2
    * officially and this function is not that
    * important, we get rid of this function
    * temporarily.
    */
    //const char *errFile = config_error_file(&conf);
    //printf("Configuration file %s (line %d) %s!\n", errFile, errLine, errText);
    printf("Configuration file ?? (line %d) %s!\n", errLine, errText);
    config_destroy(&conf);
    return (-10);
  }

  const char *ver;
  config_lookup_string(&conf, "version", &ver);
  printf("  Config file version: %s\n", ver);

  confret = config_lookup_float(&conf, "h", &pymCfg->h);
  assert(confret == CONFIG_TRUE);
  const double h = pymCfg->h;
  confret = config_lookup_float(&conf, "mu", &pymCfg->mu);
  const char *rotParamStr = 0;
  confret = config_lookup_string(&conf, "rotParam", &rotParamStr);
  pym_rot_param_t rp = RP_UNKNOWN;
  int nrp = -1;
  if (confret == CONFIG_TRUE) {
    pym_rotparam_from_string(rp, nrp, rotParamStr);
  } else {
    /* Exp Rot is default rotation parameterization! */
    rp = RP_EXP;
    nrp = 3;
  }
  pymCfg->rotparam = rp;
  pymCfg->nrp = nrp;
  long joint_constraints;
  confret = config_lookup_int(&conf, "jointConstraints", &joint_constraints);
  if (confret == CONFIG_TRUE)
    pymCfg->joint_constraints = joint_constraints;
  else
    pymCfg->joint_constraints = 0;
  
  /* TODO Ground slant parameter */
  pymCfg->slant = 0.0;

  /*
  * Parse body configurations
  */
  config_setting_t *bodyConf = config_lookup(&conf, "body");
  assert(bodyConf);
  const int nBody = config_setting_length(bodyConf);
  pym_rb_t body[128 /*nBody*/];
  memset(body, 0, sizeof(pym_rb_t)*nBody);
  double extForce[128 /*nBody*/][6];
  char bodyNames[128 /*nBody*/][128];
  memset(extForce, 0, sizeof(double)*nBody*6); /* NOTE: No external force for now. */
  for (j = 0; j < nBody; ++j)
  {
    pym_rb_named_t *bjb = &body[j].b;
    bjb->nAnchor = 0;
    config_setting_t *bConf = config_setting_get_elem(bodyConf, j);
    const char *bName;
    config_setting_lookup_string(bConf, "name", &bName);
    strncpy(bodyNames[j], bName, 128);
    strncpy(bjb->name, bName, 128);
    //printf("    Body %3d : %s\n", j, bName);
    const char *rotParamStr;
    config_setting_lookup_string(bConf, "rotParam", &rotParamStr);
    pym_rot_param_t rp = RP_UNKNOWN;
    int nrp = -1; /* Numer of Rotation Parameters */
    pym_rotparam_from_string(rp, nrp, rotParamStr);
    int i;

    bjb->rotParam = rp;
    config_setting_t *p    = config_setting_get_member(bConf, "p");    assert(p);    for (i=0;i<3;  ++i) bjb->p[i]       = csgfe(p, i);
    config_setting_t *q    = config_setting_get_member(bConf, "q");    assert(q);    for (i=0;i<nrp;++i) bjb->q[i]       = csgfe(q, i);
    config_setting_t *pd   = config_setting_get_member(bConf, "pd");   assert(pd);   for (i=0;i<3;  ++i) bjb->pd[i]      = csgfe(pd, i);
    config_setting_t *qd   = config_setting_get_member(bConf, "qd");   assert(qd);   for (i=0;i<nrp;++i) bjb->qd[i]      = csgfe(qd, i);
    config_setting_t *size = config_setting_get_member(bConf, "size"); assert(size); for (i=0;i<3;  ++i) bjb->boxSize[i] = csgfe(size, i);
    config_setting_t *mass = config_setting_get_member(bConf, "mass"); assert(mass); bjb->m = config_setting_get_float(mass);
    BoxInertiaTensorFromMassAndSize(bjb->I, bjb->m, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2]);
    const double volume = bjb->boxSize[0] * bjb->boxSize[1] * bjb->boxSize[2];
    SymbolicTensor(bjb->Ixyzw, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2], bjb->m/volume);
    BoxCorners(bjb->corners, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2]);
    config_setting_t *cExtForce = config_setting_get_member(bConf, "extForce");
    if (cExtForce)
    {
      extForce[j][0] = csgfe(cExtForce, 0);
      extForce[j][1] = csgfe(cExtForce, 1);
      extForce[j][2] = csgfe(cExtForce, 2);
    }
    config_setting_t *cExtTorque = config_setting_get_member(bConf, "extTorque");
    if (cExtTorque)
    {
      extForce[j][3] = csgfe(cExtTorque, 0);
      extForce[j][4] = csgfe(cExtTorque, 1);
      extForce[j][5] = csgfe(cExtTorque, 2);
    }
    /* duplicate initial states so that 'Reset' function can work. */
    memcpy(bjb->p_simconf, bjb->p, sizeof(double)*3);
    memcpy(bjb->q_simconf, bjb->q, sizeof(double)*4);
    memcpy(bjb->pd_simconf, bjb->pd, sizeof(double)*3);
    memcpy(bjb->qd_simconf, bjb->qd, sizeof(double)*4);
    /*
    * Add gravitational force as external force if grav field is true.
    */
    config_setting_t *grav = config_setting_get_member(bConf, "grav");
    if (grav && config_setting_get_bool(grav))
    {
      extForce[j][2] += bjb->m * (-9.81);
    }

    bjb->nFiber = 0; /* will be set after loading muscle fiber sections of configuration file */
    FOR_0(k, 3) {
      bjb->p0[k] = bjb->p[k] - h*bjb->pd[k];
      //assert(bjb->p[k] == bjb->p0[k]);
    }
    FOR_0(k, 4) {
      bjb->q0[k] = bjb->q[k] - h*bjb->qd[k];
      //assert(bjb->q[k] == bjb->q0[k]);
    }
  }
  /*
  * Parse muscle fiber configurations
  */
  config_setting_t *muscleConf = config_lookup(&conf, "muscle");
  assert(muscleConf);
  const int nMuscle = config_setting_length(muscleConf);
  pym_mf_t fiber[1024 /*nMuscle*/];
  unsigned int musclePair[1024 /*nMuscle*/][2];
  char muscleName[1024 /*nMuscle*/][128];
  int nActMuscle = 0, nLigMuscle = 0;
  for (j = 0; j < nMuscle; ++j)
  {
    pym_mf_named_t  *mfn = &fiber[j].b;
    config_setting_t *mConf = config_setting_get_elem(muscleConf, j);
    const char *mName, *mType;
    config_setting_lookup_string(mConf, "name", &mName);
    strncpy(muscleName[j], mName, 128);
    strncpy(mfn->name, mName, 128);
    config_setting_lookup_string(mConf, "mType", &mType);
    if (strcmp(mType, "MUSCLE") == 0) {
      mfn->mType = PMT_ACTUATED_MUSCLE;
      nActMuscle++;
    } else if (strcmp(mType, "LIGAMENT") == 0) {
      mfn->mType = PMT_LIGAMENT;
      nLigMuscle++;
    } else {
      assert(0);
    }
    //printf("    Fiber %3d : %s\n", j, mName);
    config_setting_t *KSE = config_setting_get_member(mConf, "KSE"); assert(KSE);
    FIBER(j,0) = config_setting_get_float(KSE);
    config_setting_t *KPE = config_setting_get_member(mConf, "KPE"); assert(KPE);
    FIBER(j,1) = config_setting_get_float(KPE);
    config_setting_t *b = config_setting_get_member(mConf, "b"); assert(b);
    FIBER(j,2) = config_setting_get_float(b);
    config_setting_t *xrest = config_setting_get_member(mConf, "xrest"); assert(xrest);
    FIBER(j,3) = config_setting_get_float(xrest);
    config_setting_t *T = config_setting_get_member(mConf, "T"); assert(T);
    FIBER(j,4) = config_setting_get_float(T);
    config_setting_t *A = config_setting_get_member(mConf, "A"); assert(A);
    FIBER(j,5) = config_setting_get_float(A);

    config_setting_t *originPos = config_setting_get_member(mConf, "originPos"); assert(originPos);
    FIBER(j,6) = csgfe(originPos, 0);
    FIBER(j,7) = csgfe(originPos, 1);
    FIBER(j,8) = csgfe(originPos, 2);
    config_setting_t *insertionPos = config_setting_get_member(mConf, "insertionPos"); assert(insertionPos);
    FIBER(j,9) = csgfe(insertionPos, 0);
    FIBER(j,10) = csgfe(insertionPos, 1);
    FIBER(j,11) = csgfe(insertionPos, 2);


    const char *orgBodyName, *insBodyName;
    config_setting_lookup_string(mConf, "origin", &orgBodyName);
    config_setting_lookup_string(mConf, "insertion", &insBodyName);
    int boIdx = FindBodyIndex(nBody, bodyNames, orgBodyName);
    int biIdx = FindBodyIndex(nBody, bodyNames, insBodyName);
    assert(boIdx >= 0 && biIdx >= 0);
    assert(boIdx != biIdx);
    musclePair[j][0] = boIdx;
    musclePair[j][1] = biIdx;
    mfn->org = boIdx;
    mfn->ins = biIdx;
    /*
    * TODO [TUNE] Muscle fiber rest length bounds
    */
    mfn->xrest_lower = mfn->xrest*0.5;
    mfn->xrest_upper = mfn->xrest*2.0;
    pym_rb_named_t *bjbOrg = &body[ boIdx ].b;
    pym_rb_named_t *bjbIns = &body[ biIdx ].b;
    bjbOrg->fiber[ bjbOrg->nFiber ] = j; ++bjbOrg->nFiber;
    bjbIns->fiber[ bjbIns->nFiber ] = j; ++bjbIns->nFiber;
  }
  
  printf("Info - # of RB parsed = %d\n", nBody);
  printf("Info - # of MF parsed = %d\n", nMuscle);
  printf("          actuated    = %d\n", nActMuscle);
  printf("          ligament    = %d\n", nLigMuscle);
  pymCfg->nJoint = 0;
  long simFrame; /* simulation length */
  confret = config_lookup_int(&conf, "simFrame", &simFrame);
  assert(confret == CONFIG_TRUE);
  pymCfg->nSimFrame = simFrame;
  long plotSamplingRate; /* Write 1 sample per 'plotSamplingRate' */
  confret = config_lookup_int(&conf, "plotSamplingRate", &plotSamplingRate);
  assert(confret == CONFIG_TRUE);
  char cOutput[128]; /* File name result is written */
  const char *cOutputTemp;
  confret = config_lookup_string(&conf, "output", &cOutputTemp);
  assert(confret == CONFIG_TRUE);
  strncpy(cOutput, cOutputTemp, 128);
  config_destroy(&conf);

  for (j = 0; j < nBody; ++j)
  {
    if (body[j].b.rotParam == RP_QUAT_WFIRST) {
      /*
      * Normalize the quaternions for each body
      * to ensure they represent rotations.
      */
      NormalizeVector(4, body[j].b.q);
      NormalizeVector(4, body[j].b.q0);
    }
  }

  /*
  * Return body, bodyName, extForce, muscle, musclePair
  */
  pymCfg->nBody  = nBody ;
  pymCfg->nFiber = nMuscle ;
  pymCfg->body   = (pym_rb_t *)malloc(sizeof(pym_rb_t)*nBody  ) ;
  pymCfg->fiber  = (pym_mf_t   *)malloc(sizeof(pym_mf_t)*(nMuscle + 100 /* reservation for joint muscles */)) ;
  memcpy(pymCfg->body , body , sizeof(pym_rb_t)*nBody  );
  memcpy(pymCfg->fiber, fiber, sizeof(pym_mf_t  )*nMuscle);

  int totFiber = 0;
  FOR_0(j, pymCfg->nBody) {
    fprintf(verbosestream,
      "   %s has %d fibers.\n", pymCfg->body[j].b.name, pymCfg->body[j].b.nFiber);
    assert(pymCfg->body[j].b.nFiber <= MAX_FIBER_PER_RB);
    totFiber += pymCfg->body[j].b.nFiber;
  }
  assert(totFiber == 2*nMuscle);

  pymCfg->chInputLen      = 0;
  pymCfg->chOutputLen     = 0;
  pymCfg->prevTotContacts = 0;
  pymCfg->curTotContacts  = 0;
  pymCfg->renderFibers = 0;
  return 0;
#undef csgfe
#undef BODY
#undef FIBER
}

void PymConvertRotParamInPlace(pym_config_t *pymCfg, pym_rot_param_t targetRotParam) {
  for (int i = 0; i < pymCfg->nBody; ++i) {
    if (pymCfg->body[i].b.rotParam == targetRotParam)
      continue;
    if (pymCfg->body[i].b.rotParam == RP_QUAT_WFIRST && targetRotParam == RP_EXP) {
      double	v[3];
      /* chi 1 rot */
      QuatToV(v, pymCfg->body[i].b.q);
      memcpy(pymCfg->body[i].b.q, v, sizeof(double)*3);
      pymCfg->body[i].b.q[3]  = DBL_MAX;	/* Should be ignored */
      /* chi 1 rot (simconf) */
      QuatToV(v, pymCfg->body[i].b.q_simconf);
      memcpy(pymCfg->body[i].b.q_simconf, v, sizeof(double)*3);
      pymCfg->body[i].b.q_simconf[3] = DBL_MAX;	/* Should be ignored */
      /* chi 0 rot */
      QuatToV(v, pymCfg->body[i].b.q0);
      memcpy(pymCfg->body[i].b.q0, v, sizeof(double)*3);
      pymCfg->body[i].b.q0[3] = DBL_MAX;	/* Should be ignored */
      pymCfg->body[i].b.rotParam = RP_EXP;
      //printf("%s - %d\n", pymCfg->body[i].b.name, pymCfg->body[i].b.rotParam);
    } else {
      assert(!"Not implemented case.");
    }
  }
}

int pym_check_coincided_fibers(int *mf_indices, const pym_rb_statedep_t *sd_all, const pym_config_t *pymCfg) {
  const int			 nf	      = pymCfg->nFiber;
  const int			 nd	      = 3 + pymCfg->nrp;
  int mfsize = 0;
  for (int i = 0; i < nf; ++i) {
    const double		*attPos1, *attPos2;
    pym_mf_named_t *mf = &pymCfg->fiber[i].b;
    attPos1				      = mf->fibb_org;
    attPos2				      = mf->fibb_ins;
    double	pt1[3], pt2[3], direction[3];
    AffineTransformPoint(pt1, sd_all[mf->org].W_1, attPos1);
    AffineTransformPoint(pt2, sd_all[mf->ins].W_1, attPos2);
    for (int k = 0; k < 3; ++k)
      direction[k] = pt2[k] - pt1[k];
    double direction_original[3];
    memcpy(direction_original, direction, sizeof(double)*3);
    double	dirLen = PymNorm(3, direction);
    double	gf[7] = {0,};
    if (dirLen <= 1e-5) {
      printf("WARN - Both ends of fiber %s are coincided. This fiber will not have any effect.\n", mf->name);
      mf_indices[mfsize] = i;
      ++mfsize;
    }
  }
  return mfsize;
}

void GetR_i(cholmod_triplet **_R_i, const pym_rb_statedep_t *sd,
  const int i /* RB index */, const pym_config_t *pymCfg, cholmod_common *cc) {
    int				 j, k;
    /*
    * PARAMETER   i   global RB index
    * LOCAL       j   local MF index of RB i
    * LOCAL       k   <generic iterate variable>
    */
    const pym_rb_named_t		*rbn	      = &pymCfg->body[i].b;
    const int			 nf	      = pymCfg->nFiber;
    const int			 nd	      = 3 + pymCfg->nrp;
    cholmod_triplet		*R_i	      = cholmod_allocate_triplet(nd*rbn->nFiber, nf, nd*rbn->nFiber, 0, CHOLMOD_REAL, cc);
    FOR_0(j, rbn->nFiber) {
      int				 fidx	      = rbn->fiber[j];	/* global MF index of local MF j of RB i */
      assert(fidx < nf);
      const double		*attPos1, *attPos2;
      int				 oppositeBidx = -1;
      const pym_mf_named_t	*mf	      = &pymCfg->fiber[ fidx ].b;
      if (i == mf->org) {
        attPos1				      = mf->fibb_org;
        attPos2				      = mf->fibb_ins;
        oppositeBidx			      = mf->ins;
      } else if (i == mf->ins) {
        attPos1				      = mf->fibb_ins;
        attPos2				      = mf->fibb_org;
        oppositeBidx			      = mf->org;
      } else {
        assert (0);
      }
      double	pt1[3], pt2[3], direction[3];
      AffineTransformPoint(pt1, sd[i].W_1, attPos1);
      AffineTransformPoint(pt2, sd[oppositeBidx].W_1, attPos2);
      FOR_0(k, 3) direction[k] = pt2[k] - pt1[k];
      double direction_original[3];
      memcpy(direction_original, direction, sizeof(double)*3);
      double	dirLen = PymNorm(3, direction);
      double	gf[7] = {0,};
      if (dirLen > 1e-5) {
        NormalizeVector(3, direction);
        if (rbn->rotParam == RP_EXP)
          GeneralizedForce(gf, rbn->q, direction, attPos1);
        else if (rbn->rotParam == RP_QUAT_WFIRST)
          GeneralizedForceQuat(gf, rbn->q, direction, attPos1);
        else
          assert(!"Not the case");
      } else {
        printf("WARN - Both ends of fiber %s are coincided. This fiber will not have any effect.\n", mf->name);
      }
      FOR_0(k, nd)
        SET_TRIPLET_RCV(R_i, nd*j + k, fidx, gf[k]);
    }

    //    /**** DEBUG ***/
    //    printf("====R[%d]====\n", i);
    //    cholmod_sparse *R_i_sp = cholmod_triplet_to_sparse(R_i, R_i->nnz, cc);
    //    PrintEntireSparseMatrix(R_i_sp);
    //    cholmod_free_sparse(&R_i_sp, cc);

    *_R_i = R_i;
}

void PymCalculateRefCom(double *refCom, const pym_config_t *const pymCfg) {
  int				i, j;
  double			totMass	 = 0;
  FOR_0(j, 3)
    refCom[j]				 = 0;
  FOR_0(i, pymCfg->nBody) {
    const pym_rb_named_t *const rbn	 = &pymCfg->body[i].b;
    FOR_0(j, 3)
      refCom[j]				+= rbn->m * rbn->chi_ref[j];
    totMass				+= rbn->m;
  }
  FOR_0(j, 3)
    refCom[j] /= totMass;
}

void PymConstructBipedEqConst(pym_biped_eqconst_t	*bod,
  const pym_rb_statedep_t	*sd,
  const pym_config_t	*pymCfg,
  cholmod_common *cc)
{
  const int						 nb = pymCfg->nBody;
  const int						 nf = pymCfg->nFiber;
  const int						 nj = pymCfg->nJoint;
  cholmod_triplet			 *A_trip[128 /*nb*/];	/* should be deallocated here */
  const int						 nd = 3 + pymCfg->nrp;
  int deg_mf_indices[1024]; // degenerated muscles (coincided muscles) indices
  const int ndf = pym_check_coincided_fibers(deg_mf_indices, sd, pymCfg);
  assert(ndf <= 1024);
  int Asubrowsizes[ ]  = { 0,	/* be calculated later */
    2*nd*nf,
    nf,
    4*nj,
    nj,
    3,
    3,
    3,
    2*nd*ndf,
  };
  int Asubcolsizes[ ]  = { 0,	/* be calculated later */
    nf,                   /* muscle(lig+act) tension */
    nf,                   /* muscle(lig+act) actuation */
    nf,                   /* muscle(lig+act) rest length */
    1,                    /* \epsilon_{ligten} */
    1,                    /* \epsilon_{actten} */
    4*nj,                 /* anchored joint dislocation vector */
    nj,                   /* \epsilon_{d} */
    3,                    /* p_com^{l+1} */
    3,                    /* \Delta p_{com,ref} */
    1,                    /* \epsilon_{com} */
    3,                    /* \tau_{com} */
    1,                    /* \epsilon_{tau,com} */
    1,                    /* \epsilon_{ligact} */
    1,                    /* \epsilon_{actact} */
    nf,                   /* \epsilon_{T} */
    nf,                   /* \epsilon_{u} */
    2*4*ndf,              /* f_TC */
  };
  BOOST_STATIC_ASSERT(sizeof(int) + sizeof(Asubrowsizes) == sizeof(bod->Ari));
  BOOST_STATIC_ASSERT(sizeof(int) + sizeof(Asubcolsizes) == sizeof(bod->Aci));

  int	i, j, k, l;
  FOR_0(i, nb) {
    GetAMatrix(A_trip + i, sd + i, pymCfg->body + i, pymCfg, cc);
    Asubrowsizes[0]   += A_trip[i]->nrow;
    Asubcolsizes[0]   += A_trip[i]->ncol;
  }
  const int	 Asubrows     = sizeof(Asubrowsizes)/sizeof(int);
  const int	 Asubcols     = sizeof(Asubcolsizes)/sizeof(int);
  int		*Ari	      = bod->Ari;
  int		*Aci	      = bod->Aci;
  Ari[0]		      = 0;
  Aci[0]		      = 0;
  FOR_0(i, Asubrows) Ari[i+1] = Ari[i] + Asubrowsizes[i];
  FOR_0(i, Asubcols) Aci[i+1] = Aci[i] + Asubcolsizes[i];

  int	*Airi = (int *)malloc( sizeof(int)*(1+nb) );
  int	*Aici		 = (int *)malloc( sizeof(int)*(1+nb) );
  bod->Airi		 = Airi;
  bod->Aici		 = Aici;
  Airi[0]		 = 0;
  Aici[0]		 = 0;
  FOR_0(i, nb) Airi[i+1] = Airi[i] + A_trip[i]->nrow;
  FOR_0(i, nb) Aici[i+1] = Aici[i] + A_trip[i]->ncol;

  size_t	nzmax	     = 0;
  size_t	totFiberCon  = 0;
  int nTotContacts = 0;
  FOR_0(i, nb) {
    nzmax		    += A_trip[i]->nnz;	/* Sub-block 01 - A_i */
    nzmax		    += nd*pymCfg->body[i].b.nFiber;	/* Sub-block 02 - [0 -1] */
    nzmax		    += nd*pymCfg->body[i].b.nFiber;	/* Sub-block 03 - R_i */
    
    totFiberCon += pymCfg->body[i].b.nFiber;
    nTotContacts += sd[i].nContacts_1;
  }
  assert(totFiberCon == 2*nf); /* Since a fiber always connected
                               to two different RB */
  nzmax += 3*nf;		/* Sub-block 04, 05, 06 - K_11, K_12, K_13 */
  nzmax += 2*4*nj;		/* sub-block 07 - D */
  nzmax += 4*nj;		/* sub-block 08 - (-1) */
  nzmax += nj;			/* sub-block 09 - (1) */
  nzmax += 3*nb;		/* sub-block 10 - A_com */
  nzmax += 3;			/* sub-block 11 - 1*M */
  nzmax += 3;			/* sub-block 12 - 1 */
  nzmax += 3;			/* sub-block 13 - -1 */
  nzmax += nd*nTotContacts;	/* sub-block 14 - G */
  nzmax += 3;			/* sub-block 15 - -1 */

  //    printf("    BipA matrix constants (nb)      : %d\n", nb);
  //    printf("    BipA matrix constants (nf)      : %d\n", nf);
  //    printf("    BipA matrix subblock dimension  : %d x %d\n", Asubrows, Asubcols);
  //    printf("    BipA matrix size                : %d x %d\n", Ari[Asubrows], Aci[Asubcols]);
  cholmod_triplet	*AMatrix_trip = cholmod_allocate_triplet(Ari[Asubrows], Aci[Asubcols], nzmax, 0, CHOLMOD_REAL, cc);
  assert(AMatrix_trip->nnz == 0);

  for (int i = 0; i < nf; ++i) {
    pymCfg->fiber[i].b.degenerated = false;
  }
  for (int i = 0; i < ndf; ++i) {
    pymCfg->fiber[ deg_mf_indices[i] ].b.degenerated = true;
  }

  /* Sub-block 01 - A */
  int	AoffsetRow = 0, AoffsetCol = 0;
  FOR_0(i, nb) {
    FOR_0(j, A_trip[i]->nnz) {
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 0, 0,
        AoffsetRow + ((int    *)(A_trip[i]->i))[j],
        AoffsetCol + ((int    *)(A_trip[i]->j))[j],
        ((double *)(A_trip[i]->x))[j]);
    }
    AoffsetRow += A_trip[i]->nrow;
    AoffsetCol += A_trip[i]->ncol;
  }
  assert(AoffsetRow == Ari[1] && AoffsetCol == Aci[1]);
  int		EoffsetRow = 0, EoffsetCol = 0;

  /* Sub-block 02 - E := [0 -1 0] */
  FOR_0(i, nb) {
    const int	nfi	   = pymCfg->body[i].b.nFiber;
    //const int nai	   = pymCfg->body[i].b.nAnchor;
    FOR_0(j, nfi) {
      const int fiber_gbl_idx = pymCfg->body[i].b.fiber[j];
      assert(fiber_gbl_idx < pymCfg->nFiber);
      if (pymCfg->fiber[fiber_gbl_idx].b.degenerated)
        continue;
      FOR_0(k, nd) {
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 1, 0,
          EoffsetRow + nd*j + k,
          EoffsetCol + sd[i].Aci[9] + nd*j + k,
          -1);
      }
    }
    EoffsetRow += nd*pymCfg->body[i].b.nFiber;
    EoffsetCol += A_trip[i]->ncol;
  }
  assert(Ari[1]+EoffsetRow == Ari[2] && Aci[0]+EoffsetCol == Aci[1]);
  /* Sub-block 03 - R */
  int			 RoffsetRow = 0;
  FOR_0(i, nb) {
    cholmod_triplet	*R_i;
    GetR_i(&R_i, sd, i, pymCfg, cc);
    FOR_0(j, R_i->nnz) {
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 1, 1,
        RoffsetRow + ((int    *)(R_i->i))[j],
        0          + ((int    *)(R_i->j))[j],
        ((double *)(R_i->x))[j]);
    }
    RoffsetRow += nd*pymCfg->body[i].b.nFiber;
    cholmod_free_triplet(&R_i, cc);
  }
  assert(Ari[1]+RoffsetRow == Ari[2]);

  /* Sub-block 04, 05, 06 - K_11, K_12, K_13 */
  if (pymCfg->real_muscle) {
    FOR_0(i, nf) {
      double	k[3];
      GetMuscleFiberK(k, pymCfg->fiber + i, pymCfg);
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 2, 1, i, i, k[0]);
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 2, 2, i, i, k[1]);
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 2, 3, i, i, k[2]);
    }
  }
  /* Sub-block 07 - D */
  FOR_0(i, nj) {
    FOR_0(l, 4) {
      const pym_anchored_joint_t	*aJoint = pymCfg->anchoredJoints + i;
      assert(aJoint->aIdx >= 0 && aJoint->bIdx >= 0 && aJoint->aIdx != aJoint->bIdx);
      assert(aJoint->aAnchorIdx >= 0 && aJoint->bAnchorIdx >= 0);
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 3, 0,
        4*i + l,
        bod->Aici[ aJoint->aIdx ] + sd[ aJoint->aIdx ].Aci[10] + 4*aJoint->aAnchorIdx + l,
        1);
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 3, 0,
        4*i + l,
        bod->Aici[ aJoint->bIdx ] + sd[ aJoint->bIdx ].Aci[10] + 4*aJoint->bAnchorIdx + l,
        -1);
    }
  }
  /* Sub-block 08 - (-1) */
  FOR_0(i, 4*nj)
    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 3, 6, i, i, -1);
  /* Sub-block 09 - (0) Joint dislocation controled by \epsilon_d directly. */
  FOR_0(i, nj) {
    //SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 4, 7, i, i, 0);
  }
  /* Sub-block 10 - A_com */
  EoffsetCol			 = 0;
  double		totMass	 = 0;
  FOR_0(i, nb) {
    const double	mass	 = pymCfg->body[i].b.m;
    FOR_0(j, 3)
      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 5, 0, j, EoffsetCol + j, mass);
    totMass			+= mass;
    EoffsetCol			+= A_trip[i]->ncol;
  }
  /* Sub-block 11 - 1*M */
  FOR_0(i, 3)
    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 5, 8, i, i, -totMass);
  /* Sub-block 12 - 1 */
  FOR_0(i, 3)
    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 6, 8, i, i, 1);
  /* Sub-block 13 - 1 */
  FOR_0(i, 3)
    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, Ari, Aci, 6, 9, i, i, -1);


  ///* Sub-block 14 - G */
  //FOR_0(i, nb) {
  //  FOR_0(j, sd[i].nContacts_1) {
  //    double	r[3];
  //    FOR_0(l, 3) {
  //      /* We are optimizing to find the next state
  //      * based on the current state and data.
  //      * If we inevitably need for data depends on the next state,
  //      * approximation is used.
  //      * In this case, we actually need for the next COM position,
  //      * which is not availble for now. So we approximate
  //      * next COM by using current COM with assumption of
  //      * they are not so different.
  //      * ('curBipCom' should have valid value.)
  //      */
  //      r[l] = sd[i].contactsFix_1[j][l] - pymCfg->curBipCom[l];
  //    }
  //    double	rx[3][3];
  //    PymCrossToMat(rx, r);
  //    const int basecol	  = bod->Aici[i] + sd[i].Aci[1] + nd*j;
  //    /*
  //    *   [  0   x   x  ]
  //    *   [  x   0   x  ]
  //    *   [  x   x   0  ]
  //    */
  //    const int indx[][2] = { {0, 1}, {0, 2},
  //    {1, 0}, {1, 2},
  //    {2, 0}, {2, 1} };
  //    FOR_0(l, 6) {
  //      SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 7, 0,
  //        indx[l][0], basecol + indx[l][1],
  //        rx[ indx[l][0] ][ indx[l][1] ]);
  //    }
  //  }
  //}
  ///* Sub-block 15 - -1 */
  //FOR_0(i, 3)
  //  SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 7, 11, i, i, -1);


  /* Finish: convert AMatrix_trip to column compressed format */
  bod->bipMat		    = cholmod_triplet_to_sparse(AMatrix_trip, AMatrix_trip->nnz, cc);
  cholmod_free_triplet(&AMatrix_trip, cc);	/* not needed anymore */
  assert(bod->bipMat);
  /*********************
  * Eta vector setup  *
  *********************/
  double	*bipEta	    = (double *)calloc(Ari[Asubrows], sizeof(double));
  int		 etaOffset  = 0;
  /* Eta sub-block 00 - eta */
  FOR_0(i, nb) {
    double	*etai;
    GetEta(&etai, sd + i, pymCfg->body + i, pymCfg, cc);	/* Space for 'etai' is allocated at GetEta() */
    memcpy(bipEta + etaOffset, etai, sizeof(double)*A_trip[i]->nrow);
    free(etai);			/* so free here */
    etaOffset		   += A_trip[i]->nrow;
  }
  /* Eta sub-block 02 - s */
  if (pymCfg->real_muscle) {
    FOR_0(i, nf) {
      bipEta[ Ari[2] + i ] = GetMuscleFiberS(i, sd, pymCfg);
    }
  }
  /* Eta sub-block 04 - d_disloc */
  FOR_0(i, nj) {
    const char	*anchorName = pymCfg->pymJa[ pymCfg->anchoredJoints[i].aAnchorIdxGbl ].name;
    char	 iden[128];
    ExtractAnchorIdentifier(iden, anchorName);
    //printf("Name: %s (%s)\n", anchorName, iden);
  }
  /* Eta sub-block 06 - p_{com,ref} */
  double	refCom[3];
  PymCalculateRefCom(refCom, pymCfg);
  memcpy(bipEta + Ari[6], refCom, sizeof(double)*3);

  bod->bipEta = bipEta;

  FOR_0(i, nb) cholmod_free_triplet(&A_trip[i], cc);
}

void PymSetPymCfgChiRefToCurrentState(pym_config_t *pymCfg) {
  int	i;
  FOR_0(i, pymCfg->nBody) {
    memcpy(pymCfg->body[i].b.chi_ref  , pymCfg->body[i].b.p, sizeof(double)*3);
    memcpy(pymCfg->body[i].b.chi_ref+3, pymCfg->body[i].b.q, sizeof(double)*4);
  }
}
