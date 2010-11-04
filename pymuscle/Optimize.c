/*
 * Optimize.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */
#include "PymPch.h"
#include "PymStruct.h"
#include "Config.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "StateDependents.h"
#include "PymuscleConfig.h"
#include "DebugPrintDef.h"
#include "MathUtil.h"
#include "Optimize.h"
#include "PymDebugMessageFlags.h"

#define HORIZONTAL_LINE_NL						\
  "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"

static int DevStatCompare(const void * a, const void * b) {
  deviation_stat_entry *at = (deviation_stat_entry *)a;
  deviation_stat_entry *bt = (deviation_stat_entry *)b;
  double diff = bt->chi_d_norm - at->chi_d_norm;
  if (diff > 0) return 1;
  else if (diff < 0) return -1;
  else return 0;
}

static void MSKAPI printstr(void *handle,
                            char str[])
{
  FILE *out = (FILE *)handle;
  fprintf(out, "%s", str);
} /* printstr */

void PymInitializeMosek(MSKenv_t *env) {
  MSKrescodee  r;
  /* Create the mosek environment. */
  r = MSK_makeenv(env,NULL,NULL,NULL,NULL);
  /* Check if return code is ok. */
  assert ( r==MSK_RES_OK );
  /* Directs the log stream to the
     'printstr' function. */
  r = MSK_linkfunctoenvstream(*env,MSK_STREAM_LOG,NULL,printstr);
  assert ( r==MSK_RES_OK );
  /* Manually configure the CPU type */
  r = MSK_putcpudefaults(*env, MSK_CPU_INTEL_CORE2, 128*1024, 6144*1024);
  assert ( r==MSK_RES_OK );
  /* Initialize the environment. */
  assert ( r==MSK_RES_OK );
  r = MSK_initenv(*env);
}

void PymCleanupMosek(MSKenv_t *env) {
  /* Delete the environment and the associated data. */
  MSK_deleteenv(env);
}

void AppendConeRange(MSKtask_t task, int x, int r1, int r2) {
  /*
   * Append a second-order cone constraint which has
   * the form of x >= norm([ r1, r1+1, ... , r2-1 ])
   */
  assert(r1>=0 && r2>=0 && r2>=r1 && x>=0);
  int csub[1 + r2 - r1];
  csub[0] = x;
  int j;
  for (j=0; j<r2 - r1; ++j) csub[j+1] = r1 + j;
  MSKrescodee r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, 1+r2-r1, csub);
  assert(r == MSK_RES_OK);
}

static pym_opt_t PymNewOptimization(const pym_biped_eqconst_t *const bod,
				    const pym_rb_statedep_t *const sd,
				    const pym_config_t *const pymCfg,
				    MSKenv_t *pEnv, cholmod_common *cc)
{
  pym_opt_t o;
  memset(&o, 0, sizeof(pym_opt_t));
  /* Number of Ax=b style constraints */
  const int NUMCON = bod->bipMat->nrow;
  /* Number of optimization variables. */
  const int NUMVAR = bod->bipMat->ncol;
  const int nb = pymCfg->nBody;
  o.bkc     = malloc(sizeof(MSKboundkeye) * NUMCON);
  o.blc     = malloc(sizeof(double) * NUMCON);
  o.buc     = malloc(sizeof(double) * NUMCON);
  o.bkx     = malloc(sizeof(MSKboundkeye) * NUMVAR);
  o.blx     = malloc(sizeof(double) * NUMVAR);
  o.bux     = malloc(sizeof(double) * NUMVAR);
  o.c       = calloc(NUMVAR, sizeof(double));
  o.xx      = calloc(bod->bipMat->ncol, sizeof(double));
  o.nplist  = calloc(nb, sizeof(int));
  o._solsta = MSK_RES_OK;
  o.opttime = DBL_MAX;
  o.cost    = DBL_MAX;
  o.bod     = bod;
  o.sd      = sd;
  o.pymCfg  = pymCfg;
  o.pEnv    = pEnv;
  o.cc      = cc;
  return o;
}

void PymDelOptimization(pym_opt_t *o) {
  free(o->bkc); o->bkc = 0;
  free(o->blc); o->blc = 0;
  free(o->buc); o->buc = 0;
  free(o->bkx); o->bkx = 0;
  free(o->blx); o->blx = 0;
  free(o->bux); o->bux = 0;
  free(o->c); o->c = 0;
  free(o->xx); o->xx = 0;
  free(o->nplist); o->nplist = 0;
}

inline void SET_NONNEGATIVE(const pym_opt_t *const o, const int j) {
  o->bkx[j] = MSK_BK_LO;
  o->blx[j] = 0;
  o->bux[j] = MSK_INFINITY;
}

inline void SET_FIXED(const pym_opt_t *const o, const int j,
		      const double v) {
  o->bkx[j] = MSK_BK_FX;
  o->blx[j] = v;
  o->bux[j] = v;
}

inline void SET_FIXED_ONE(const pym_opt_t *const o, const int j) {
  SET_FIXED(o, j, 1.0);
}

inline void SET_FIXED_ZERO(const pym_opt_t *const o, const int j) {
  SET_FIXED(o, j, 0.0);
}

inline void SET_LOWER_BOUND(const pym_opt_t *const o, const int j,
			    const double lb) {
  o->bkx[j] = MSK_BK_LO;
  o->blx[j] = lb;
  o->bux[j] = MSK_INFINITY;
}

inline void SET_UPPER_BOUND(const pym_opt_t *const o, const int j,
			    const double ub) {
  o->bkx[j] = MSK_BK_UP;
  o->blx[j] = -MSK_INFINITY;
  o->bux[j] = ub;
}

inline void SET_RANGE(const pym_opt_t *const o, const int j,
		      const double lb, const double ub) {
  o->bkx[j] = MSK_BK_RA;
  o->blx[j] = lb;
  o->bux[j] = ub;
}

static void pym_optimize_init_upper_lower(pym_opt_t *pymOpt) {
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  /* Number of Ax=b style constraints */
  const int NUMCON = bod->bipMat->nrow;
  /* Number of optimization variables. */
  const int NUMVAR = bod->bipMat->ncol;
  int i;
  /* 'x_lower < A*x < x_upper' style constraints.
   * Default is 'fixed' equality constraint, i.e. a*x=b. */
  FOR_0(i, NUMCON) {
    pymOpt->bkc[i] = MSK_BK_FX; /* 'Fixed' for equality constraints */
    pymOpt->blc[i] = bod->bipEta[i];
    pymOpt->buc[i] = bod->bipEta[i];
  }

  /* 'x_lower < x < x_upper' style constraints.
   * (constraints related to optimization variables itself.)
   * Initialize all optimization variables as 'free' variables for now. */
  FOR_0(i, NUMVAR) {
    pymOpt->bkx[i] = MSK_BK_FR;
    pymOpt->blx[i] = -MSK_INFINITY;
    pymOpt->bux[i] = +MSK_INFINITY;
  }
}

static void pym_optimize_cost_function(pym_opt_t *pymOpt) {
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  int i, j;
  double *c = pymOpt->c;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const int nb = pymCfg->nBody;
  const int nf = pymCfg->nFiber;
  int tauOffset;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    FOR_0(j, pymOpt->nplist[i]) {
      /*
       * TODO [TUNE] Minimize the contact normal force
       * Walk0, Nav0  - 0
       * Exer0        - 10
       * Jump0        - ?
       * Jump1        - 2
       */
      c[ tauOffset + sd[i].Aci[2] + 5*j + 4 ] = 0;

      /* Estimated position of z-coordinate of contact point
       * Default: 2e-1, 1e-3      */
      c[ tauOffset + sd[i].Aci[3] + 4*j + 2 ] = 0;
    }
    for (j= tauOffset + sd[i].Aci[5];
	 j < tauOffset + sd[i].Aci[6]; ++j) {
      /*
       * TODO [TUNE] Minimize the movement of candidate contact points
       *
       * Walk0        -  1
       * Nav0         -  5e-1
       * Exer0        -  10
       * Jump0        -  ?
       * Jump1        -  ?
       */
      c[j] = 0;
    }
    /*
     * TODO [TUNE] Reference following coefficient
     */
    c[ tauOffset + sd[i].Aci[8] ] = 1;
    /*
     * TODO [TUNE] Previous close coefficient
     */
    c[ tauOffset + sd[i].Aci[13] ] = 0;
  }
  FOR_0(j, nf) {
    const char *const fibName = pymCfg->fiber[j].b.name;

    if (strncmp(fibName + strlen(fibName)-4, "Cen", 3) == 0) {
      /* No cost for axis-center ligaments on knees */
    } else {
      //c[ Aci[2] + j ] = 1e-6;
    }
  }
  /* Cost for COM deviation */
  c[ Aci[10] ] = 0;
  /* Cost for torque-around-COM */
  c[ Aci[12] ] = 0;

  assert(Aci[2] - Aci[1] == pymCfg->nFiber);
  assert(Aci[3] - Aci[2] == pymCfg->nFiber);
  /* minimize aggregate tension of actuated muscle fiber */
  c[ Aci[4] ] = 0; /* ligament actuation */

  /*
   * Since actuation forces on actuated muscle fibers are
   * non-negative values we can either minimize the second-order
   * cone constraint variable c[Aci[5]] OR
   * the optimization variables c[Aci[2]+j] separately where
   * 'j' is the index of an actuated muscle fiber.
   */
  c[ Aci[5] ] = 0; /* actuated muscle fiber actuation */

  FOR_0(i, pymCfg->nJoint) {
    /* Dislocation constraint */
    const int idx = bod->Aci[7] + i;
    c[ idx ] = 0;
  }
}

static void pym_optimize_contact_points(pym_opt_t *pymOpt) {
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  pymOpt->nptotal = 0;
  int i;
  FOR_0(i, nb) {
    pymOpt->nplist[i] = sd[i].nContacts_2;
    pymOpt->nptotal += sd[i].nContacts_2;
  }
  PRINT_VECTOR_INT(pymOpt->nplist, nb);
}

static void pym_optimize_rb_com(pym_opt_t *pymOpt) {
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    /* chi_2_z: Z-axis of COM for each body should be nonnegative */
    SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[0] + 2 );

    /* rotation parameterization constraint */
    /* Should not be used with Nav0 reference. */
    //SET_RANGE( pymOpt, tauOffset + sd[i].Aci[11], 0, 1.1*M_PI );

    /* eps_delta: eps_delta > |chi - chi_ref| */
    for(j=tauOffset + sd[i].Aci[8]; j<tauOffset + sd[i].Aci[9]; ++j) {
      SET_NONNEGATIVE( pymOpt, j );
    }

    if (strcmp(pymOpt->pymCfg->body[i].b.name, "soleL") == 0
	||strcmp(pymOpt->pymCfg->body[i].b.name, "soleR") == 0
	//	||strcmp(pymOpt->pymCfg->body[i].b.name, "toeL") == 0
	//||strcmp(pymOpt->pymCfg->body[i].b.name, "toeR") == 0
	) {
      // SET_UPPER_BOUND(pymOpt, tauOffset + sd[i].Aci[8], 0.2);
    }
  }
}

static void pym_optimize_biped_com(pym_opt_t *pymOpt) {
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  /* COM norm deviation constraint variable */
  if (pymOpt->nptotal) {
    //SET_RANGE( pymOpt, bod->Aci[10], 0, 0.05 );
  }
}

static void pym_optimize_contact_force(pym_opt_t *pymOpt) {
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const int nd = 6;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    FOR_0(j, pymOpt->nplist[i]) {
      /* f_c_z: Contact normal force should be nonnegative */
      SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[1] + nd*j + 2 );
      //SET_RANGE(pymOpt, tauOffset + sd[i].Aci[i]+nd*j+2, 0, 200);
      /* c_c : Contact force basis (5-dimension)
	 c_c_x: tangential x
	 c_c_y: tangential y
	 c_c_z: tangential z (should be 0)
	 c_c_w: homogeneous component (should be 0)
	 c_c_n: normal z (should be nonnegative) */
      /* c_c_z: Contact tangential force's Z component should be zero. */
      SET_FIXED_ZERO ( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 2 );
      /* c_c_w: Contact tangential force's homogeneous part
	 should be fixed to 0 since force is vector quantity. */
      SET_FIXED_ZERO ( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 3 );
      /*
       * c_c_n
       * TODO [TUNE] Contact normal force constraint tuning
       * Walk0   :  Nonnegative
       * Nav0    :  0~200
       * Exer0   :  0~200 (failed)
       */
      SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 4 );
    }
  }
}

static void pym_optimize_contact_point(pym_opt_t *pymOpt) {
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    /* p_c_2_z: next step CP z-pos */
    FOR_0(j, pymOpt->nplist[i]) {
      /* TODO: Inclined ground */
      const double ctY = sd[i].contactsFix_2[j][1];
      const double theta = pymCfg->slant;
      const double z = -ctY*tan(theta);
      assert(z == 0); /* Only flat ground implemented. */
      SET_LOWER_BOUND( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2, z );
    }
    /*
     * TODO [TUNE] Constraints for fixing contact points
     * p_c_2_z
     */
    int nfixpoint = 0;
    FOR_0(j, pymOpt->nplist[i]) {
      const char *rbname = pymCfg->body[i].b.name;
      //printf("nplistname = %s\n", pymCfg->body[i].b.name);

      /* Move along the XY-plane strictly probihited.(USELESS!) */
      //SET_UPPER_BOUND ( pymOpt, tauOffset + sd[i].Aci[4] + 4*j + 0, 0.0 );
      //SET_UPPER_BOUND ( pymOpt, tauOffset + sd[i].Aci[4] + 4*j + 1, 0.0 );
      //SET_UPPER_BOUND(pymOpt, tauOffset + sd[i].Aci[5] + j, 0.25);
      const int cidx = sd[i].contactIndices_2[j];
      assert(0 <= j && j <= 7);
      assert(0 <= cidx && cidx <= 7);
      const int isFoot =
	strcmp(rbname, "soleL") == 0 ||	strcmp(rbname, "soleR") == 0 ||
	strcmp(rbname, "toeL") == 0 || strcmp(rbname, "toeR") == 0;
      if (nfixpoint < 3 && cidx%2 == 0 && isFoot ) { /* if sole side */
	/* Next time step Z-position of contact points should remain 0 */
	//SET_FIXED_ZERO ( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2 );
	/* SET_RANGE ( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2, */
	/* 	    0, 0.05); */
	++nfixpoint;
      }
    }
    /* p_c_2_w */
    FOR_0(j, pymOpt->nplist[i]) {
      SET_FIXED_ONE  ( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 3 );
    }
  }
}

static void pym_optimize_anchored_joint(pym_opt_t *pymOpt) {
  int i, j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  FOR_0(i, pymCfg->nJoint) {
    /* Fix homogeneous components anchored joint
       dislocation vectors (d_A) to 0 */
    const int idx = bod->Aci[6] + 4*i + 3;
    SET_FIXED_ZERO( pymOpt, idx );
  }
}

static void pym_optimize_muscle(pym_opt_t *pymOpt) {
  int i, j;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  const int nf = pymCfg->nFiber;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  FOR_0(j, nf) {
    const pym_muscle_type_e mt = pymCfg->fiber[j].b.mType;
    const char *fibName = pymCfg->fiber[j].b.name;
    /* Tension range constraint */
    i = Aci[1]+j;
    //        bkx[i] = MSK_BK_RA;
    //        blx[i] = -1000;
    //        bux[i] = +1000;
    /* Actuation force range constraint */
    i = Aci[2]+j;
    if (mt == PMT_ACTUATED_MUSCLE) {
      //SET_RANGE( pymOpt, i, -200*9.81, 200*9.81 );
    }
    else if (mt == PMT_LIGAMENT) {
      if (strncmp(fibName + strlen(fibName)-4, "Cen", 3) == 0) {
	//SET_RANGE(i, -1000, 1000);
      } else {
	/*
	 *  Muscle fiber nonnegative
	 */
	//  SET_NONNEGATIVE(i);
	//SET_RANGE(i, -1000, 1000);
      }
    }
    else {
      abort();
    }
    /* Walk0 (loose) : -47~47 */
    /* Walk0 (correct) : -100~100 */
    /* Jump0 : -98~98, -140~90 */
    //SET_RANGE(pymOpt, i, -120, 120);
    /* Rest length range constraint */
    i = Aci[3]+j;
    SET_RANGE( pymOpt, i, pymCfg->fiber[j].b.xrest_lower,
    	       pymCfg->fiber[j].b.xrest_upper );
    //SET_RANGE( pymOpt, i, 0.5, 0.6);
  }
}

static void pym_optimize_upper_lower(pym_opt_t *pymOpt) {
  /* x_l < x < x_u style constraints */
  pym_optimize_contact_points(pymOpt);
  pym_optimize_rb_com(pymOpt);
  pym_optimize_biped_com(pymOpt);
  pym_optimize_contact_force(pymOpt);
  pym_optimize_contact_point(pymOpt);
  pym_optimize_anchored_joint(pymOpt);
  pym_optimize_muscle(pymOpt);
}

static MSKtask_t pym_optimize_init_mosek_task(MSKenv_t *pEnv,
					      int NUMVAR, int NUMCON,
					      int NUMANZ) {
  MSKtask_t   task;
  MSKrescodee r;
  /* Create the optimization task. */
  r = MSK_maketask(*pEnv,NUMCON,NUMVAR,&task);
  assert(r == MSK_RES_OK);

  /* On my(gykim) computer, the other values except 1 for the
   * number of threads does not work. (segfault or hang)
   * Other PCs seemed to fine, though.
   */
  //r = MSK_putintparam (task, MSK_IPAR_INTPNT_NUM_THREADS, 1);
  //assert(r == MSK_RES_OK);

  //MSK_putintparam (task , MSK_IPAR_OPTIMIZER , MSK_OPTIMIZER_FREE_SIMPLEX);
  //MSK_putintparam (task , MSK_IPAR_OPTIMIZER , MSK_OPTIMIZER_NONCONVEX);

  //r = MSK_putintparam (task , MSK_IPAR_CPU_TYPE , MSK_CPU_INTEL_CORE2);
  //assert(r == MSK_RES_OK);
  //    r = MSK_putintparam (task , MSK_IPAR_CACHE_SIZE_L1, 128*1024);
  //    assert(r == MSK_RES_OK);
  //    r = MSK_putintparam (task , MSK_IPAR_CACHE_SIZE_L2, 6144*1024);
  //    assert(r == MSK_RES_OK);
  r = MSK_putintparam (task , MSK_IPAR_DATA_CHECK, MSK_OFF);
  assert(r == MSK_RES_OK);
  //    r = MSK_putintparam (task , MSK_IPAR_INTPNT_MAX_ITERATIONS, 10);
  //    assert(r == MSK_RES_OK);


  /* Give MOSEK an estimate of the size of the input data.
     This is done to increase the speed of inputting data.
     However, it is optional. */
  r = MSK_putmaxnumvar(task,NUMVAR);  assert(r == MSK_RES_OK);
  r = MSK_putmaxnumcon(task,NUMCON);  assert(r == MSK_RES_OK);
  r = MSK_putmaxnumanz(task,NUMANZ);  assert(r == MSK_RES_OK);

  /* Append 'NUMCON' empty constraints.
     The constraints will initially have no bounds. */
  r = MSK_append(task,MSK_ACC_CON,NUMCON);  assert(r == MSK_RES_OK);

  /* Append 'NUMVAR' variables.
     The variables will initially be fixed at zero (x=0). */
  r = MSK_append(task,MSK_ACC_VAR,NUMVAR);   assert(r == MSK_RES_OK);
  return task;
}

static void pym_optimize_mosek_cost_function(const pym_opt_t *const pymOpt,
					     MSKtask_t task) {
  int j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  /* Number of optimization variables. */
  const int NUMVAR = bod->bipMat->ncol;
  MSKrescodee r = MSK_RES_OK;
  for(j=0; j<NUMVAR && r == MSK_RES_OK; ++j) {
    /* Set the linear term c_j in the objective.*/
    r = MSK_putcj(task,j,pymOpt->c[j]);
    assert(r == MSK_RES_OK);
  }
  /* Optionally add a constant term to the objective. */
  r = MSK_putcfix(task,0.0);
  assert(r == MSK_RES_OK);
}

static void pym_optimize_mosek_upper_lower(const pym_opt_t *const pymOpt,
					   MSKtask_t task) {
  int i, j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  /* Number of optimization variables */
  const int NUMVAR = bod->bipMat->ncol;
  /* Number of Ax=b type constraints */
  const int NUMCON = bod->bipMat->nrow;
  MSKrescodee r = MSK_RES_OK;
  int __annz = 0;
  for(j=0; j<NUMVAR && r == MSK_RES_OK; ++j)
    {
      /* Set the bounds on variable j.
	 blx[j] <= x_j <= bux[j] */
      r = MSK_putbound(task,
                       MSK_ACC_VAR, /* Put bounds on variables.*/
                       j,           /* Index of variable.*/
                       pymOpt->bkx[j], /* Bound key.*/
                       pymOpt->blx[j], /* Numerical value of lower bound.*/
                       pymOpt->bux[j]);/* Numerical value of upper bound.*/
      assert(r == MSK_RES_OK);

      /* Input column j of A */
      if(r == MSK_RES_OK) {
	int colstart = ((int *)(bod->bipMat->p))[j  ];
	int colend   = ((int *)(bod->bipMat->p))[j+1];
	__annz += colend-colstart;
	/* Pointer to row indexes of column j. */
	int *rowIdxOfColJ = (int *)(bod->bipMat->i) + colstart;
	/* Pointer to Values of column j. */
	double *valOfColJ = (double *)(bod->bipMat->x) + colstart;
	r = MSK_putavec(task,
			MSK_ACC_VAR,     /* Input columns of A.*/
			j,               /* Variable (column) index.*/
			colend-colstart, /* Number of non-zeros in column j.*/
			rowIdxOfColJ,
			valOfColJ);
      }
    }
  assert(__annz == cholmod_nnz(bod->bipMat, pymOpt->cc));
  //printf("__annz = %d\n", __annz);

  /* Set the bounds on constraints.
     for i=1, ...,NUMCON : blc[i] <= constraint i <= buc[i] */
  for(i=0; i<NUMCON && r==MSK_RES_OK; ++i) {
    r = MSK_putbound(task,
		     MSK_ACC_CON, /* Put bounds on constraints.*/
		     i,           /* Index of constraint.*/
		     pymOpt->bkc[i],      /* Bound key.*/
		     pymOpt->blc[i],      /* Numerical value of lower bound.*/
		     pymOpt->buc[i]);     /* Numerical value of upper bound.*/
    assert(r == MSK_RES_OK );
  }
}

static void pym_optimize_mosek_cone_rb(const pym_opt_t *const pymOpt,
				       MSKtask_t task) {
  int i, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    /* Reference trajectory cone constraints, i.e.,
     * epsilon_Delta >= || Delta_chi_{i,ref} || (6-DOF)      */
    AppendConeRange(task,
		    tauOffset + sd[i].Aci[8],
		    tauOffset + sd[i].Aci[7],
		    tauOffset + sd[i].Aci[8]);
    /* Previous state close cone constraints, i.e.,
     * epsilon_Delta >= || Delta_chi_{i,prv} || (6-DOF)      */
    AppendConeRange(task,
		    tauOffset + sd[i].Aci[13],
		    tauOffset + sd[i].Aci[12],
		    tauOffset + sd[i].Aci[13]);
  }
  /* Rotation parameterization constraints */
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    AppendConeRange(task,
		    tauOffset + sd[i].Aci[11],        // epsilon_rot
		    tauOffset + sd[i].Aci[0] + 3,     // chi[3..6]
		    tauOffset + sd[i].Aci[0] + 6);
  }
}

static void pym_optimize_mosek_cone_contact_point
(const pym_opt_t *const pymOpt,
 MSKtask_t task) {
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    FOR_0(j, pymOpt->nplist[i]) {
      /*
       * TODO [TUNE] CP movement minimization
       *
       * Walk0, Nav0 - including z-axis movement (0~3)
       * Exer0       - excluding z-axis movement (0~2)
       */
      AppendConeRange(task,
		      tauOffset + sd[i].Aci[5] + j,
		      tauOffset + sd[i].Aci[4]+4*j+0,
		      tauOffset + sd[i].Aci[4]+4*j+2);
    }
  }
}

static void pym_optimize_mosek_cone_contact_force
(const pym_opt_t *const pymOpt,
 MSKtask_t task) {
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
       tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
    FOR_0(j, pymOpt->nplist[i]) {
      /* Friction cone constraints */
      AppendConeRange(task,
		      tauOffset + sd[i].Aci[6]+j,            // mu*c_n
		      tauOffset + sd[i].Aci[2]+5*j+0,        // c_tx ~ c_tz
		      tauOffset + sd[i].Aci[2]+5*j+3);
    }
  }
}

static void pym_optimize_mosek_cone_anchored_joint
(const pym_opt_t *const pymOpt,
 MSKtask_t task) {
  int j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  FOR_0(j, pymCfg->nJoint) {
    /* Anchored joint dislocation constraints */
    AppendConeRange(task,
		    Aci[7] + j,                      // epsilon_d
		    Aci[6] + 4*j,                    // dAx ~ dAz
		    Aci[6] + 4*j + 3);
  }
}

static void pym_optimize_mosek_cone_muscle
(const pym_opt_t *const pymOpt,
 MSKtask_t task) {
  int j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  MSKrescodee r = MSK_RES_OK;
  /*
   * TODO [TUNE] Cone constraints for minimizing tension/actuation forces
   */
  const int nf = pymCfg->nFiber;
  int csubLigaAct[1+nf];
  int nCsubLigaAct = 1;
  csubLigaAct[0] = Aci[4];
  int csubActAct[1+nf];
  int nCsubActAct = 1;
  csubActAct[0] = Aci[5];
  FOR_0(j, nf) {
    const pym_muscle_type_e mt = pymCfg->fiber[j].b.mType;
    if (mt == PMT_ACTUATED_MUSCLE) {
      csubActAct[nCsubActAct] = Aci[1] + j;
      ++nCsubActAct;
    }
    else if (mt == PMT_LIGAMENT) {
      csubLigaAct[nCsubLigaAct] = Aci[1] + j;
      ++nCsubLigaAct;
    }
    else
      abort();
  }
  /*** TODO ***/
  r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, nCsubLigaAct, csubLigaAct);
  assert(r == MSK_RES_OK);
  r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, nCsubActAct, csubActAct);
  assert(r == MSK_RES_OK);
}

static void pym_optimize_mosek_cone_biped(const pym_opt_t *const pymOpt,
					  MSKtask_t task) {
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  /* COM constraint */
  AppendConeRange(task,
		  Aci[10],                      // epsilon_com
		  Aci[9],                     // delta p_{com,ref} (z-axis only)
		  Aci[10]);
  /* Torque-about-COM constraint cone */
  AppendConeRange(task,
		  Aci[12], /* epsilon_{tau,com} */
		  Aci[11], /* Torque-about-COM (tau_com) */
		  Aci[12]);
}

static void pym_optimize_mosek_cone(const pym_opt_t *const pymOpt,
				    MSKtask_t task) {
  pym_optimize_mosek_cone_rb(pymOpt, task);
  pym_optimize_mosek_cone_contact_point(pymOpt, task);
  pym_optimize_mosek_cone_contact_force(pymOpt, task);
  pym_optimize_mosek_cone_anchored_joint(pymOpt, task);
  pym_optimize_mosek_cone_biped(pymOpt, task);
  pym_optimize_mosek_cone_muscle(pymOpt, task);
}

static MSKrescodee pym_optimize_mosek_optimize(MSKtask_t task) {
  MSKrescodee trmcode;
  MSKrescodee r;
  r = MSK_optimizetrm(task, &trmcode);
  if (r == MSK_RES_TRM_MAX_ITERATIONS) {
    printf("Error - MSK_RES_TRM_MAX_ITERATIONS returned. Abort.\n");
  } else if (r == MSK_RES_TRM_STALL) {
    printf("Error - MSK_RES_TRM_STALL returned. Abort.\n");
  }
  return r;
}

static void pym_optimize_mosek_analyze_result(pym_opt_t *pymOpt,
					      MSKtask_t task,
					      MSKrescodee taskr) {
  double cost = DBL_MAX;
  const int NUMVAR = pymOpt->bod->bipMat->ncol;
  MSK_getdouinf ( task , MSK_DINF_OPTIMIZER_TIME , &pymOpt->opttime );
  /* Print a summary containing information
     about the solution for debugging purposes*/
  MSK_solutionsummary (task, MSK_STREAM_LOG);
  static const char *unknownMsg =
    "   ***   The status of the solution could not be determined.   ***\n";
  unknownMsg = unknownMsg;
  if ( taskr==MSK_RES_OK ) {
    MSK_getsolutionstatus (task,
			   MSK_SOL_ITR,
			   NULL,
			   &pymOpt->_solsta);
    switch (pymOpt->_solsta) {
    case MSK_SOL_STA_UNKNOWN:
      //printf(unknownMsg);
    case MSK_SOL_STA_OPTIMAL:
    case MSK_SOL_STA_NEAR_OPTIMAL:
      MSK_getsolutionslice(task,
			   MSK_SOL_ITR, /* Request the interior solution. */
			   MSK_SOL_ITEM_XX,/* Which part of solution.     */
			   0,              /* Index of first variable.    */
			   NUMVAR,         /* Index of last variable+1.   */
			   pymOpt->xx);
      /* TODO: Better way to calculate(or query) cost from MOSEK */
      //cost = Dot(NUMVAR, xx, pymOpt->c);
      MSK_getprimalobj(task, MSK_SOL_ITR, &cost);
      break;
    case MSK_SOL_STA_DUAL_INFEAS_CER:
    case MSK_SOL_STA_PRIM_INFEAS_CER:
    case MSK_SOL_STA_NEAR_DUAL_INFEAS_CER:
    case MSK_SOL_STA_NEAR_PRIM_INFEAS_CER:
      printf("Primal or dual infeasibility certificate found.\n");
      cost = DBL_MAX;
      break;

    default:
      printf("Other solution status.");
      cost = DBL_MAX;
      break;
    }
  } else {
    printf("Error - An error occurred while optimizing.\n");
    printf("        Refer to /tmp/pymoptimize_log and pymuscle_c.opf\n");
    printf("        for further investigation.\n");
    MSK_writedata(task, "pymuscle_c.opf");

    /* In case of an error print error code and description. */
    char symname[MSK_MAX_STR_LEN];
    char desc[MSK_MAX_STR_LEN];
    MSK_getcodedesc (taskr, symname, desc);
    printf("Error - %s: %s\n", symname, desc);
    cost = DBL_MAX;
  }
  pymOpt->cost = cost;
}

void PymOptimize(pym_opt_t *pymOpt) {
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  MSKenv_t *pEnv = pymOpt->pEnv;
  cholmod_common *cc = pymOpt->cc;

  /* Number of constraints related to linear function
     of optimization variables. */
  assert(pymCfg->nJoint);
  /* Number of Ax=b style constraints */
  const int NUMCON = bod->bipMat->nrow;
  /* Number of optimization variables. */
  const int NUMVAR = bod->bipMat->ncol;
  /* Number of non-zeros in A. */
  const int NUMANZ = cholmod_nnz(bod->bipMat, cc);
  MSKrescodee  r;
  
  /* Configure optimization data (without MOSEK 'task' involved.) */
  pym_optimize_init_upper_lower(pymOpt);
  pym_optimize_contact_points(pymOpt);
  pym_optimize_cost_function(pymOpt);
  pym_optimize_upper_lower(pymOpt);
  
  MSKtask_t task = pym_optimize_init_mosek_task(pEnv, NUMVAR, NUMCON, NUMANZ);
  if (!task) {
    printf("Error - MOSEK task init failed.\n");
    pymOpt->cost = DBL_MAX;
    return;
  }
  /* If some error or infeasibility condition detected from
   * MOSEK, the user can inspect this log file.     */
  FILE *mosekLogFile = 0;
  /* TODO MOSEK optimization logging */
  mosekLogFile = fopen("/tmp/pymoptimize_log", "w");
  if (!mosekLogFile) {
    printf("Error - Opening MOSEK output log file failed.\n");
    pymOpt->cost = DBL_MAX;
    return;
  }
  r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, mosekLogFile, printstr);
  assert(r == MSK_RES_OK);

  pym_optimize_mosek_cost_function(pymOpt, task);
  pym_optimize_mosek_upper_lower(pymOpt, task);
  pym_optimize_mosek_cone(pymOpt, task);
  /* Write the whole optimization setup on OPF file for debugging */
  MSK_writedata(task, "a.opf");
  r = pym_optimize_mosek_optimize(task);
  pym_optimize_mosek_analyze_result(pymOpt, task, r);

  if (mosekLogFile)
    fclose(mosekLogFile);
  /* Delete the task and the associated data. */
  MSK_deletetask(&task);
}

void PymConstructSupportPolygon(pym_config_t *pymCfg,
				pym_rb_statedep_t *sd) {
  int chOutputLen = 0;
  int i, j;
  pymCfg->chInputLen = 0;
  const int nb = pymCfg->nBody;
  FOR_0(i, nb) {
    pym_rb_statedep_t *sdi = sd + i;
    FOR_0(j, sdi->nContacts_2) {
      pymCfg->chInput[ pymCfg->chInputLen ].x = sdi->contactsFix_2[j][0];
      pymCfg->chInput[ pymCfg->chInputLen ].y = sdi->contactsFix_2[j][1];
      ++pymCfg->chInputLen;
    }
  }
  assert(pymCfg->chInputLen < 100);
  if (pymCfg->chInputLen > 0) {
    pymCfg->chOutputLen = PymConvexHull(pymCfg->chInput,
					pymCfg->chInputLen, pymCfg->chOutput);
  } else {
    pymCfg->chOutputLen = 0;
  }
  assert(pymCfg->chInputLen >= chOutputLen);
}

static void pym_analyze_rb_deviation_stat(const pym_opt_t *const pymOpt,
					  FILE *outputFile,
					  FILE *dmstreams[]) {
  const double *const xx = pymOpt->xx;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  deviation_stat_entry dev_stat[nb];
  memset(dev_stat, 0, sizeof(deviation_stat_entry)*nb);
  int j, tauOffset;
  for (j = 0, tauOffset = 0;
       j < nb;
       tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++) {
    const double *chi_2 = xx + tauOffset;
    const pym_rb_named_t *rbn = &pymCfg->body[j].b;

    double chi_1[6], chi_0[6], chi_r[6], chi_v[6];
    memcpy(chi_1    , rbn->p,       sizeof(double)*3);
    memcpy(chi_1 + 3, rbn->q,       sizeof(double)*3);
    memcpy(chi_0    , rbn->p0,      sizeof(double)*3);
    memcpy(chi_0 + 3, rbn->q0,      sizeof(double)*3);
    memcpy(chi_r    , rbn->chi_ref, sizeof(double)*6);
    memcpy(chi_v    , rbn->pd,      sizeof(double)*3);
    memcpy(chi_v + 3, rbn->qd,      sizeof(double)*3);

    double chi_d[6];
    int k;
    FOR_0(k, 6) {
      chi_d[k] = chi_2[k] - chi_r[k];
      dev_stat[j].chi_d_norm += chi_d[k] * chi_d[k];
    }
    dev_stat[j].chi_d_norm = sqrt(dev_stat[j].chi_d_norm);
    dev_stat[j].bodyIdx = j;
    dev_stat[j].nContact = sd[j].nContacts_2;

    if (outputFile) {
      FOR_0(k, 6) fprintf(outputFile, "%18.8e", chi_2[k]);
      fprintf(outputFile, "\n");
    }
  }
  qsort(dev_stat, nb, sizeof(deviation_stat_entry), DevStatCompare);

  FILE *dmst = dmstreams[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT];
  fprintf(dmst, "Reference trajectory deviation report\n");
  const int itemsPerLine = PymMin(nb, 6);
  int j0 = 0, j1 = itemsPerLine;
  while (j0 < nb && j1 <= nb) {
    for (j = j0; j < j1; ++j) {
      const pym_rb_named_t *rbn = &pymCfg->body[ dev_stat[j].bodyIdx ].b;
      fprintf(dmst,
	      "  %9s", rbn->name);
    }
    fprintf(dmstreams[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT],
	    "\n");
    for (j = j0; j < j1; ++j) {
      fprintf(dmst, "  %9.3e", dev_stat[j].chi_d_norm);
    }
    fprintf(dmst,
	    "\n");
    for (j = j0; j < j1; ++j) {
      fprintf(dmst, "  %9d", dev_stat[j].nContact);
    }
    fprintf(dmst, "\n");
    j0 = PymMin(nb, j0 + itemsPerLine);
    j1 = PymMin(nb, j1 + itemsPerLine);
  }
}

static void pym_update_rb_and_biped_com
(pym_config_t *pymCfg,
 const pym_opt_t *const pymOpt,
 const pym_biped_eqconst_t *const bipEq) {
  const double *const xx = pymOpt->xx;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  int j, tauOffset;
  for (j = 0, tauOffset = 0;
       j < nb;
       tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++) {
    const double *chi_2 = xx + tauOffset;
    /* Update the current state of rigid bodies */
    SetRigidBodyChi_1(pymCfg->body + j, chi_2, pymCfg);
  }
  /* Update COM of biped. This is part of opt. var. */
  memcpy(pymCfg->bipCom, xx + bipEq->Aci[8], sizeof(double)*3);
}

static void pym_update_muscle
(pym_config_t *pymCfg,
 const pym_opt_t *const pymOpt,
 const pym_biped_eqconst_t *const bipEq) {
  const int nf = pymCfg->nFiber;
  const double *const xx = pymOpt->xx;
  int j;
  FOR_0(j, nf) {
    pym_mf_named_t *mfn = &pymCfg->fiber[j].b;
    const double T_0        = xx[ bipEq->Aci[1] + j ];
    const double u_0        = xx[ bipEq->Aci[2] + j ];
    const double xrest_0    = xx[ bipEq->Aci[3] + j ];
    /* Update the current state of muscle fibers */
    mfn->T     = T_0;
    mfn->A     = u_0;
    mfn->xrest = xrest_0;
    //            printf("%16s -   T = %15.8e     u = %15.8e     xrest = %15.8e\n", mfn->name, T_0, u_0, xrest_0);
  }
}

static void pym_analyze_anchored_joint_stat
(pym_config_t *pymCfg,
 const pym_opt_t *const pymOpt,
 const pym_biped_eqconst_t *const bipEq,
 FILE *dmstreams[]) {
  FILE *dmst = dmstreams[PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT];
  fprintf(dmst, "Anchored joints dislocation report\n");
  const double *const xx = pymOpt->xx;
  int j;
  const int nj = pymCfg->nJoint;
  FOR_0(j, nj) {
    const double *dAj    = xx + bipEq->Aci[6] + 4*j;
    assert(dAj[3] == 0); /* homogeneous component */
    const double disloc  = PymNorm(4, dAj);
    const int ajBodyAIdx = pymCfg->anchoredJoints[j].aIdx;
    const pym_rb_named_t *ajBodyA
      = &pymCfg->body[ ajBodyAIdx ].b;
    const int ancIdx     = pymCfg->anchoredJoints[j].aAnchorIdx;
    const char *aAncName = ajBodyA->jointAnchorNames[ ancIdx ];
    char iden[128];
    ExtractAnchorIdentifier(iden, aAncName);
    fprintf(dmst,
	    "%12s disloc = %e", iden, disloc);
    if (j%2) fprintf(dmst, "\n");

    if (pymCfg->anchoredJoints[j].maxDisloc < disloc)
      pymCfg->anchoredJoints[j].maxDisloc = disloc;
  }
  if (nj%2) fprintf(dmst, "\n");
}

static void pym_analyze_biped_com_deviation_stat
(const pym_opt_t *const pymOpt,
 const pym_biped_eqconst_t *const bipEq,
 FILE *dmstreams[]) {
  const double *const xx = pymOpt->xx;
  FILE *dmst = dmstreams[PDMTE_FBYF_REF_COM_DEVIATION_REPORT];
  const double *const comDev = xx + bipEq->Aci[9];
  const double comDevLenSq = PymNormSq(3, comDev);
  fprintf(dmst, "COM deviation: dev=[%lf,%lf,%lf], |dev|^2=%lf\n",
	  comDev[0], comDev[1], comDev[2], comDevLenSq);
}

static void pym_count_total_num_cp(pym_config_t *pymCfg,
				   pym_rb_statedep_t *sd) {
  pymCfg->prevTotContacts = pymCfg->curTotContacts;
  pymCfg->curTotContacts  = pymCfg->nextTotContacts;
  pymCfg->nextTotContacts = 0;
  int j;
  FOR_0(j, pymCfg->nBody) {
    pymCfg->nextTotContacts += sd[j].nContacts_2;
  }
  if (pymCfg->curTotContacts > 0 && pymCfg->nextTotContacts == 0) {
    printf("Flight phase started!\n");
  } else if (pymCfg->curTotContacts == 0 && pymCfg->nextTotContacts > 0) {
    printf("Flight phase ended!\n");
  }
  /* printf("prevTotContacts = %d\n", pymCfg->prevTotContacts); */
  /* printf("curTotContacts  = %d\n", pymCfg->curTotContacts); */
  /* printf("nextTotContacts = %d\n", pymCfg->nextTotContacts); */
}

static void pym_pass_opt_result(double *pureOptTime, const char **_solstaStr,
				double *_cost, const pym_opt_t *const pymOpt) {
  if (pureOptTime) {
    *pureOptTime = pymOpt->opttime;
  }
  if (pymOpt->cost == DBL_MAX) {
    printf("Something goes wrong while optimizing.\n");
  }
  const char *solstaStr = 0;
  switch (pymOpt->_solsta) {
  case MSK_SOL_STA_UNKNOWN:      solstaStr = "UNKNOWN"; break;
  case MSK_SOL_STA_OPTIMAL:      solstaStr = "optimal"; break;
  case MSK_SOL_STA_NEAR_OPTIMAL: solstaStr = "near optimal"; break;
  default:                       solstaStr = "ERROR!"; break;
  }
  *_solstaStr = solstaStr;
  *_cost      = pymOpt->cost;
}

static void pym_copy_cp_and_cf(pym_config_t *pymCfg,
			       pym_rb_statedep_t *sd,
			       const double *const xx) {
  const int nb = pymCfg->nBody;
  int j, k, tauOffset;
  for (j = 0, tauOffset = 0; j < nb;
       tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++) {
    const double *const chi_2 = xx + tauOffset;
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    const pym_rb_statedep_t *sdj = sd + j;
    rbn->nContacts_2 = sdj->nContacts_2;
    FOR_0(k, sdj->nContacts_2) {
      memcpy(rbn->contactsPoints_2a[k], chi_2 + sdj->Aci[3] + 4*k,
	     sizeof(double)*4);
      memcpy(rbn->contactsForce_2[k],   chi_2 + sdj->Aci[1] + 6*k,
	     sizeof(double)*3);
    }
  }
}

static void pym_analyze_cp_slip(pym_config_t *pymCfg,
				pym_rb_statedep_t *sd,
				const double *const xx) {
  const int nb = pymCfg->nBody;
  int j, k, tauOffset;
  for (j = 0, tauOffset = 0; j < nb;
       tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++) {
    //const double *chi_2 = xx + tauOffset;
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    const pym_rb_statedep_t *sdj = sd + j;

    rbn->nContacts_2 = sdj->nContacts_2;
    FOR_0(k, sdj->nContacts_2) {
      //const double epsil = *(chi_2 + sdj->Aci[5] + k);
      //const double zpos  = *(chi_2 + sdj->Aci[3] + 4*k + 2);
      /* printf("%s [%d <%d>] - CP dev %lf / Z-pos %lf\n", */
      /* 	     rbn->name, k, sdj->contactIndices_2[k], epsil, zpos); */
    }
  }
}

static void pym_analyze_and_update(pym_config_t *pymCfg,
				   pym_opt_t *pymOpt,
				   FILE *outputFile,
				   FILE *dmstreams[],
				   pym_biped_eqconst_t *bipEq,
				   pym_rb_statedep_t *sd,
				   const double *const xx) {
  pym_analyze_rb_deviation_stat(pymOpt, outputFile, dmstreams);
  pym_update_rb_and_biped_com(pymCfg, pymOpt, bipEq);
  pym_update_muscle(pymCfg, pymOpt, bipEq);
  pym_analyze_anchored_joint_stat(pymCfg, pymOpt, bipEq, dmstreams);
  pym_analyze_biped_com_deviation_stat(pymOpt, bipEq, dmstreams);
  pym_copy_cp_and_cf(pymCfg, sd, xx);
  pym_analyze_cp_slip(pymCfg, sd, xx);
}

static void pym_print_opt_fail_log() {
  printf("Optimization failure report\n");
  printf(HORIZONTAL_LINE_NL);
  int ret = 0;
  //ret = system("cat /tmp/pymoptimize_log");
  printf(HORIZONTAL_LINE_NL);
  if (ret)
    printf("Warning - /tmp/pyoptimize_log opening failure.\n");
}

int PymOptimizeFrameMove(double *pureOptTime, FILE *outputFile,
                         pym_config_t *pymCfg, pym_rb_statedep_t *sd,
                         FILE *dmstreams[],
                         const char **_solstaStr, double *_cost,
                         cholmod_common *cc, MSKenv_t env) {
  const int nb = pymCfg->nBody;
  int j;
  FOR_0(j, nb) {
    PymConstructRbStatedep(sd + j, pymCfg->body + j, dmstreams, pymCfg, cc);
  }
  pym_count_total_num_cp(pymCfg, sd);
  //PymConstructSupportPolygon(pymCfg, sd);

  pym_biped_eqconst_t bipEq;
  PymConstructBipedEqConst(&bipEq, sd, pymCfg, cc);
  //cholmod_print_sparse(bod.bipMat, "bipMat", cc);

  /*
   * TODO [TUNE] Drop tiny values from constraint matrix A
   */
  //cholmod_drop(1e-6, bipEq.bipMat, cc);

  //PrintEntireSparseMatrix(bod.bipMat);
  //__PRINT_VECTOR_VERT(bod.bipEta, bod.bipMat->nrow);
  pym_opt_t pymOpt = PymNewOptimization(&bipEq, sd, pymCfg, &env, cc);
  PymOptimize(&pymOpt);
  pym_pass_opt_result(pureOptTime, _solstaStr, _cost, &pymOpt);
  const double *const xx = pymOpt.xx;
  int ret = 0;
  if (pymOpt.cost != DBL_MAX) {
    //system("cat /tmp/pymoptimize_log");
    pym_analyze_and_update(pymCfg, &pymOpt, outputFile, dmstreams,
			   &bipEq, sd, xx);
    ret = 0;
  } else {
    pym_print_opt_fail_log();
    ret = -1;
  }
  PymDestroyBipedEqconst(&bipEq, cc);
  FOR_0(j, pymCfg->nBody) {
    PymDestroyRbStatedep(sd + j, &pymCfg->body[j].b, cc);
  }
  PymDelOptimization(&pymOpt);
  return ret;
}
