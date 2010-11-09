#ifndef __TRAJPARSER_H_
#define __TRAJPARSER_H_

#define MAX_CORRESMAP (50)

typedef struct aaa_pym_traj_t {
  int exportFps;
  int nBlenderFrame;
  int nBlenderBody;
  int corresMapIndex[MAX_CORRESMAP]; //[pymCfg->nBody];
  char corresMap[MAX_CORRESMAP][2][128];
  int nCorresMap;
  double *trajData;
} pym_traj_t;

PYMPARSER_API int PymParseTrajectoryFile(pym_traj_t *pymTraj,
                           const char *fnRbCfg,
                           const char *fnTraj);

PYMPARSER_API int PymCorresMapIndexFromCorresMap(pym_traj_t *pymTraj,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT]);

PYMPARSER_API int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
				      pym_traj_t *pymTraj);
#endif
