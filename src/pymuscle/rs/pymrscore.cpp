#include "PymRsPch.h"

#include "PrsGraphCapi.h"
#include "PymStruct.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PhysicsThreadMain.h"
#include "PymCmdLineParser.h"
#include "MathUtil.h"

#include "pymrscore.h"

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

static FILE *devnull;

FILE *PymGetDevnull() {
  return devnull;
}

void PrintCmdLineHelp(int argc, char *argv[]) {
  printf("  Usage:\n");
  printf("    %s config_file <simulation conf file> [Options]\n",
	 strrchr(argv[0], PATH_SEPARATOR) + 1);
  printf("\n");
  printf("  Options:\n");
  printf("    --test    simulator test mode (no simulation conf file needed)\n");
  printf("\n");
}

void pym_update_debug_msg_streams(int *dmflags, FILE *dmstreams[]) {
  int i;
#ifdef WIN32
  devnull = fopen("nul", "w");
#else
  devnull = fopen("/dev/null", "w");
#endif
  assert(devnull);
  FOR_0(i, PDMTE_COUNT) {
    if (dmflags[i]) dmstreams[i] = stdout;
    else dmstreams[i] = devnull;
  }
}

void pym_init_debug_msg_streams(FILE *dmstreams[]) {
  int dmflags[PDMTE_COUNT] = {0,};
  dmflags[PDMTE_INIT_MF_FOR_EACH_RB] = 1;
  dmflags[PDMTE_INIT_RB_CORRESPONDENCE_1] = 1;
  dmflags[PDMTE_INIT_RB_CORRESPONDENCE_2] = 1;
  dmflags[PDMTE_INIT_JOINT_ANCHOR_ATTACH_REPORT] = 1;

  pym_update_debug_msg_streams(dmflags, dmstreams);
}

int pym_init_global(int argc, char *argv[], pym_config_t *pymCfg,
		    pym_traj_t *pymTraj,
		    pym_cmdline_options_t *cmdopt,
		    FILE** _outputFile,
		    FILE *dmstreams[]) {
  int ret = 0;
  if ( argc < 2 || (argc == 2 && strcmp(argv[1], "--help") == 0) ) {
    PrintCmdLineHelp(argc, argv);
    return 1;
  }
  ret = PymParseCmdlineOptions(cmdopt, argc, argv);
  if (ret < 0) {
    printf("Failed.\n");
    return -2;
  }
  /* Initialize debug message flags (dmflags) */
  pym_init_debug_msg_streams(dmstreams);

  /* Construct pymCfg structure */
  ret = PymConstructConfig(cmdopt->simconf, pymCfg,
    dmstreams[PDMTE_INIT_MF_FOR_EACH_RB]);
  if (ret < 0) {
    printf("PymConstructConfig() Failed.\n");
    return -1;
  }
  
  PymConvertRotParamInPlace(pymCfg, pymCfg->rotparam);
  const char *t1 = strrchr(cmdopt->trajconf, '/') + 1;
  const char *t2 = strchr(t1, '.');
  size_t tlen = t2-t1;
  strncpy(pymCfg->trajName, t1, tlen);
  pymCfg->trajName[tlen] = 0;
  printf("trajName = %s\n", pymCfg->trajName);
  int i;
  FOR_0(i, pymCfg->nBody)
    pymTraj->corresMapIndex[i] = -1;
  if (cmdopt->trajconf) {
    char fnJaCfg[128] = {0};
    int parseRet = PymParseTrajectoryFile(pymTraj,
					  cmdopt->trajconf,
					  cmdopt->trajdata);
    if (parseRet) {
      /* from now on, we should not refer 'pymTraj' variable in this case. */
      printf("Warn - trajectory data file not exists.\n");
      /* No trajconf provided. */
      PymSetPymCfgChiRefToCurrentState(pymCfg);
    } else {
      assert(pymTraj->nCorresMap > 0);
      /* The simulation time step defined in simconf and
      * the frame time (reciprocal of FPS) in trajdata
      * should have the same value. If mismatch happens
      * we ignore simulation time step in simconf.
      */
      if (fabs(1.0/pymTraj->exportFps - pymCfg->h) > 1e-6)
      {
        printf("Warning - simulation time step defined in simconf and\n");
        printf("          trajectory data do not match.\n");
        printf("            simconf  : %3d FPS (%lf sec per frame)\n",
          (int)ceil(1.0/pymCfg->h), pymCfg->h);
        printf("            trajconf : %3d FPS (%lf sec per frame)\n",
          pymTraj->exportFps, 1.0/pymTraj->exportFps);
        printf("          simconf's value will be ignored.\n");
        pymCfg->h = 1.0/pymTraj->exportFps;
      }

      PymCorresMapIndexFromCorresMap(pymTraj,
        pymCfg,
        dmstreams);
      /*
      * We need at least three frames of trajectory data
      * to follow one or more reference trajectory frames since
      * the first (frame 0) is used as previous step and
      * the second (frame 1) is used as current step and
      * the third (frame 2) is used as the reference trajectory for next step
      *
      * We need (nBlenderFrame-2) simulation iteration to complete
      * following the trajectory entirely. So the following assertion helds:
      */
      assert(pymTraj->nBlenderFrame >= 3);
      PymSetInitialStateUsingTrajectory(pymCfg, pymTraj);
    }
    
    PymInferJointAnchorConfFileName(fnJaCfg, cmdopt->trajconf);
    const size_t num_joint_anchor =
      sizeof(pymCfg->pymJa)/sizeof(pym_joint_anchor_t);
    pymCfg->na = PymParseJointAnchorFile(pymCfg->pymJa, num_joint_anchor,
					fnJaCfg);
    if (pymCfg->na < 0) {
      /* Joint anchor conf file parse failure. Ignore joint anchors happily. */
      pymCfg->na = 0;
    }
    printf("Info - # of joint anchors = %d\n", pymCfg->na);
    PymInitJointAnchors(pymCfg, dmstreams);
    PymConstructAnchoredJointList(pymCfg);

    if (pymCfg->joint_constraints) {
      printf("Fibers for joint constraints (joint muscles) are created automatically.\n");
      for (int i = 0; i < pymCfg->nJoint; ++i) {
        const int ai = pymCfg->anchoredJoints[i].aIdx;
        const int bi = pymCfg->anchoredJoints[i].bIdx;
        pym_rb_named_t *rbA = &pymCfg->body[ai].b;
        pym_rb_named_t *rbB = &pymCfg->body[bi].b;
        double *a_apos = pymCfg->body[ai].b.jointAnchors[ pymCfg->anchoredJoints[i].aAnchorIdx ];
        double *b_apos = pymCfg->body[bi].b.jointAnchors[ pymCfg->anchoredJoints[i].bAnchorIdx ];

        pym_mf_named_t *mf = &pymCfg->fiber [ pymCfg->nFiber ].b;
        mf->A = 0;
        mf->b = 1;
        memcpy(mf->fibb_org, a_apos, sizeof(double)*3);
        memcpy(mf->fibb_ins, b_apos, sizeof(double)*3);
        mf->org = ai;
        mf->ins = bi;
        mf->kpe = 1;
        mf->kse = 1;
        mf->mType = PMT_JOINT_MUSCLE;
        mf->T = 0;
        mf->xrest = 1.0;
        mf->xrest_lower = 0.0;
        mf->xrest_upper = 10.0;
        strcpy(mf->name, "joint muscle fiber");

        rbA->fiber[ rbA->nFiber ] = pymCfg->nFiber;
        rbB->fiber[ rbB->nFiber ] = pymCfg->nFiber;
        ++rbA->nFiber;
        ++rbB->nFiber;
        ++pymCfg->nFiber;
      }
    }
  } else {
    /* No trajconf provided. */
    PymSetPymCfgChiRefToCurrentState(pymCfg);
  }
  pymCfg->real_muscle = false;
  if (cmdopt->frame >= 0) {
    /* Total simulation frame set by user */
    pymCfg->nSimFrame = cmdopt->frame;
  } else if (cmdopt->trajconf && pymTraj->trajData) {
    pymCfg->nSimFrame = pymTraj->nBlenderFrame - 2;
  } else {
    /* if no traj conf data provided */
    pymCfg->nSimFrame = 100;
  }
  assert(pymCfg->nSimFrame >= 1);
  FILE *outputFile = fopen(cmdopt->output, "w");
  if (!outputFile) {
    printf("Error: Opening the output file %s failed.\n", cmdopt->output);
    return -3;
  }
  fprintf(outputFile, "%d %d\n", pymCfg->nSimFrame, pymCfg->nBody);
  *_outputFile = outputFile;
  /* Let's start the simulation happily :) */
  printf("Starting the simulation...\n");
  return 0;
}


void pym_init_phy_thread_ctx(pym_physics_thread_context_t* cxt,
  pym_config_t *pymCfg,
			pym_cmdline_options_t *cmdopt,
			pym_traj_t *pymTraj,
			FILE *outputFile,
			FILE *dmstreams[]) {
  pym_physics_thread_context_t phyCon;
  cxt->pymCfg             = pymCfg;
  cxt->cmdopt             = cmdopt;
  cxt->pymTraj            = pymTraj;
  cxt->outputFile         = outputFile;
  cxt->dmstreams          = dmstreams;
  cxt->totalPureOptTime   = 0;
  cxt->stop               = 0;
  cxt->trunkExternalForce[0] = 0;
  cxt->trunkExternalForce[1] = 0;
  cxt->trunkExternalForce[2] = 0;
  cxt->comZGraph          = PrsGraphNew("COM Z");
  cxt->comDevGraph        = PrsGraphNew("COM Z Dev.");
  cxt->exprotGraph.resize(pymCfg->nBody);
  for (int i = 0; i < pymCfg->nBody; ++i)
    cxt->exprotGraph[i]   = PrsGraphNew("Body Rot");
  cxt->actGraph		= PrsGraphNew("Act");
  cxt->ligGraph		= PrsGraphNew("Lig");
  cxt->sd			= (pym_rb_statedep_t *)malloc(sizeof(pym_rb_statedep_t) * pymCfg->nBody);
  for (int i = 0; i < pymCfg->nBody; ++i)
    pym_init_statedep(cxt->sd[i]);
}

pym_rs_t *PymRsInitContext(int argc, char *argv[]) {
  printf("Pymuscle realtime simulator      -- 2010 Geoyeob Kim\n");
  pym_rs_t *rs = new pym_rs_t;
  int ret = pym_init_global(argc, argv, &rs->pymCfg, &rs->pymTraj,
			    &rs->cmdopt, &rs->outputFile, rs->dmstreams);
  if (ret) {
    /* something goes wrong */
    return 0;
  }
  rs->drawing_options = 0;
  /* Initialize the physics thread context */
  pym_init_phy_thread_ctx(&rs->phyCon, &rs->pymCfg, &rs->cmdopt,
				       &rs->pymTraj,
				       rs->outputFile, rs->dmstreams);
  /* comZGraph */
  PrsGraphSetMaxY(rs->phyCon.comZGraph, 4.0);
  PRSGRAPHDATA simComGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(simComGd, 0, 1, 0);
  PrsGraphAttach(rs->phyCon.comZGraph, PCG_SIM_COMZ, simComGd);
  PRSGRAPHDATA refComGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(refComGd, 1, 0, 0);
  PrsGraphAttach(rs->phyCon.comZGraph, PCG_REF_COMZ, refComGd);
  /* comDevGraph */
  PrsGraphSetMaxY(rs->phyCon.comDevGraph, 2.0);
  PRSGRAPHDATA comDevGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(comDevGd, 0, 1, 0);
  PrsGraphAttach(rs->phyCon.comDevGraph, PCG_COMDEV, comDevGd);
  PrsGraphAddGuideY(rs->phyCon.comDevGraph, 0.01, 1, 1, 0);
  /* actGraph */
  PrsGraphSetMaxY(rs->phyCon.actGraph, 10000);
  PRSGRAPHDATA actActGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(actActGd, 1, 0, 0);
  PrsGraphAttach(rs->phyCon.actGraph, PCG_ACT_ACT, actActGd);
  PRSGRAPHDATA actTenGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(actTenGd, 0, 1, 0);
  PrsGraphAttach(rs->phyCon.actGraph, PCG_ACT_TEN, actTenGd);
  /* ligGraph */
  PrsGraphSetMaxY(rs->phyCon.ligGraph, 10000);
  PRSGRAPHDATA ligActGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(ligActGd, 1, 0, 0);
  PrsGraphAttach(rs->phyCon.ligGraph, PCG_LIG_ACT, ligActGd);
  PRSGRAPHDATA ligTenGd = PrsGraphDataNew(rs->pymCfg.nSimFrame);
  PrsGraphDataSetLineColor(ligTenGd, 0, 1, 0);
  PrsGraphAttach(rs->phyCon.ligGraph, PCG_LIG_TEN, ligTenGd);
  /* exprotGraph */
  for (std::vector<PRSGRAPH>::iterator i = rs->phyCon.exprotGraph.begin(); i != rs->phyCon.exprotGraph.end(); ++i) {
    PrsGraphSetMaxY(*i, 2*M_PI);
    PRSGRAPHDATA d = PrsGraphDataNew(rs->pymCfg.nSimFrame);
    PrsGraphDataSetLineColor(d, 1, 0, 0);
    PrsGraphAttach(*i, 0, d);
  }
  
  return rs;
}

void PymRsDestroyContext(pym_rs_t *rs) {
  printf("Accumulated pure MOSEK optimizer time : %lf s\n",
    rs->phyCon.totalPureOptTime);
  fclose(rs->outputFile);
  printf("Output written to %s\n", rs->cmdopt.output);
  PymDestoryConfig(&rs->pymCfg);
  PrsGraphDelete(rs->phyCon.comZGraph);
  PrsGraphDelete(rs->phyCon.comDevGraph);
  PrsGraphDelete(rs->phyCon.actGraph);
  PrsGraphDelete(rs->phyCon.ligGraph);
  for (std::vector<PRSGRAPH>::iterator i = rs->phyCon.exprotGraph.begin(); i != rs->phyCon.exprotGraph.end(); ++i) {
    PrsGraphDelete(*i);
  }
  free(rs->pymTraj.trajData);
  free(rs->phyCon.sd);
  if (rs->cmdopt.freeTrajStrings) {
    free(rs->cmdopt.trajconf);
    free(rs->cmdopt.trajdata);
  }
  if (rs->cmdopt.freeOutputStrings) {
    free(rs->cmdopt.output);
  }
  delete rs;
}
