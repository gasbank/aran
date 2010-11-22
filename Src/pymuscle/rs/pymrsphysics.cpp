/*
 * pymrsphysics.c: Dedicated physics simulation thread
 *                      for realtime simulator
 * 2010 Geoyeob Kim
 */
#include "PymRsPch.h"
#include "PrsGraphCapi.h"
#include "PymStruct.h"
#include "ConvexHullCapi.h"
#include "MathUtil.h"
#include "PymConfig.h"
#include "PymCmdLineParser.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "PymCmdLineParser.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PhysicsThreadMain.h"
#include "Optimize.h"
#include "DebugPrintDef.h"

#include "pymrscore.h"
#include "pymrsphysics.h"

int PymRsInitPhysics(pym_rs_t *rs) {
  pym_physics_thread_context_t	*phyCon = &rs->phyCon;
  pym_config_t			*pymCfg = phyCon->pymCfg;
  pym_cmdline_options_t		*cmdopt = phyCon->cmdopt;

  const int             nBlenderBody   = phyCon->pymTraj->nBlenderBody;
  const int *const      corresMapIndex = phyCon->pymTraj->corresMapIndex;
  const double *const   trajData       = phyCon->pymTraj->trajData;

  /* TODO: Output file of realtime simulator */
  FILE *outputFile = 0;
  FILE **dmstreams = phyCon->dmstreams;

  /* Initilize CHOLMOD library */
  cholmod_start (&rs->cc) ;
  /* Initialize MOSEK instance */
  PymInitializeMosek(&rs->env);
  return 0;
}

int PymRsDestroyPhysics(pym_rs_t *rs) {
  PymCleanupMosek(&rs->env);
  cholmod_finish(&rs->cc);
  return 0;
}

int PymRsResetPhysics(pym_rs_t *rs) {
  pym_physics_thread_context_t	*phyCon	= &rs->phyCon;
  pym_config_t			*pymCfg	= phyCon->pymCfg;
  //  pthread_mutex_lock(&main_mutex);
  printf("Resetting the physics states...\n");
  if (phyCon->pymTraj->trajData)
    PymSetInitialStateUsingTrajectory(pymCfg, phyCon->pymTraj);
  else
    PymSetInitialStateUsingSimconf(pymCfg);
  //  pthread_mutex_unlock(&main_mutex);
  pymCfg->prevTotContacts = 0;
  pymCfg->curTotContacts = 0;
  int j;
  FOR_0(j, pymCfg->nFiber)
    pymCfg->fiber[j].b.T = 0;
  return 0;
}

int PymRsFrameMove(pym_rs_t *rs, int fidx, char *result_msg) {
  /* Function argument 'i' is the current frame. */
  pym_physics_thread_context_t	*phyCon		= &rs->phyCon;
  pym_config_t			*pymCfg		= phyCon->pymCfg;
  pym_cmdline_options_t		*cmdopt		= phyCon->cmdopt;
  const int			 nBlenderBody   = phyCon->pymTraj->nBlenderBody;
  const int *const		 corresMapIndex = phyCon->pymTraj->corresMapIndex;
  const double *const		 trajData       = phyCon->pymTraj->trajData;
  /* TODO: Output file of realtime simulator */
  FILE *outputFile = 0;
  FILE **dmstreams = phyCon->dmstreams;
  int j, k;
  /* Get trunk RB address for later external force testing */
  pym_rb_named_t *rbnTrunk = 0;
  FOR_0(j, pymCfg->nBody) {
    pym_rb_named_t *rbn = &pymCfg->body[j].b;
    if (strcmp(rbn->name, "trunk") == 0) {
      rbnTrunk = rbn;
    }
  }
  assert(pymCfg->nSimFrame >= 3);
  //assert(i < pymCfg->nSimFrame);
  //    printf("%5d / %5d\n", i, pymCfg->nSimFrame-1);
  /*
   * A very long and time-consuming simulation task goes here ...
   */
  int ret = PymCheckRotParam(pymCfg);
  if (ret < 0) {
    printf("Error - Rotation parameterization failure detected.\n");
    return -2;
  }

  /* Set reference */
  if (cmdopt->trajconf && trajData) {
    FOR_0(j, pymCfg->nBody) {
      const double *const ref =
        trajData + (fidx+2)*nBlenderBody*6 + corresMapIndex[j]*6;
      //printf("pymCfg address : %p\n", pymCfg);
      //printf("%p -- ", &pymCfg->body[j].b.chi_ref[0]);
      for (k = 0; k < 6; ++k) {
        pymCfg->body[j].b.chi_ref[k] = ref[k];
	      //printf("%e ", ref[k]);
      }
      //printf("\n");
    }
  }
  /* Compute current step simulated biped COM */
  /* Compute next step reference COM */
  double curSimCom[3] = {0,};
  double refCom[3] = {0,};
  double totMass = 0;
  FOR_0(j, pymCfg->nBody)
    totMass += pymCfg->body[j].b.m;
  FOR_0(j, pymCfg->nBody) {
    const double massj = pymCfg->body[j].b.m;
    FOR_0(k, 3) {
      curSimCom[k] += massj * pymCfg->body[j].b.p[k];
      refCom[k]    += massj * pymCfg->body[j].b.chi_ref[k];
    }
  }
  FOR_0(j, 3) {
    curSimCom[j] /= totMass;
    refCom[j] /= totMass;
  }
  memcpy(pymCfg->curBipCom, curSimCom, sizeof(double)*3);
  /* Tune reference to have no significant COM deviation between
   * simulated result. */
  //        const double comdev = PymDist(3, pymCfg->bipCom, refCom);
  //        if (cmdopt->trajconf && i != 0) {
  //            if (comdev > 0.25) {
  //                double comdiff[3];
  //                FOR_0(k, 3) {
  //                    comdiff[k] = pymCfg->bipCom[k] - refCom[k];
  //                    printf("comdiff[%d] = %lf\n", k, comdiff[k]);
  //                }
  //
  //                FOR_0(j, pymCfg->nBody) {
  //                    for (k = 0; k < 3; ++k) {
  //                        pymCfg->body[j].b.chi_ref[k] += comdiff[k];
  //                    }
  //                }
  //            }
  //        }

  double pureOptTime = 0;
  const char *solstaStr;
  double cost = 0;
  ret = PymOptimizeFrameMove(&pureOptTime, outputFile, pymCfg, phyCon->sd,
			     dmstreams,
			     &solstaStr, &cost, &rs->cc, rs->env);
  
  if (ret) {
    printf("Error - Something goes wrong during optimization frame move.\n");
    return -1;
  }
  const int nf = pymCfg->nFiber;
  double totActAct = 0, totActTen = 0;
  double totLigAct = 0, totLigTen = 0;
  FOR_0(j, nf) {
    pym_mf_named_t *mf = &pymCfg->fiber[j].b;
    if (mf->mType == PMT_ACTUATED_MUSCLE) {
      totActAct += mf->A;
      totActTen += fabs(mf->T);
    } else {
      totLigAct += mf->A;
      totLigTen += fabs(mf->T);
    }
  }

  PrsGraphPushBackTo(phyCon->comZGraph, PCG_SIM_COMZ, pymCfg->bipCom[2]);
  PrsGraphPushBackTo(phyCon->comZGraph, PCG_REF_COMZ, refCom[2]);
  const double comzdev = fabs(pymCfg->bipCom[2] - refCom[2]);
  PrsGraphPushBackTo(phyCon->comDevGraph, PCG_COMDEV, comzdev);

  PrsGraphPushBackTo(phyCon->actGraph, PCG_ACT_ACT, fabs(totActAct));
  PrsGraphPushBackTo(phyCon->actGraph, PCG_ACT_TEN, totActTen);
  PrsGraphPushBackTo(phyCon->ligGraph, PCG_LIG_ACT, totActTen - fabs(totActAct));
  //PrsGraphPushBackTo(phyCon->ligGraph, PCG_LIG_ACT, fabs(totLigAct));
  //PrsGraphPushBackTo(phyCon->ligGraph, PCG_LIG_TEN, totLigTen);

  /* Simulated biped COM position */
  memcpy(phyCon->bipCom,    pymCfg->bipCom,    sizeof(double)*3);
  /* Reference biped COM position (note that pymCfg, not phyCon!) */
  memcpy(pymCfg->bipRefCom, refCom, sizeof(double)*3);

  /*memcpy(pymCfg->renChInput, pymCfg->chInput,  sizeof(pymCfg->chInput));
  pymCfg->renChInputLen = pymCfg->chInputLen;
  memcpy(pymCfg->renChOutput, pymCfg->chOutput,  sizeof(pymCfg->chOutput));
  pymCfg->renChOutputLen = pymCfg->chOutputLen;*/

  /* Write external force exertion */
  if (rbnTrunk) {
    memcpy(rbnTrunk->extForce, phyCon->trunkExternalForce,
      sizeof(double)*3);
    rbnTrunk->extForcePos[2] = 0.2;
  }
  memset(phyCon->trunkExternalForce, 0, sizeof(double)*3);

  const double percent = (double)(fidx+1)/(pymCfg->nSimFrame)*100;
  char step_result_summary[256];
  sprintf(step_result_summary, "%5d / %5d  (%6.2lf %%) result - %12s, cost = %.6e",
    fidx, pymCfg->nSimFrame-1, percent, solstaStr, cost);
  fprintf(dmstreams[PDMTE_FBYF_STEP_RESULT_SUMMARY], "%s\n", step_result_summary);
  strcat(result_msg, step_result_summary);
  phyCon->totalPureOptTime += pureOptTime;
  return 0;
}
