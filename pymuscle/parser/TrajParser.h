#ifndef __TRAJPARSER_H_
#define __TRAJPARSER_H_

#define MAX_CORRESMAP (50)

#if defined(_USRDLL) && defined(PYMPARSER_EXPORTS)
#define PYMPARSER_API			PYMPARSER_API_EXPORT
#define PYMPARSER_API_EXTERN
#else
#define PYMPARSER_API			PYMPARSER_API_IMPORT
#define PYMPARSER_API_EXTERN		extern
#endif

#ifdef WIN32
#define PYMPARSER_API_EXPORT __declspec(dllexport)
#define PYMPARSER_API_IMPORT __declspec(dllimport)
#else
#define PYMPARSER_API_EXPORT
#define PYMPARSER_API_IMPORT
#endif

typedef struct aaa_pym_traj_t {
  int exportFps;
  int nBlenderFrame;
  int nBlenderBody;
  int corresMapIndex[MAX_CORRESMAP]; //[pymCfg->nBody];
  char corresMap[MAX_CORRESMAP][2][128];
  int nCorresMap;
  double *trajData;
} pym_traj_t;

int PymParseTrajectoryFile(pym_traj_t *pymTraj,
                           const char *fnRbCfg,
                           const char *fnTraj);

int PymCorresMapIndexFromCorresMap(pym_traj_t *pymTraj,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT]);

PYMPARSER_API int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
				      pym_traj_t *pymTraj);
#endif
