#ifndef __TRAJPARSER_H_
#define __TRAJPARSER_H_

#ifdef WIN32
#define MAX_CORRESMAP (50)
#else
static const int MAX_CORRESMAP = 50;
#endif

int PymParseTrajectoryFile(char corresMap[MAX_CORRESMAP][2][128],
                           int *_nCorresMap,
                           double **_trajData,
                           int *_nBody,
                           int *_nFrame,
                           int *_exportFps,
                           const char *fnRbCfg,
                           const char *fnTraj);

int PymCorresMapIndexFromCorresMap(int corresMapIndex[],
                                   int nCorresMap,
                                   char corresMap[nCorresMap][2][128],
                                   int nBlenderBody,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT]);

int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
                                      int nBlenderBody,
                                      const int corresMapIndex[],
                                      const double *trajData);
#endif
