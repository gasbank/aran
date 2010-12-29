/*
* Optimize.c
* 2010 Geoyeob Kim
* As a part of the thesis implementation
*/
#include "PymOptimizerPch.h"
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
#define MAX_NBODY (1024)


static int DevStatCompare(const void * a, const void * b)
{
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

void PymInitializeMosek(MSKenv_t *env)
{
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

void PymCleanupMosek(MSKenv_t *env)
{
  /* Delete the environment and the associated data. */
  MSK_deleteenv(env);
}

void AppendConeRange(MSKtask_t task, int x, int r1, int r2)
{
  /*
  * Append a second-order cone constraint which has
  * the form of x >= norm([ r1, r1+1, ... , r2-1 ])
  */
#define CSUB_MAX_SIZE (4096)
  int csub[CSUB_MAX_SIZE];
  int j = 0;
  MSKrescodee r;
  assert(1 + r2 - r1 <= CSUB_MAX_SIZE);
  assert(r1>=0 && r2>=0 && r2>=r1 && x>=0);
  csub[0] = x;
  for (j=0; j<r2 - r1; ++j) csub[j+1] = r1 + j;
  r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, 1+r2-r1, csub);
  assert(r == MSK_RES_OK);
}

static pym_opt_t PymNewOptimization(const pym_biped_eqconst_t *const bod,
  const pym_rb_statedep_t *const sd,
  const pym_config_t *const pymCfg,
  MSKenv_t *pEnv, cholmod_common *cc)
{
  pym_opt_t o;
  const int NUMCON = bod->bipMat->nrow;
  const int NUMVAR = bod->bipMat->ncol;
  const int nb = pymCfg->nBody;
  memset(&o, 0, sizeof(pym_opt_t));
  /* Number of Ax=b style constraints */
  /* Number of optimization variables. */
  o.bkc     = (MSKboundkeye *)malloc(sizeof(MSKboundkeye) * NUMCON);
  o.blc     = (double *)malloc(sizeof(double) * NUMCON);
  o.buc     = (double *)malloc(sizeof(double) * NUMCON);
  o.bkx     = (MSKboundkeye *)malloc(sizeof(MSKboundkeye) * NUMVAR);
  o.blx     = (double *)malloc(sizeof(double) * NUMVAR);
  o.bux     = (double *)malloc(sizeof(double) * NUMVAR);
  o.c       = (double *)calloc(NUMVAR, sizeof(double));
  o.xx      = (double *)calloc(bod->bipMat->ncol, sizeof(double));
  o._solsta = (MSKsolstae)MSK_RES_OK;
  o.opttime = DBL_MAX;
  o.cost    = DBL_MAX;
  o.bod     = bod;
  o.sd      = sd;
  o.pymCfg  = pymCfg;
  o.pEnv    = pEnv;
  o.cc      = cc;
  return o;
}

void PymDelOptimization(pym_opt_t *o)
{
  free(o->bkc); o->bkc = 0;
  free(o->blc); o->blc = 0;
  free(o->buc); o->buc = 0;
  free(o->bkx); o->bkx = 0;
  free(o->blx); o->blx = 0;
  free(o->bux); o->bux = 0;
  free(o->c); o->c = 0;
  free(o->xx); o->xx = 0;
}

void SET_NONNEGATIVE(const pym_opt_t *const o, const int j)
{
  o->bkx[j] = MSK_BK_LO;
  o->blx[j] = 0;
  o->bux[j] = MSK_INFINITY;
}

void SET_FIXED(const pym_opt_t *const o, const int j,
  const double v)
{
  o->bkx[j] = MSK_BK_FX;
  o->blx[j] = v;
  o->bux[j] = v;
}

void SET_FIXED_ONE(const pym_opt_t *const o, const int j)
{
  SET_FIXED(o, j, 1.0);
}

void SET_FIXED_ZERO(const pym_opt_t *const o, const int j)
{
  SET_FIXED(o, j, 0.0);
}

void SET_LOWER_BOUND(const pym_opt_t *const o, const int j,
  const double lb)
{
  o->bkx[j] = MSK_BK_LO;
  o->blx[j] = lb;
  o->bux[j] = MSK_INFINITY;
}

void SET_UPPER_BOUND(const pym_opt_t *const o, const int j,
  const double ub)
{
  o->bkx[j] = MSK_BK_UP;
  o->blx[j] = -MSK_INFINITY;
  o->bux[j] = ub;
}

void SET_RANGE(const pym_opt_t *const o, const int j,
  const double lb, const double ub)
{
  o->bkx[j] = MSK_BK_RA;
  o->blx[j] = lb;
  o->bux[j] = ub;
}

static void pym_optimize_init_upper_lower(pym_opt_t *pymOpt)
{
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

static void pym_optimize_cost_function(pym_opt_t *pymOpt)
{
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  int i, j;
  double *c = pymOpt->c;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const int nb = pymCfg->nBody;
  const int nf = pymCfg->nFiber;
  const int nj = pymCfg->nJoint;
  int tauOffset;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++)
  {
    FOR_0(j, sd[i].nContacts_1) {
      /*
      * TODO [TUNE] Minimize the contact normal force
      * Walk0, Nav0  - 0
      * Exer0        - 10
      * Jump0        - ?
      * Jump1        - 2
      */
      c[ tauOffset + sd[i].Aci[2] + 5*j + 4 ] = pymCfg->opt_cost_coeffs[oct_normal_force];

      /* Estimated position of z-coordinate of contact point
      * Default: 2e-1, 1e-3      */
      c[ tauOffset + sd[i].Aci[3] + 4*j + 2 ] = pymCfg->opt_cost_coeffs[oct_contact_point_zpos];
      /* Penalty method normal force compensation variable */
      c[ tauOffset + sd[i].Aci[15] + j ] = pymCfg->opt_cost_coeffs[oct_normal_force_nonneg_comp];
    }
    for (j= tauOffset + sd[i].Aci[5];
      j < tauOffset + sd[i].Aci[6]; ++j)
    {
      /*
      * TODO [TUNE] Minimize the movement of candidate contact points
      *
      * Walk0        -  1
      * Nav0         -  5e-1
      * Exer0        -  10
      * Jump0        -  ?
      * Jump1        -  ?
      */
      c[j] = pymCfg->opt_cost_coeffs[oct_contact_point_movement];
    }
    for (j= tauOffset + sd[i].Aci[14];
      j < tauOffset + sd[i].Aci[15]; ++j)
    {
      /*
      * TODO [TUNE] 'eps' >= |p_c_z|
      */
      //c[j] = pymCfg->opt_cost_coeffs[oct_contact_point_zpos_epsilon];
    }
    /*
    * TODO [TUNE] Reference following coefficient
    */
    if (pymCfg->body[i].b.track) {
      c[ tauOffset + sd[i].Aci[8] ] = pymCfg->opt_cost_coeffs[oct_rb_reference_deviation];

      /*const char *rbname = pymCfg->body[i].b.name;
      const bool is_left_leg = strcmp(rbname, "soleL") == 0 || strcmp(rbname, "toeL") == 0;
      const bool is_right_leg = strcmp(rbname, "soleR") == 0 || strcmp(rbname, "toeR") == 0;
      if (is_left_leg || is_right_leg) {
        c[ tauOffset + sd[i].Aci[8] ] *= 0.5;
      }*/
    }
    /*
    * TODO [TUNE] Previous close coefficient
    */
    c[ tauOffset + sd[i].Aci[13] ] = pymCfg->opt_cost_coeffs[oct_rb_previous_deviation];

    /* epsilon F_joint */
    c[ tauOffset + sd[i].Aci[20] ] = pymCfg->opt_cost_coeffs[oct_rb_eps_freecom];
  }
  FOR_0(j, nf) {
    const char *const fibName = pymCfg->fiber[j].b.name;

    c[ Aci[15] + j ] = pymCfg->opt_cost_coeffs[oct_uniform_tension_cost];
    c[ Aci[16] + j ] = pymCfg->opt_cost_coeffs[oct_uniform_actuation_cost];
  }
  /* Cost for COM deviation */
  c[ Aci[10] ] = pymCfg->opt_cost_coeffs[oct_biped_com_deviation];
  /* Cost for torque-around-COM */
  c[ Aci[12] ] = pymCfg->opt_cost_coeffs[oct_torque_around_com];

  assert(Aci[2] - Aci[1] == pymCfg->nFiber);
  assert(Aci[3] - Aci[2] == pymCfg->nFiber);
  /* minimize aggregate tension of actuated muscle fiber */
  c[ Aci[4] ] = pymCfg->opt_cost_coeffs[oct_ligament_actuation]; /* ligament actuation */

  /*
  * Since actuation forces on actuated muscle fibers are
  * non-negative values we can either minimize the second-order
  * cone constraint variable c[Aci[5]] OR
  * the optimization variables c[Aci[2]+j] separately where
  * 'j' is the index of an actuated muscle fiber.
  */
  c[ Aci[5] ] = pymCfg->opt_cost_coeffs[oct_actuated_muscle_actuation]; /* actuated muscle fiber actuation */

  FOR_0(i, pymCfg->nJoint) {
    /* Dislocation constraint */
    const int idx = bod->Aci[7] + i;
    c[ idx ] = pymCfg->opt_cost_coeffs[oct_joint_dislocation];
  }

  c[ Aci[18] ] = pymCfg->opt_cost_coeffs[oct_com_force_deviation_cost];

  /* eps_Fjoint*/
  for (int i = 0; i < 2*nj; ++i) {
    c[ Aci[20] + i ] = pymCfg->opt_cost_coeffs[oct_biped_eps_fjoint];
  }
  /* eps E_Fjoint */
  c[ Aci[22] ] = pymCfg->opt_cost_coeffs[oct_biped_eps_efjoint_xy];
  c[ Aci[23] ] = pymCfg->opt_cost_coeffs[oct_biped_eps_efjoint_z];

  bool is_cost_funtion_nonzero = false;
  for (int i = 0; i < bod->bipMat->ncol; ++i) {
    if (c[i]) {
      is_cost_funtion_nonzero = true;
      break;
    }
  }
  if (!is_cost_funtion_nonzero) {
    std::cout << "INFO - Cost function is always zero." << std::endl;
  }

  pym_rb_named_t *soleL = 0;
  pym_rb_named_t *soleR = 0;
  for (int i = 0; i < nb; ++i) {
    pym_rb_named_t *rbn = &pymCfg->body[i].b;
    const char *rbname = rbn->name;
    if (strcmp(rbname, "soleL") == 0) {
      soleL = rbn;
    } else if (strcmp(rbname, "soleR") == 0) {
      soleR = rbn;
    }
  }


  //const double sole_ydist = fabs(soleL->p[1] - soleR->p[1]);
  //for (int i = 0; i < nb; ++i) {
  //  pym_rb_named_t *rbn = &pymCfg->body[i].b;
  //  const char *rbname = rbn->name;
  //  const bool is_left_leg = strcmp(rbname, "soleL") == 0 || strcmp(rbname, "toeL") == 0;
  //  const bool is_right_leg = strcmp(rbname, "soleR") == 0 || strcmp(rbname, "toeR") == 0;
  //  if (soleL->p[2] > soleR->p[2]) {
  //    // Right is stance leg.
  //    if (is_right_leg && sole_ydist < 0.7) {
  //      FOR_0(j, MAX_CONTACTS) {
  //        if (j%2 == 0) { /* if planting side */
  //          c[ pymOpt->bod->Aici[i] + sd[i].Aci[18] + j] = PymMin(100, 1.0/sole_ydist);
  //        }
  //      }
  //    }
  //  } else {
  //    // Left is stance leg.
  //    if (is_left_leg && sole_ydist < 0.7) {
  //      FOR_0(j, MAX_CONTACTS) {
  //        if (j%2 == 0) { /* if planting side */
  //          c[ pymOpt->bod->Aici[i] + sd[i].Aci[18] + j] = PymMin(100, 1.0/sole_ydist);
  //        }
  //      }
  //    }
  //  }
  //}

  for (int i = 0; i < nb; ++i) {
    pym_rb_named_t *rbn = &pymCfg->body[i].b;
    const char *rbname = rbn->name;
    const bool is_left_leg = strcmp(rbname, "soleL") == 0 || strcmp(rbname, "toeL") == 0;
    const bool is_right_leg = strcmp(rbname, "soleR") == 0 || strcmp(rbname, "toeR") == 0;
    if (is_right_leg) {
      FOR_0(j, MAX_CONTACTS) {
        if (j%2 == 0) { /* if planting side */
          c[ pymOpt->bod->Aici[i] + sd[i].Aci[18] + j] = pymCfg->opt_cost_coeffs[oct_cornerpoint_eps_z];
        }
      }
    }
  }

  c[ Aci[ 26 ] ] = pymCfg->opt_cost_coeffs[oct_biped_eps_sp_deviation];
}

static void pym_optimize_rb_com(pym_opt_t *pymOpt)
{
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const int nd = 3 + pymOpt->pymCfg->nrp;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++)
  {
    /* Set gravitational force */
    for (int j = 0; j < nd; ++j) {
      SET_FIXED(pymOpt, tauOffset + sd[i].Aci[16] + j, sd[i].f_g[j]);
    }

    // Cannot make big change between frames!
    //SET_RANGE( pymOpt, tauOffset + sd[i].Aci[13], 0, 0.3);
    const char *const rbname = pymOpt->pymCfg->body[i].b.name;
    const bool is_left_leg = strcmp(rbname, "soleL") == 0 || strcmp(rbname, "toeL") == 0;
    const bool is_right_leg = strcmp(rbname, "soleR") == 0 || strcmp(rbname, "toeR") == 0;

    //if (strcmp(rbname, "trunk") != 0 && !is_left_leg && !is_right_leg) {
    //if (strcmp(rbname, "trunk") != 0) {
    
    if (true) {
      for (int j = 0; j < nd; ++j) {
        //SET_FIXED_ZERO(pymOpt, tauOffset + sd[i].Aci[19] + j);
      }

      //SET_FIXED_ZERO(pymOpt, tauOffset + sd[i].Aci[19] + 0);
      //SET_FIXED_ZERO(pymOpt, tauOffset + sd[i].Aci[19] + 1);
      //SET_RANGE(pymOpt, tauOffset + sd[i].Aci[19] + 2, -MSK_INFINITY, 0);
    }

    /* chi_2_z: Z-axis of COM for each body should be nonnegative */
    //SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[0] + 2 );

    /* rotation parameterization constraint */
    /* Should not be used with Nav0 reference. */
    //SET_RANGE( pymOpt, tauOffset + sd[i].Aci[11], 0, 1.1*3.14 );

    /* eps_delta: eps_delta > |chi - chi_ref| */
    for(j=tauOffset + sd[i].Aci[8]; j<tauOffset + sd[i].Aci[9]; ++j) {
      //SET_NONNEGATIVE( pymOpt, j );
    }

    if (strcmp(pymOpt->pymCfg->body[i].b.name, "soleL") == 0
      ||strcmp(pymOpt->pymCfg->body[i].b.name, "soleR") == 0
      //	||strcmp(pymOpt->pymCfg->body[i].b.name, "toeL") == 0
      //||strcmp(pymOpt->pymCfg->body[i].b.name, "toeR") == 0
      ) {
        // SET_UPPER_BOUND(pymOpt, tauOffset + sd[i].Aci[8], 0.2);
    }

    if (pymOpt->pymCfg->freeze_pose) {
      SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+0, pymOpt->pymCfg->body[i].b.p[0]);
      SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+1, pymOpt->pymCfg->body[i].b.p[1]);
      SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+2, pymOpt->pymCfg->body[i].b.p[2]);
      //SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+3, pymOpt->pymCfg->body[i].b.q[0]);
      //SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+4, pymOpt->pymCfg->body[i].b.q[1]);
      //SET_FIXED(pymOpt, tauOffset + sd[i].Aci[0]+5, pymOpt->pymCfg->body[i].b.q[2]);
    }
  }
}

static void pym_optimize_biped_com(pym_opt_t *pymOpt)
{
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const int ns_opt = PymMax(0, pymCfg->use_relaxed_ch_as_constraint ? (pymCfg->chVOutputLen-1) : (pymCfg->chOutputLen-1) );
  /* COM norm deviation constraint variable */

  /* TODO: Constraints related to bod->Aci[10] */
  //SET_RANGE(pymOpt, bod->Aci[10], 0, 0.02);

  if (pymCfg->support_polygon_constraint) {
    for (int i = 0; i < ns_opt; ++i) {
      SET_LOWER_BOUND(pymOpt, bod->Aci[24] + i, 0);
    }
  }
}

static void pym_optimize_contact_force(pym_opt_t *pymOpt)
{
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const int nd = 3 + pymOpt->pymCfg->nrp;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++)
  {
    FOR_0(j, sd[i].nContacts_1) {
      
      //SET_RANGE( pymOpt, tauOffset + sd[i].Aci[1] + nd*j + 2, -50, 50 );

      /* no frictional force */
      /*SET_FIXED_ZERO( pymOpt, tauOffset + sd[i].Aci[1] + nd*j + 0);
      SET_FIXED_ZERO( pymOpt, tauOffset + sd[i].Aci[1] + nd*j + 1);
      SET_FIXED_ZERO( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 0);
      SET_FIXED_ZERO( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 1);*/

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
      /*
      * c_c_n
      * TODO [TUNE] Contact normal force constraint tuning
      * Walk0   :  Nonnegative
      * Nav0    :  0~200
      * Exer0   :  0~200 (failed)
      */
      SET_FIXED_ZERO ( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 3 );
      SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[2] + 5*j + 4 );
      
      /* Penalty method normal force compensation variable */
      SET_NONNEGATIVE( pymOpt, tauOffset + sd[i].Aci[15] + j );
    }
  }
}

static void pym_optimize_contact_point(pym_opt_t *pymOpt)
{
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++)
  {
    int nfixpoint = 0;
    /* p_c_2_z: next step CP z-pos */
    FOR_0(j, sd[i].nContacts_1) {
      /* [TUNE] Non-penetration constraint */
      //SET_FIXED_ZERO( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2 );
    }

    const char *rbname = pymOpt->pymCfg->body[i].b.name;
    const bool is_left_leg = strcmp(rbname, "soleL") == 0 || strcmp(rbname, "toeL") == 0;
    const bool is_right_leg = strcmp(rbname, "soleR") == 0 || strcmp(rbname, "toeR") == 0;
    if (is_right_leg) {
      FOR_0(j, MAX_CONTACTS) {
        if (j%2 == 0) {
          //SET_UPPER_BOUND( pymOpt, tauOffset + sd[i].Aci[17] + 4*j + 2, 0.0 );
        }
      }
    }
    
    /*
    * TODO [TUNE] Constraints for fixing contact points
    * p_c_2_z
    */
    FOR_0(j, sd[i].nContacts_1) {
      const char *rbname = pymCfg->body[i].b.name;
      //printf("nplistname = %s\n", pymCfg->body[i].b.name);

      /* Move along the XY-plane strictly probihited.(USELESS!) */
      //SET_UPPER_BOUND ( pymOpt, tauOffset + sd[i].Aci[4] + 4*j + 0, 0.0 );
      //SET_UPPER_BOUND ( pymOpt, tauOffset + sd[i].Aci[4] + 4*j + 1, 0.0 );
      //SET_UPPER_BOUND(pymOpt, tauOffset + sd[i].Aci[5] + j, 0.005);
      const int cidx = sd[i].contactIndices_1[j];
      const int isFoot =
        strcmp(rbname, "soleL") == 0 ||	strcmp(rbname, "soleR") == 0 ||
        strcmp(rbname, "toeL") == 0 || strcmp(rbname, "toeR") == 0;
      assert(0 <= j && j <= 7);
      assert(0 <= cidx && cidx <= 7);

      if (nfixpoint < 3 && cidx%2 == 0 && isFoot ) { /* if sole side */
        //SET_UPPER_BOUND(pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2, 0);
        /* Next time step Z-position of contact points should remain 0 */
        //SET_RANGE(pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2, -100, 0.05 );
        /* SET_RANGE ( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 2, */
        /* 	    0, 0.05); */
        ++nfixpoint;
      }
    }
    /* p_c_2_w */
    FOR_0(j, sd[i].nContacts_1) {
      SET_FIXED_ONE  ( pymOpt, tauOffset + sd[i].Aci[3] + 4*j + 3 );
    }
  }
}

static void pym_optimize_anchored_joint(pym_opt_t *pymOpt)
{
  int i, j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  FOR_0(i, pymCfg->nJoint) {
    /* Fix homogeneous components anchored joint
    dislocation vectors (d_A) to 0 */
    SET_FIXED_ZERO( pymOpt, bod->Aci[6] + 4*i + 3 ); 

    /* [TUNE] Joint dislocation threashold */
    if (pymCfg->joint_dislocation_enabled) {
      assert(pymCfg->joint_dislocation_threshold >= 0);
      
      SET_RANGE( pymOpt, bod->Aci[7] + i, 0, pymCfg->joint_dislocation_threshold );

      for (int j = 0; j < 3; ++j) {
        //SET_RANGE(pymOpt, Aci[6] + 4*i + j, -1e-5, 1e-5);

        //SET_FIXED_ZERO(pymOpt, Aci[6] + 4*i + j);
      }
    }
  }
  // F_joint is a force vector. Fix homogeneous part to 0.
  for (int i = 0; i < 2*pymCfg->nJoint; ++i) {
    SET_NONNEGATIVE(pymOpt, Aci[20] + i);

    //SET_RANGE(pymOpt, Aci[19] + 4*i + 0, -2000, 2000);
    //SET_RANGE(pymOpt, Aci[19] + 4*i + 1, -2000, 2000);
    //SET_RANGE(pymOpt, Aci[19] + 4*i + 2, -2000, 2000);
    SET_FIXED(pymOpt, Aci[19] + 4*i + 3, 0);
  }
  // Testing. No help force
  for (int i = 0; i < 4; ++i) {
    SET_FIXED(pymOpt, Aci[21] + i, 0);
  }
}

static void pym_optimize_muscle(pym_opt_t *pymOpt)
{
  int i, j;
  const pym_config_t *pymCfg = pymOpt->pymCfg;
  const int nd = 3 + pymCfg->nrp;
  const int nb = pymCfg->nBody;
  const int nf = pymCfg->nFiber;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  FOR_0(j, nf) {
    pym_mf_named_t *mf = &pymCfg->fiber[j].b;
    const pym_muscle_type_e mt = mf->mType;
    const char *fibName = mf->name;
    /* Tension range constraint */
    i = Aci[1]+j;
    if (mt == PMT_JOINT_MUSCLE) {
      //SET_FIXED_ZERO(pymOpt, i);
    } else if (mt == PMT_ACTUATED_MUSCLE) {
      SET_RANGE(pymOpt, i, 0, 0);
    } else if (mt == PMT_LIGAMENT) {
      SET_RANGE(pymOpt, i, 0, 0);
    }

    /* Actuation force range constraint */
    i = Aci[2]+j;
    if (mt == PMT_JOINT_MUSCLE) {

    } else if (mt == PMT_ACTUATED_MUSCLE) {
      //SET_RANGE(pymOpt, i, 0, 1);
    } else if (mt == PMT_LIGAMENT) {
      //SET_RANGE(pymOpt, i, 0, 1);
    }

    /* Rest length range constraint */
    i = Aci[3]+j;
    /*SET_RANGE( pymOpt, i, pymCfg->fiber[j].b.xrest_lower,
      pymCfg->fiber[j].b.xrest_upper );*/
    //SET_RANGE( pymOpt, i, 0.5, 0.6);
  }

  for (int i = 0; i < nb; ++i) {
    pym_rb_named_t *rbn = &pymCfg->body[i].b;
    for (int j = 0; j < rbn->nFiber; ++j) {
      pym_mf_named_t *mfn = &pymCfg->fiber[ rbn->fiber[j] ].b;
      // Actuated and ligament fibers can be degenerated, i.e.,, its length
      // become 0 or very near to 0.

      /*if (mfn->degenerated && mfn->mType != PMT_JOINT_MUSCLE) {
        for (int k = 0; k < nd; ++k) {
          SET_FIXED(pymOpt, pymOpt->bod->Aici[i] + pymOpt->sd[i].Aci[9] + nd*j + k, 0);
        }
      }*/
    }
  }
}

static void pym_optimize_upper_lower(pym_opt_t *pymOpt) {
  /* x_l < x < x_u style constraints */
  pym_optimize_rb_com(pymOpt);
  pym_optimize_biped_com(pymOpt);
  pym_optimize_contact_force(pymOpt);
  pym_optimize_contact_point(pymOpt);
  pym_optimize_anchored_joint(pymOpt);
  pym_optimize_muscle(pymOpt);
}

static MSKtask_t pym_optimize_init_mosek_task(MSKenv_t *pEnv,
  int NUMVAR, int NUMCON,
  int NUMANZ)
{
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
  /*r = MSK_putintparam (task , MSK_IPAR_DATA_CHECK, MSK_OFF);
  assert(r == MSK_RES_OK);*/
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
  MSKtask_t task)
{
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
  MSKtask_t task)
{
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
      /* Pointer to row indexes of column j. */
      int *rowIdxOfColJ = (int *)(bod->bipMat->i) + colstart;
      /* Pointer to Values of column j. */
      double *valOfColJ = (double *)(bod->bipMat->x) + colstart;
      __annz += colend-colstart;

      r = MSK_putavec(task,
        MSK_ACC_VAR,     /* Input columns of A.*/
        j,               /* Variable (column) index.*/
        colend-colstart, /* Number of non-zeros in column j.*/
        rowIdxOfColJ,
        valOfColJ);
    }
  }
  const int bipMat_nnz = cholmod_nnz(bod->bipMat, pymOpt->cc);
  assert(__annz == bipMat_nnz);
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
  MSKtask_t task)
{
  int i, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
      /* Reference trajectory cone constraints, i.e.,
      * epsilon_Delta >= || Delta_chi_{i,ref} || (6-DOF)      */
      if (pymOpt->c[ tauOffset + sd[i].Aci[8] ]) {
        AppendConeRange(task,
          tauOffset + sd[i].Aci[8],
          tauOffset + sd[i].Aci[7],
          tauOffset + sd[i].Aci[8]);
      }
      
      /* Previous state close cone constraints, i.e.,
      * epsilon_Delta >= || Delta_chi_{i,prv} || (6-DOF)      */
      AppendConeRange(task,
      tauOffset + sd[i].Aci[13],
      tauOffset + sd[i].Aci[12],
      tauOffset + sd[i].Aci[13]);

      /* eps_freecom */
      if (pymOpt->c[ tauOffset + sd[i].Aci[20] ]) {
        AppendConeRange(task,
          tauOffset + sd[i].Aci[20],
          tauOffset + sd[i].Aci[19],
          tauOffset + sd[i].Aci[20]);
      }
  }
}

static void pym_optimize_mosek_cone_contact_point
  (const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++)
  {
    for (int j = 0; j < sd[i].nContacts_1; ++j) {
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
      /*
      * TODO [TUNE] CP Z-pos: eps >= |p_c_z|
      */
      if (pymOpt->c[ tauOffset + sd[i].Aci[14] + j ]) {
        AppendConeRange(task,
          tauOffset + sd[i].Aci[14] + j,
          tauOffset + sd[i].Aci[3]+4*j+2,
          tauOffset + sd[i].Aci[3]+4*j+3);
      }
    }
    for (int j = 0; j < MAX_CONTACTS; ++j) {
      /*
      * TODO [TUNE] Corner point Z-pos: eps >= |p_all_z|
      */
      if (pymOpt->c[ tauOffset + sd[i].Aci[18] + j ]) {
        AppendConeRange(task,
          tauOffset + sd[i].Aci[18] + j,
          tauOffset + sd[i].Aci[17]+4*j+2,
          tauOffset + sd[i].Aci[17]+4*j+3);
      }
    }
  }
}

static void pym_optimize_mosek_cone_contact_force
  (const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  int i, j, tauOffset;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  for (i = 0, tauOffset = 0; i < nb;
    tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
      FOR_0(j, sd[i].nContacts_1) {
        if (sd[i].contactTypes_1[j] == static_contact) {
          /* Friction cone constraints */
          AppendConeRange(task,
            tauOffset + sd[i].Aci[6]+j,            // mu*c_n
            tauOffset + sd[i].Aci[2]+5*j+0,        // c_tx ~ c_tz
            tauOffset + sd[i].Aci[2]+5*j+3);
        }
      }
  }
}

static void pym_optimize_mosek_cone_anchored_joint
  (const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  int j;
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const int nj = pymCfg->nJoint;
  FOR_0(j, pymCfg->nJoint) {
    /* Anchored joint dislocation constraints */
    AppendConeRange(task,
      Aci[7] + j,                      // epsilon_d
      Aci[6] + 4*j,                    // dAx ~ dAz
      Aci[6] + 4*j + 3);
  }

  // Fjoint eps - Joint muscle minimization cone
  for (int i = 0; i < 2*nj; ++i) {
    if (pymOpt->c[ Aci[20] + i ]) {
      AppendConeRange(task, Aci[20] + i, Aci[19] + 4*i, Aci[19] + 4*(i+1));
    }
  }
  // E_Fjoint_xy eps
  if (pymOpt->c[ Aci[22] ])
    AppendConeRange(task, Aci[22], Aci[21], Aci[21]+2);
  // E_Fjoint_z eps
  if (pymOpt->c[ Aci[23] ])
    AppendConeRange(task, Aci[23], Aci[21]+2, Aci[21]+3);
}

static void pym_optimize_mosek_cone_muscle
  (const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  MSKrescodee r = MSK_RES_OK;
  /*
  * TODO [TUNE] Cone constraints for minimizing tension/actuation forces
  */
  const int nf = pymCfg->nFiber;
  const int nj = pymCfg->nJoint;
  
  /* cones for tensions (sum of absolute values; Manhattan distance; L_1 distance) */
  for (int i = 0; i < nf; ++i) {
    int ab[2] = { Aci[15] + i, Aci[1] + i }; /* a >= |b| */
    //r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, 2, ab);
    AppendConeRange(task, Aci[15] + i, Aci[1] + i, Aci[1] + i+1);
    assert(r == MSK_RES_OK);
  }
  /* cones for actuations (sum of absolute values; Manhattan distance; L_1 distance) */
  for (int i = 0; i < nf; ++i) {
    int ab[2] = { Aci[16] + i, Aci[2] + i }; /* a >= |b| */
    //r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, 2, ab);
    AppendConeRange(task, Aci[16] + i, Aci[2] + i, Aci[2] + i+1);
    assert(r == MSK_RES_OK);
  }
}

static void pym_optimize_mosek_cone_biped(const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  const pym_biped_eqconst_t *const bod = pymOpt->bod;
  const int *const Aci = bod->Aci;
  /* Reference COM - COM constraint */
  AppendConeRange(task,
    Aci[10],                      // epsilon_com
    Aci[9],                     // delta p_{com,ref}
    Aci[10]);
  /* SP center - COM constraint */
  AppendConeRange(task,
    Aci[26],                      // epsilon_{com,spcen}
    Aci[25],                     // delta p_{com,spcen} (xy plane only)
    Aci[26]);
  /* COM force deviation */
  AppendConeRange(task,
    Aci[18],                      // epsilon_com
    Aci[17],                     // delta p_{com,ref} (z-axis only)
    Aci[18]);
  ///* Torque-about-COM constraint cone */
  //AppendConeRange(task,
  //  Aci[12], /* epsilon_{tau,com} */
  //  Aci[11], /* Torque-about-COM (tau_com) */
  //  Aci[12]);
}

static void pym_optimize_mosek_cone(const pym_opt_t *const pymOpt,
  MSKtask_t task)
{
  pym_optimize_mosek_cone_rb(pymOpt, task);
  pym_optimize_mosek_cone_contact_point(pymOpt, task);
  pym_optimize_mosek_cone_contact_force(pymOpt, task);
  pym_optimize_mosek_cone_anchored_joint(pymOpt, task);
  //pym_optimize_mosek_cone_biped(pymOpt, task);
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
  MSKrescodee taskr)
{
  double cost = DBL_MAX;
  const int NUMVAR = pymOpt->bod->bipMat->ncol;
  static const char *unknownMsg =
    "   ***   The status of the solution could not be determined.   ***\n";

  MSK_getdouinf ( task , MSK_DINF_OPTIMIZER_TIME , &pymOpt->opttime );
  /* Print a summary containing information
  about the solution for debugging purposes*/
  MSK_solutionsummary (task, MSK_STREAM_LOG);
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
    char symname[MSK_MAX_STR_LEN];
    char desc[MSK_MAX_STR_LEN];

    printf("Error - An error occurred while optimizing.\n");
    printf("        Refer to /tmp/pymoptimize_log and pymuscle_c.opf\n");
    printf("        for further investigation.\n");
    MSK_writedata(task, "pymuscle_c.opf");

    /* In case of an error print error code and description. */
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

  /* Number of Ax=b style constraints */
  const int NUMCON = bod->bipMat->nrow;
  /* Number of optimization variables. */
  const int NUMVAR = bod->bipMat->ncol;
  /* Number of non-zeros in A. */
  const int NUMANZ = cholmod_nnz(bod->bipMat, cc);
  MSKrescodee  r;
  MSKtask_t task;
  /* If some error or infeasibility condition detected from
  * MOSEK, the user can inspect this log file.     */
  FILE *mosekLogFile = 0;
  assert(pymCfg->nJoint >= 0);

  /* Configure optimization data (without MOSEK 'task' involved.) */
  pym_optimize_init_upper_lower(pymOpt);
  if (pymCfg->zero_cost_func) {
    
  } else {
    pym_optimize_cost_function(pymOpt);
  }
  pym_optimize_upper_lower(pymOpt);

  task = pym_optimize_init_mosek_task(pEnv, NUMVAR, NUMCON, NUMANZ);
  if (!task) {
    printf("Error - MOSEK task init failed.\n");
    pymOpt->cost = DBL_MAX;
    return;
  }
  /* TODO MOSEK optimization logging */
  std::string workingPath(getenv("WORKING"));
  std::string logPath = workingPath + "/pymoptimize_log.txt";
  mosekLogFile = fopen(logPath.c_str(), "w");
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
  std::string opfPath = workingPath + "/a.opf";
  char *opfPath_raw = (char *)malloc(sizeof(char)*opfPath.length()+1);
  strcpy(opfPath_raw, opfPath.c_str());
  r = MSK_writedata(task, opfPath_raw);
  assert(r == MSK_RES_OK);
  free(opfPath_raw);
  r = pym_optimize_mosek_optimize(task);
  pym_optimize_mosek_analyze_result(pymOpt, task, r);

  if (mosekLogFile)
    fclose(mosekLogFile);
  /* Delete the task and the associated data. */
  MSK_deletetask(&task);
}

void PymConstructSupportPolygon(pym_config_t *pymCfg,
  pym_rb_statedep_t *sd)
{
  int i, j;
  const int nb = pymCfg->nBody;
  pymCfg->chInputLen = 0;
  pymCfg->chVInputLen = 0;
  for (int i = 0; i < nb; ++i) {
    pym_rb_named_t *rbn = &pymCfg->body[i].b;
    pym_rb_statedep_t *sdi = sd + i;

    // Physical contact points
    FOR_0(j, sdi->nContacts_1) {
      pymCfg->chInput[ pymCfg->chInputLen ].x = sdi->contacts_1[ sdi->contactIndices_1[j] ][0];
      pymCfg->chInput[ pymCfg->chInputLen ].y = sdi->contacts_1[ sdi->contactIndices_1[j] ][1];
      ++pymCfg->chInputLen;
    }

    // Virtual contact points
    for (int j = 0; j < 8; ++j) {
      double pcj_1_W[3], pcj_0_W[3], pcj_ref[3];
      AffineTransformPoint(pcj_1_W, sdi->W_1, rbn->corners[j]);
      AffineTransformPoint(pcj_0_W, sdi->W_0, rbn->corners[j]);
      
      double groundLevel = 0.1;
      if (pcj_1_W[2] <= groundLevel) {
        pymCfg->chVInput[ pymCfg->chVInputLen ].x = pcj_1_W[0];
        pymCfg->chVInput[ pymCfg->chVInputLen ].y = pcj_1_W[1];
        ++pymCfg->chVInputLen;
      }
    }
  }
  
  assert(pymCfg->chInputLen < 1000);
  if (pymCfg->chInputLen > 0) {
    pymCfg->chOutputLen = PymConvexHull(pymCfg->chInput,
      pymCfg->chInputLen, pymCfg->chOutput);
  } else {
    pymCfg->chOutputLen = 0;
  }
  assert(pymCfg->chInputLen + 1 >= pymCfg->chOutputLen);

  assert(pymCfg->chVInputLen < 1000);
  if (pymCfg->chVInputLen > 0) {
    pymCfg->chVOutputLen = PymConvexHull(pymCfg->chVInput,
      pymCfg->chVInputLen, pymCfg->chVOutput);
  } else {
    pymCfg->chVOutputLen = 0;
  }
  assert(pymCfg->chVInputLen + 1 >= pymCfg->chVOutputLen);
}

static double pym_analyze_rb_deviation_stat(const pym_opt_t *const pymOpt,
  FILE *outputFile,
  FILE *dmstreams[])
{
  const double *const xx = pymOpt->xx;
  const int nb = pymOpt->pymCfg->nBody;
  const pym_config_t *const pymCfg = pymOpt->pymCfg;
  const pym_rb_statedep_t *const sd = pymOpt->sd;
  deviation_stat_entry dev_stat[MAX_NBODY /* nb */];
  int j, tauOffset;
  FILE *dmst; 
  const int itemsPerLine = PymMin(nb, 6);
  int j0 = 0, j1 = itemsPerLine;
  double chi_d_norm_sum = 0;
  memset(dev_stat, 0, sizeof(deviation_stat_entry)*nb);
  for (j = 0, tauOffset = 0;
    j < nb;
    tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++)
  {
    const double *chi_2 = xx + tauOffset;
    const pym_rb_named_t *rbn = &pymCfg->body[j].b;
    double chi_1[3+4], chi_r[3+4], chi_v[3+4];
    double chi_d[3+4];
    int k;
    memcpy(chi_1    , rbn->p,       sizeof(double)*3);
    memcpy(chi_1 + 3, rbn->q,       sizeof(double)*4);
    memcpy(chi_r    , rbn->chi_ref, sizeof(double)*7);
    memcpy(chi_v    , rbn->pd,      sizeof(double)*3);
    memcpy(chi_v + 3, rbn->qd,      sizeof(double)*4);

    FOR_0(k, 6) {
      chi_d[k] = chi_2[k] - chi_r[k];
      dev_stat[j].chi_d_norm += chi_d[k] * chi_d[k];
    }
    dev_stat[j].chi_d_norm = sqrt(dev_stat[j].chi_d_norm);
    dev_stat[j].bodyIdx = j;
    chi_d_norm_sum += dev_stat[j].chi_d_norm;
    if (outputFile) {
      FOR_0(k, 6) fprintf(outputFile, "%18.8e", chi_2[k]);
      fprintf(outputFile, "\n");
    }
  }
  qsort(dev_stat, nb, sizeof(deviation_stat_entry), DevStatCompare);

  dmst = dmstreams[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT];
  fprintf(dmst, "Reference trajectory deviation report\n");
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
      fprintf(dmst, "  %9d", sd[j].nContacts_1);
    }
    fprintf(dmst, "\n");
    j0 = PymMin(nb, j0 + itemsPerLine);
    j1 = PymMin(nb, j1 + itemsPerLine);
  }

  return chi_d_norm_sum;
}

static void pym_update_rb_and_biped_com
  (pym_config_t *pymCfg,
  const pym_opt_t *const pymOpt,
  const pym_biped_eqconst_t *const bipEq)
{
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
}

static void pym_update_muscle
  (pym_config_t *pymCfg,
  const pym_opt_t *const pymOpt,
  const pym_biped_eqconst_t *const bipEq)
{
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
  FILE *dmstreams[])
{
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
  FILE *dmstreams[])
{
  const double *const xx = pymOpt->xx;
  FILE *dmst = dmstreams[PDMTE_FBYF_REF_COM_DEVIATION_REPORT];
  const double *const comDev = xx + bipEq->Aci[9];
  const double comDevLenSq = PymNormSq(3, comDev);
  fprintf(dmst, "COM deviation: dev=[%lf,%lf,%lf], |dev|^2=%lf\n",
    comDev[0], comDev[1], comDev[2], comDevLenSq);
}

static void pym_count_total_num_cp(pym_config_t *pymCfg,
  pym_rb_statedep_t *sd)
{
  pymCfg->prevTotContacts = pymCfg->curTotContacts;
  pymCfg->curTotContacts  = 0;
  int j;
  FOR_0(j, pymCfg->nBody) {
    pymCfg->curTotContacts += sd[j].nContacts_1;
  }
}

static void pym_pass_opt_result(double *pureOptTime, const char **_solstaStr,
  double *_cost, const pym_opt_t *const pymOpt)
{
  if (pureOptTime) {
    *pureOptTime = pymOpt->opttime;
  }
  if (pymOpt->cost == DBL_MAX) {
    printf("Something goes wrong while optimizing.\n");
    for (int i = 0; i < pymOpt->pymCfg->nBody; ++i) {
      std::cout << pymOpt->pymCfg->body[i] << std::endl;
    }
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
  const double *const xx)
{
  const int nd = 3 + pymCfg->nrp;
  const int nb = pymCfg->nBody;
  int j, k, tauOffset;
  for (j = 0, tauOffset = 0; j < nb;
    tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++)
  {
    const double *const chi_2 = xx + tauOffset;
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    const pym_rb_statedep_t *sdj = sd + j;
    rbn->nContacts_1 = sdj->nContacts_1;
    FOR_0(k, sdj->nContacts_1) {
      memcpy(rbn->contactsPoints_1[k], sdj->contacts_1[ sdj->contactIndices_1[k] ],
        sizeof(double)*3);
      memcpy(rbn->contactsPoints_2[k], chi_2 + sdj->Aci[3] + 4*k,
        sizeof(double)*3);
      memcpy(rbn->contactsForce_2[k],   chi_2 + sdj->Aci[1] + nd*k,
        sizeof(double)*3);
    }
  }
}

static void pym_analyze_cp_slip(pym_config_t *pymCfg,
  pym_rb_statedep_t *sd,
  const double *const xx)
{
  const int nb = pymCfg->nBody;
  int j, k, tauOffset;
  for (j = 0, tauOffset = 0; j < nb;
    tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++)
  {
    //const double *chi_2 = xx + tauOffset;
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    const pym_rb_statedep_t *sdj = sd + j;

    rbn->nContacts_1 = sdj->nContacts_1;
    FOR_0(k, sdj->nContacts_1) {
      //const double epsil = *(chi_2 + sdj->Aci[5] + k);
      //const double zpos  = *(chi_2 + sdj->Aci[3] + 4*k + 2);
      /* printf("%s [%d <%d>] - CP dev %lf / Z-pos %lf\n", */
      /* 	     rbn->name, k, sdj->contactIndices_2[k], epsil, zpos); */
    }
  }
}

void pym_check_muscle_fiber_force( pym_opt_t *pymOpt ) 
{
  const int nb = pymOpt->pymCfg->nBody;
  const int nd = 3 + pymOpt->pymCfg->nrp;
  double tot_muscle_force[3] = {0,};
  double tot_joint_muscle_force[3] = {0,};
  for (int i = 0; i < nb; ++i) {
    pym_rb_named_t *rbn = &pymOpt->pymCfg->body[i].b;
    for (int j = 0; j < rbn->nFiber; ++j) {
      pym_mf_named_t *mfn = &pymOpt->pymCfg->fiber[ rbn->fiber[j] ].b;
      assert(mfn->mType != PMT_UNKNOWN);
      if (mfn->mType != PMT_JOINT_MUSCLE) {
        for (int k = 0; k < 3; ++k) {
          tot_muscle_force[k] += pymOpt->xx[ pymOpt->bod->Aici[i] + pymOpt->sd[i].Aci[ 9 ] + nd*j + k ];
        }
      } else {
        //printf("rbn->name = %s\n", rbn->name);
        for (int k = 0; k < 3; ++k) {
          tot_joint_muscle_force[k] += pymOpt->xx[ pymOpt->bod->Aici[i] + pymOpt->sd[i].Aci[ 9 ] + nd*j + k ];
          //printf("%lf\n", pymOpt->xx[ pymOpt->bod->Aici[i] + pymOpt->sd[i].Aci[ 9 ] + nd*j + k ]);
        }
        //printf("-----****-----\n");
      }
    }
  }
  
  /*for (int i = pymOpt->bod->Aci[19]; i < pymOpt->bod->Aci[20]; ++i) {
    if ((i - pymOpt->bod->Aci[19])%4 == 0)
      printf("......!!.....\n");
    printf("%lf\n", pymOpt->xx[i]);
  }*/

  const double nrm = PymNorm(3, tot_muscle_force);
  const double joint_nrm = PymNorm(3, tot_joint_muscle_force);
  if (pymOpt->_solsta == MSK_SOL_STA_OPTIMAL || pymOpt->_solsta == MSK_SOL_STA_NEAR_OPTIMAL) {
    assert(-1e-8 <= nrm && nrm <= 1e-8);
    assert(-1e-2 <= joint_nrm && joint_nrm <= 1e-2);
  }
}

void pym_analyze_com_force_deviation_stat( pym_opt_t * pymOpt, pym_biped_eqconst_t * bipEq, FILE * dmstreams[] ) 
{
  fprintf(dmstreams[PDMTE_FBYF_COM_FORCE_DEVIATION_REPORT], "COM force deviation: %lf %lf %lf\n",
    pymOpt->xx [ bipEq->Aci[17] + 0 ],
    pymOpt->xx [ bipEq->Aci[17] + 1 ],
    pymOpt->xx [ bipEq->Aci[17] + 2 ]);
}

static void pym_analyze_and_update(pym_config_t *pymCfg,
  pym_opt_t *pymOpt,
  FILE *outputFile,
  FILE *dmstreams[],
  pym_biped_eqconst_t *bipEq,
  pym_rb_statedep_t *sd,
  const double *const xx)
{
  pymCfg->chi_d_norm_sum = pym_analyze_rb_deviation_stat(pymOpt, outputFile, dmstreams);
  
  pym_update_rb_and_biped_com(pymCfg, pymOpt, bipEq);
  pym_update_muscle(pymCfg, pymOpt, bipEq);
  pym_analyze_anchored_joint_stat(pymCfg, pymOpt, bipEq, dmstreams);
  pym_analyze_biped_com_deviation_stat(pymOpt, bipEq, dmstreams);
  pym_analyze_com_force_deviation_stat(pymOpt, bipEq, dmstreams);
  pym_copy_cp_and_cf(pymCfg, sd, xx);
  pym_analyze_cp_slip(pymCfg, sd, xx);
  //pym_exhaustive_solution_vector_info(*pymOpt); //test
  //pym_check_muscle_fiber_force(pymOpt);
  pym_update_com( pymCfg );
}

static void pym_print_opt_fail_log()
{
  printf("Optimization failure report\n");
  printf(HORIZONTAL_LINE_NL);
  int ret = 0;
  //ret = system("cat /tmp/pymoptimize_log");
  printf(HORIZONTAL_LINE_NL);
  if (ret)
    printf("Warning - /tmp/pyoptimize_log opening failure.\n");
}

void PymComputeBipedStatedep( pym_config_t * pymCfg, const pym_rb_statedep_t *const sd ) 
{
  static bool run_once = false;
  const size_t nb = pymCfg->nBody;
  const size_t nf = pymCfg->nFiber;
  for (size_t j = 0; j < nf; ++j) {
    const double *attPos1, *attPos2;
    pym_mf_named_t *mf = &pymCfg->fiber[ j ].b;
    double	pt1[3], pt2[3], direction[3];
    AffineTransformPoint(pt1, sd[ mf->org ].W_1, mf->fibb_org);
    AffineTransformPoint(pt2, sd[ mf->ins ].W_1, mf->fibb_ins);
    for (size_t k = 0; k < 3; ++k)
      direction[k] = pt2[k] - pt1[k];
    double direction_original[3];
    memcpy(direction_original, direction, sizeof(double)*3);
    double	dirLen = PymNorm(3, direction);
    
    memcpy(mf->disloc_dir, direction, sizeof(double)*3);
    mf->disloc_len0 = mf->disloc_len;
    mf->disloc_len = dirLen;
    
    if (mf->disloc_len > DEGENERATE_THRESHOLD) {
      NormalizeVector(3, mf->disloc_dir);
    }
  }
  if (!run_once) {
    for (size_t i = 0; i < nf; ++i) {
      pym_mf_named_t *mf = &pymCfg->fiber[ i ].b;
      mf->disloc_len0 = mf->disloc_len;
    }
    run_once = true;
  }
}

int PymOptimizeFrameMove(double *pureOptTime, FILE *outputFile,
  pym_config_t *pymCfg, pym_rb_statedep_t *sd,
  FILE *dmstreams[],
  const char **_solstaStr, double *_cost,
  cholmod_common *cc, MSKenv_t env)
{
  const int nb = pymCfg->nBody;
  int j;
  FOR_0(j, nb) {
    PymConstructRbStatedep(sd, sd + j, pymCfg->body + j, dmstreams, pymCfg, cc);
  }
  pym_count_total_num_cp(pymCfg, sd);
  PymConstructSupportPolygon(pymCfg, sd);

  PymComputeBipedStatedep(pymCfg, sd);

  pym_biped_eqconst_t bipEq;
  PymConstructBipedEqConst(&bipEq, sd, pymCfg, cc);
  //cholmod_print_sparse(bipEq.bipMat, "bipMat", cc);

  /*
  * TODO [TUNE] Drop tiny values from constraint matrix A
  */
  //cholmod_drop(1e-6, bipEq.bipMat, cc);
  
  /*cholmod_dense *ddd = cholmod_sparse_to_dense(bipEq.bipMat, cc);
  cholmod_print_dense(ddd, "ddd", cc);
  __PRINT_VECTOR_VERT_EXCEPT_ZERO(((double *)ddd->x), (ddd->ncol*ddd->nrow));
  __PRINT_VECTOR_VERT(bipEq.bipEta, bipEq.bipMat->nrow);*/

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
    if (dmstreams[PDMTE_FBYF_SOLUTION_VECTOR] == stdout)
      pym_exhaustive_solution_vector_info(pymOpt);
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

inline std::string stream_indent(int indent_level) {
  const static int unit_indent = 2;
  std::string s(indent_level * unit_indent, ' ');
  return s;
}

struct solution_vector_slice {
  std::string name;
  int unit_size;
  int count;
};
const static int RB_A_BLOCK_SIZE = -1; /* Size of A_block for each RB_i */
const static int RB_NP = -2; /* # of contact points for each RB_i */
const static int RB_NF = -3; /* # of fibers for each RB_i */
const static int RB_NJ = -4; /* # of anchored joints for each RB_i */

int pym_calculate_slice_size(const pym_opt_t &pymOpt, int rb_index, const solution_vector_slice &slice) {
  int real_count = 0, real_unit_size = 0;

  if (slice.count >= 0) {
    real_count = slice.count;
  } else if (slice.count == RB_NP) {
    assert(0 <= rb_index && rb_index < pymOpt.pymCfg->nBody);
    real_count = pymOpt.pymCfg->body[rb_index].b.nContacts_1;
  } else if (slice.count == RB_NF) {
    assert(0 <= rb_index && rb_index < pymOpt.pymCfg->nBody);
    real_count = pymOpt.pymCfg->body[rb_index].b.nFiber;
  } else if (slice.count == RB_NJ) {
    assert(0 <= rb_index && rb_index < pymOpt.pymCfg->nBody);
    real_count = pymOpt.pymCfg->body[rb_index].b.nAnchor;
  } else {
    assert(!"Unknown count symbol");
  }

  if (slice.unit_size >= 0) {
    real_unit_size = slice.unit_size;
  } else if (slice.unit_size == RB_A_BLOCK_SIZE) {
    // SPECIAL CASE: A_matrix part
    real_unit_size = pymOpt.bod->Aci[1];
    real_count = 1;
  } else {
    assert(!"Uknown unit size symbol");
  }

  assert(real_unit_size >= 0);
  assert(real_count >= 0);
  return real_count * real_unit_size;
}

void pym_exhaustive_solution_vector_info(const pym_opt_t &pymOpt)
{
  const pym_config_t *const pymCfg = pymOpt.pymCfg;
  const int ns_opt = PymMax(0, pymCfg->use_relaxed_ch_as_constraint ? (pymCfg->chVOutputLen-1) : (pymCfg->chOutputLen-1) );
  const solution_vector_slice block_slices[] = {
    {"tau (rigid body)", RB_A_BLOCK_SIZE, pymCfg->nBody},
    {"T (tension)", 1, pymCfg->nFiber},
    {"u (actuation)", 1, pymCfg->nFiber},
    {"x_r (muscle rest length)", 1, pymCfg->nFiber},
    {"eps_ligten (ligament tension epsilon)", 1, 1},
    {"eps_actten (actuated tension epsilon)", 1, 1},
    {"d_A (anchored joint dislocation)", 4, pymCfg->nJoint},
    {"eps_d (anchored joint dislocation epsilon)", 1, pymCfg->nJoint},
    {"p_com^(l+1) (estimated next step COM position)", 3, 1},
    {"delta p_{com,ref} (difference between reference COM and estimated next step COM)", 3, 1},
    {"eps_com (COM dev. epsilon)", 1, 1},
    {"tau_com (...)", 3, 1},
    {"eps_tau_com (...)", 1, 1},
    {"eps_ligact (ligament actuation epsilon)", 1, 1},
    {"eps_actact (actuated actuation epsilon)", 1, 1},
    {"eps_T (tension epsilon)", 1, pymCfg->nFiber},
    {"eps_u (actuation epsilon)", 1, pymCfg->nFiber},
    {"f_comfdev (difference between desired COM force and current COM force)", 3, 1},
    {"eps_comfdev (f_comfdev epsilon)", 1, 1},
    {"F_joint (Free force acting on anchored joints)", 4, 2*pymCfg->nJoint},
    {"eps_Fjoint (eps F_joint)", 1, 2*pymCfg->nJoint},
    {"E_Fjoint (Sum of all F_joint forces)", 4, 1},
    {"eps_EFjoint (eps E_Fjointxy)", 1, 1},
    {"eps_EFjoint (eps E_Fjointz)", 1, 1},
    {"X_sp (Support polygon constraint: should be nonnegative number)", 1, ns_opt},
    {"delta p_{com,spcen} (difference between center of SP and estimated next step COM)", 2, 1},
    {"eps_{com,spcen} (delta p_{com,spcen} epsilon)", 1, 1},
  };
  BOOST_STATIC_ASSERT( 1+sizeof(block_slices)/sizeof(block_slices[0]) == sizeof(pymOpt.bod->Aci)/sizeof(pymOpt.bod->Aci[0]) );
  const int nd = 3 + pymCfg->nrp;
  
  const solution_vector_slice A_block_slices[] = {
    {"chi^(l+1) (next state)", nd, 1},
    {"f_c (contact force)", nd, RB_NP},
    {"c_c (contact force basis)", 5, RB_NP},
    {"p_c^(l+1) (estimated contact point)", 4, RB_NP},
    {"delta p_c (difference between fixed contact point)", 4, RB_NP},
    {"eps_c ('delta p_c' epsilon)", 1, RB_NP},
    {"mu*c_n", 1, RB_NP},
    {"delta chi_ref (reference deviation)", nd, 1},
    {"eps_delta_chi_ref (delta chi_ref epsilon)", 1, 1},
    {"f_T (fiber tension)", nd, RB_NF},
    {"p_A^(l+1) (estimated joint anchor point)", 4, RB_NJ},
    {"eps_rot (rotparm epsilon ??)", 1, 1},
    {"delta chi_prv (difference between the prev state)", nd, 1},
    {"eps_delta_chi_prv ('delta chi_prv' epsilon)", 1, 1},
    {"eps_p_z (contact point Z epsilon)", 1, RB_NP},
    {"kappa (normal force compensation)", 1, RB_NP},
    {"f_g (generalized gravitational force)", nd, 1},
    {"p_c^(l+1)_all (estimated corner point)", 4, MAX_CONTACTS},
    {"eps_pall_z (corner point Z epsilon)", 1, MAX_CONTACTS},
    {"f_freecom", nd, 1},
    {"eps_freecom", 1, 1},
  };
  BOOST_STATIC_ASSERT( 1+sizeof(A_block_slices)/sizeof(A_block_slices[0]) == sizeof(pymOpt.sd[0].Aci)/sizeof(pymOpt.sd[0].Aci[0]) );

  const double *const xx = pymOpt.xx;
  const int Aci_last_idx = sizeof(pymOpt.bod->Aci)/sizeof(pymOpt.bod->Aci[0]); //  1 + Aci size
  const int Aici_last_idx = sizeof(pymOpt.sd[0].Aci)/sizeof(pymOpt.sd[0].Aci[0]); // 1 + Aici size
  const int *const Aci = pymOpt.bod->Aci;
  const int xx_size = Aci [ Aci_last_idx-1 ];
  int tau_ofs = 0;
  const int nb = pymCfg->nBody;
  const int nf = pymCfg->nFiber;
  const int nj = pymCfg->nJoint;

  std::ofstream solvec_out("solvec.txt");
  assert(solvec_out.is_open());

  // A matrix part
  for (int i = 0; i < nb; ++i) {
    solvec_out << stream_indent(0) << A_block_slices[0].name << std::endl;
    solvec_out << stream_indent(1) << "[" << pymCfg->body[i].b.name << "]" << std::endl;
    int slice_ofs = 0;
    for (int j = 0; j < pymOpt.sd[i].Asubcols; ++j) {
      solvec_out << stream_indent(2) << A_block_slices[j].name << std::endl;

      const int sls = pym_calculate_slice_size(pymOpt, i, A_block_slices[j]);
      for (int k = 0; k < sls; ++k) {
        if (k && (k % A_block_slices[j].unit_size) == 0) {
          solvec_out << stream_indent(3) << "----" << std::endl;
        }
        const int ofs = tau_ofs + slice_ofs + k;
        solvec_out << stream_indent(3) << ofs << " : " << xx[ofs] << " (cost=" << pymOpt.c[ofs] << ")" << std::endl;
      }
      slice_ofs += sls;
    }
    tau_ofs += pymOpt.sd[i].Aci[ pymOpt.sd[i].Asubcols ];
  }
  int slice_ofs = 0;
  for (int i = 1; i < Aci_last_idx-1; ++i) {
    solvec_out << stream_indent(0) << block_slices[i].name << std::endl;
    const int sls = pym_calculate_slice_size(pymOpt, -1, block_slices[i]);
    for (int j = 0; j < sls; ++j) {
      if (j && (j % block_slices[i].unit_size) == 0) {
        solvec_out << stream_indent(1) << "----" << std::endl;
      }
      const int ofs = tau_ofs + slice_ofs + j;
      solvec_out << stream_indent(1) << ofs << " : " << xx[ofs] << " (cost=" << pymOpt.c[ofs] << ")" << std::endl;
    }
    slice_ofs += sls;
  }

  //for (int i = 0; i < xx_size; ++i) {
  //  for (int j = 0; j < Aci_last_idx-1; ++j) {
  //    if (Aci[j] == i) {
  //      indent = 0;
  //      solvec_out << stream_indent(indent) << block_slices[j].name << std::endl;
  //      ++indent;
  //    }
  //  }

  //  if (i < Aci[1]) {

  //    for (int l = 0; l < Aici_last_idx-1; ++l) {
  //      if (tau_ofs + pymOpt.sd[body_idx].Aci[l] == i) {
  //        if (l)
  //          --indent;
  //        if (l == 0)
  //          solvec_out << stream_indent(indent) << "[" << pymCfg->body[body_idx].b.name << "]" << std::endl;
  //        solvec_out << stream_indent(indent) << A_block_slices[l].name << std::endl;
  //        vec_ofs = 0;
  //        cur_A_block_idx = l;
  //        ++indent;
  //        break;
  //      }
  //    }

  //    /*if (tau_ofs == i) {
  //      tau_ofs += pymOpt.sd[k].Aci[ pymOpt.sd[k].Asubcols ];
  //      ++body_idx;
  //    }*/
  //  }
  //  
  //  
  //  if (i < Aci[1] && vec_ofs % A_block_slices[ cur_A_block_idx ].unit_size == 0 && vec_ofs) {
  //    solvec_out << stream_indent(indent) << "------" << std::endl;
  //  }

  //  if (cur_A_block_idx+2 == Aici_last_idx) {
  //    tau_ofs += vec_ofs;
  //    ++body_idx;
  //  }

  //  solvec_out << stream_indent(indent) << i << " : " << xx[i] << " (cost=" << pymOpt.c[i] << ")" << std::endl;
  //  ++vec_ofs;
  //}

  std::cout << "solvec.txt file written." << std::endl;
}
