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

void PrintCmdLineHelp(int argc, char *argv[]) {
  printf("  Usage:\n");
  printf("    %s config_file <simulation conf file> [Options]\n",
	 strrchr(argv[0], PATH_SEPARATOR) + 1);
  printf("\n");
  printf("  Options:\n");
  printf("\n");
}

void pym_init_debug_msg_streams(FILE *dmstreams[]) {
  int dmflags[PDMTE_COUNT] = {0,};
  //dmflags[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT] = 1;
  //dmflags[PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT] = 1;
  //dmflags[PDMTE_FBYF_REF_COM_DEVIATION_REPORT] = 1;
  int i;
  FILE *devnull = fopen("/dev/null", "w");
  FOR_0(i, PDMTE_COUNT) {
    if (dmflags[i]) dmstreams[i] = stdout;
    else dmstreams[i] = devnull;
  }
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
    printf("Failed.\n");
    return -1;
  }
  PymConvertRotParamInPlace(pymCfg, RP_EXP);
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
    int parseRet = PymParseTrajectoryFile(pymTraj,
					  cmdopt->trajconf,
					  cmdopt->trajdata);
    assert(parseRet == 0);
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
    char fnJaCfg[128] = {0};
    PymInferJointAnchorConfFileName(fnJaCfg, cmdopt->trajconf);
    const size_t num_joint_anchor =
      sizeof(pymCfg->pymJa)/sizeof(pym_joint_anchor_t);
    pymCfg->na = PymParseJointAnchorFile(pymCfg->pymJa, num_joint_anchor,
					fnJaCfg);
    assert(pymCfg->na >= 0);
    printf("Info - # of joint anchors parsed = %d\n", pymCfg->na);
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
    PymInitJointAnchors(pymCfg, dmstreams);
    PymConstructAnchoredJointList(pymCfg);
  } else {
    /* No trajconf provided. */
    PymSetPymCfgChiRefToCurrentState(pymCfg);
  }

  if (cmdopt->frame >= 0) {
    /* Last frame set by user */
    pymCfg->nSimFrame = cmdopt->frame;
  } else if (cmdopt->trajconf) {
    pymCfg->nSimFrame = pymTraj->nBlenderFrame - 2;
  } else {
    pymCfg->nSimFrame = 100;
  }

  FILE *outputFile = fopen(cmdopt->output, "w");
  if (!outputFile) {
    printf("Error: Opening the output file %s failed.\n", cmdopt->output);
    return -3;
  }
  fprintf(outputFile, "%d %d\n", pymCfg->nSimFrame, pymCfg->nBody);
  *_outputFile = outputFile;
  /* Let's start the simulation happily :) */
  printf("Starting the tracking simulation...\n");
  return 0;
}

pym_physics_thread_context_t
pym_init_phy_thread_ctx(pym_config_t *pymCfg,
			pym_cmdline_options_t *cmdopt,
			pym_traj_t *pymTraj,
			FILE *outputFile,
			FILE *dmstreams[]) {
  pym_physics_thread_context_t phyCon;
  phyCon.pymCfg             = pymCfg;
phyCon.cmdopt             = cmdopt;
phyCon.pymTraj            = pymTraj;
phyCon.outputFile         = outputFile;
phyCon.dmstreams          = dmstreams;
phyCon.totalPureOptTime   = 0;
phyCon.stop               = 0;
phyCon.trunkExternalForce[0] = 0;
phyCon.trunkExternalForce[1] = 0;
phyCon.trunkExternalForce[2] = 0;
phyCon.renBody            = (pym_rb_t *)calloc(pymCfg->nBody,  sizeof(pym_rb_t));
phyCon.renFiber           = (pym_mf_t *)calloc(pymCfg->nFiber, sizeof(pym_mf_t));
phyCon.comZGraph          = PrsGraphNew("COM Z");
phyCon.comDevGraph        = PrsGraphNew("COM Z Dev.");
phyCon.actGraph		= PrsGraphNew("Act");
phyCon.ligGraph		= PrsGraphNew("Lig");
phyCon.sd			= (pym_rb_statedep_t *)malloc(sizeof(pym_rb_statedep_t) * pymCfg->nBody);
  return phyCon;
}

PYMRS PymRsInitContext(int argc, char *argv[]) {
  printf("Pymuscle realtime simulator      -- 2010 Geoyeob Kim\n");
  pym_rs_t *rs = (pym_rs_t *)malloc(sizeof(pym_rs_t));
  int ret = pym_init_global(argc, argv, &rs->pymCfg, &rs->pymTraj,
			    &rs->cmdopt, &rs->outputFile, rs->dmstreams);
  if (ret) {
    /* something goes wrong */
    return 0;
  }
  /* Initialize the physics thread context */
  rs->phyCon = pym_init_phy_thread_ctx(&rs->pymCfg, &rs->cmdopt,
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
  PrsGraphSetMaxY(rs->phyCon.comDevGraph, 0.1);
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
  return rs;
}

void PymRsDestroyContext(PYMRS rs) {
  printf("Accumulated pure MOSEK optimizer time : %lf s\n",
	 rs->phyCon.totalPureOptTime);
  fclose(rs->outputFile);
  printf("Output written to %s\n", rs->cmdopt.output);
  PymDestoryConfig(&rs->pymCfg);
  PrsGraphDelete(rs->phyCon.comZGraph);
  PrsGraphDelete(rs->phyCon.comDevGraph);
  PrsGraphDelete(rs->phyCon.actGraph);
  PrsGraphDelete(rs->phyCon.ligGraph);
  free(rs->pymTraj.trajData);
  free(rs->phyCon.sd);
  if (rs->cmdopt.freeTrajStrings) {
    free(rs->cmdopt.trajconf);
    free(rs->cmdopt.trajdata);
  }
  if (rs->cmdopt.freeOutputStrings) {
    free(rs->cmdopt.output);
  }
  free(rs);
}
