/*
 * TrajParser.c : Trajectory parser
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "PymStruct.h"
#include "Config.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
//#include "PymJointAnchor.h"
//#include "ConvexHullCapi.h"
#include "PymConfig.h"
#include "PymRbPod.h"

#define MAX_LINE_LEN (4096)


int PymParseTrajectoryFile(pym_traj_t *pymTraj,
                           const char *fnRbCfg,
                           const char *fnTraj) {
  FILE *rbCfg = fopen(fnRbCfg, "r");
  int i, j, k;
  int nCorresMap = 0;
  const char delimiters[] = " \n";
  FILE *traj = 0;
  int lineLen = 0;
  char *endp;
  int nFrame = 0;
  int nBody = 0;
  int exportFps = 0;
  double *trajData = 0;
  char trajHeader[MAX_LINE_LEN];
  size_t trajHeaderLen = 0;
  printf("Opening trajconf %s...\n", fnRbCfg);
  if (rbCfg == 0) {
    printf("Error - %s opening failure.\n", fnRbCfg);
    return -1;
  }


  while (1) {
    char aLine[MAX_LINE_LEN];
    size_t aLineLen = 0;
	char *ret = fgets(aLine, MAX_LINE_LEN, rbCfg);
    char *cp = 0;
	char *bodyName = 0;
	char *unusedToken = 0;
	char *anotherName = 0;
	if (!ret)
      break;
    //printf("%s", aLine);
    cp = strdup(aLine);
    bodyName = strtok(cp, delimiters);
    FOR_0(i, 6) unusedToken = strtok (0, delimiters);
    anotherName = strtok (0, delimiters);

    //printf("%s ... %s\n", bodyName, anotherName);
    strcpy(pymTraj->corresMap[nCorresMap][0], bodyName);
    strcpy(pymTraj->corresMap[nCorresMap][1], anotherName);
    ++nCorresMap;
    assert(nCorresMap <= MAX_CORRESMAP);
    free(cp);
  }
  printf("nCorresMap = %d\n", nCorresMap);

  /* Parse the trajectory file */
  traj  = fopen(fnTraj, "r");
  printf("Opening trajdata %s...\n", fnTraj);
  if (traj == 0) {
    printf("Error - Trajectory file %s opening failure.\n", fnTraj);
    return -2;
  }
  fgets(trajHeader, MAX_LINE_LEN, traj);
  assert(strlen(trajHeader) > 0);
  nFrame = strtol( trajHeader, &endp, 10 );
  if (!endp || nFrame <= 0) {
    printf("Error - trajectory data header's 'nFrame' field corrupted.\n");
    return -1;
  }
  nBody  = strtol( endp, &endp, 10 );
  if (!endp || nBody <= 0) {
    printf("Error - trajectory data header's 'nBody' field corrupted.\n");
    return -2;
  }
  exportFps = strtol( endp, &endp, 10 );
  if (!endp || exportFps <= 0) {
    printf("Error - trajectory data header's 'exportFps' field corrupted.\n");
    return -3;
  }
  printf("Trajectory header information exported from Blender\n");
  printf("  # of frames exported       : %d\n", nFrame);
  printf("  # of rigid bodies exported : %d\n", nBody);
  printf("  Frames per second (FPS)    : %d (%lf sec per frame)\n",
	 exportFps, 1.0/exportFps);
  assert(nFrame > 0 && nBody > 0);
  assert(nBody == nCorresMap);
  trajData = (double *)malloc(nFrame*nBody*sizeof(double)*6);
  FOR_0(i, nFrame) {
    FOR_0(j, nBody) {
      char aLine[MAX_LINE_LEN];
      size_t aLineLen = 0;
      char *cp = 0;
	  char *qexp[6];
	  fgets(aLine, MAX_LINE_LEN, traj);
	  aLineLen = strlen(aLine);
	  assert(aLineLen > 0);
      cp = strdup(aLine);
      qexp[0] = strtok(cp, delimiters);
      qexp[1] = strtok(0, delimiters);
      qexp[2] = strtok(0, delimiters);
      qexp[3] = strtok(0, delimiters);
      qexp[4] = strtok(0, delimiters);
      qexp[5] = strtok(0, delimiters);

      FOR_0(k, 6) {
		double parsed = strtod(qexp[k], 0);
		trajData[ i*nBody*6 + j*6 + k ] = parsed;
		//printf("  %e", parsed);
      }
      //printtf("\n");

      free(cp);
    }
  }

  fclose(rbCfg);
  fclose(traj);
  pymTraj->trajData        = trajData;
  pymTraj->nBlenderBody    = nBody;
  pymTraj->nBlenderFrame   = nFrame;
  pymTraj->exportFps       = exportFps;
  pymTraj->nCorresMap      = nCorresMap;
  return 0;
}

int PymCorresMapIndexFromCorresMap(pym_traj_t *pymTraj,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT])
{
  int i, j;
  FILE *__dmstream = dmstreams[PDMTE_INIT_RB_CORRESPONDENCE_1];
  fprintf(__dmstream,
	  "Simulated body and trajectory body correspondence\n");
  FOR_0(i, pymCfg->nBody) {
    FOR_0(j, pymTraj->nBlenderBody) {
      if (strcmp(pymCfg->body[i].b.name, pymTraj->corresMap[j][1]) == 0) {
	fprintf(__dmstream,
		"%3d %15s -- %d\n", i, pymCfg->body[i].b.name, j);
	pymTraj->corresMapIndex[i] = j;
	break;
      }
    }
    assert(pymTraj->corresMapIndex[i] >= 0);
  }
  j = 0;
  __dmstream = dmstreams[PDMTE_INIT_RB_CORRESPONDENCE_2];
  FOR_0(i, pymTraj->nCorresMap) {
    fprintf(__dmstream,
	    "%20s <----> %-20s",
	    pymTraj->corresMap[i][0],
	    pymTraj->corresMap[i][1]);
    if (strcmp(pymTraj->corresMap[i][1], "*") != 0) {
      fprintf(__dmstream,
	      " (index=%d)\n", pymTraj->corresMapIndex[j]);
      ++j;
    } else {
      fprintf(__dmstream,
	      "\n");
    }
  }
  return 0;
}

PYMPARSER_API int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
                                      pym_traj_t *pymTraj)
{
  /* Set current and previous state according
   * to the initial frame of trajectory data. */
  const int prevFrameIdx = 0;
  const int curFrameIdx  = 1;
  int i, j;
  double totMass = 0;
  FOR_0(i, pymCfg->nBody) {
    pym_rb_named_t *rbn = &pymCfg->body[i].b;
    double *prev = 0;
	double *current = 0;
	printf("initial rbn name = %s rotParam = %d mass = %lf kg\n",
	   rbn->name, rbn->rotParam, rbn->m);
    totMass += rbn->m;
    assert(rbn->rotParam == RP_EXP);
    /* previous step */
    prev = pymTraj->trajData + prevFrameIdx*pymTraj->nBlenderBody*6
      + pymTraj->corresMapIndex[i]*6 + 0;
    memcpy(rbn->p0, prev + 0, sizeof(double)*3);
    memcpy(rbn->q0, prev + 3, sizeof(double)*3);
    /* current step */
    current = pymTraj->trajData + curFrameIdx*pymTraj->nBlenderBody*6
      + pymTraj->corresMapIndex[i]*6 + 0;
    memcpy(rbn->p, current + 0, sizeof(double)*3);
    memcpy(rbn->q, current + 3, sizeof(double)*3);
    /* update discrete velocity based on current and previous step */
    FOR_0(j, 3) {
      rbn->pd[j] = (rbn->p[j] - rbn->p0[j]) / pymCfg->h;
      rbn->qd[j] = (rbn->q[j] - rbn->q0[j]) / pymCfg->h;
    }
  }
  printf("Total mass = %lf\n", totMass);
  return 0;
}
