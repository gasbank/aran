#ifndef __TRAJPARSER_H_
#define __TRAJPARSER_H_

#define MAX_CORRESMAP (50)

struct pym_traj_t {
  int exportFps;
  int nBlenderFrame;
  int nBlenderBody;
  int corresMapIndex[MAX_CORRESMAP]; //[pymCfg->nBody];
  char corresMap[MAX_CORRESMAP][2][128];
  int nCorresMap;
  double *trajData;
};

struct pym_config_t;

PYMPARSER_API int PymParseTrajectoryFile(pym_traj_t *pymTraj,
                           const char *fnRbCfg,
                           const char *fnTraj);

PYMPARSER_API int PymCorresMapIndexFromCorresMap(pym_traj_t *pymTraj,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT]);

PYMPARSER_API int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
				      pym_traj_t *pymTraj);
PYMPARSER_API int PymSetInitialStateUsingSimconf(pym_config_t *pymCfg);
#endif
