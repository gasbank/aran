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
#include "PymJointAnchor.h"
#include "PymConfig.h"
#include "PymRbPod.h"

int PymParseTrajectoryFile(char corresMap[MAX_CORRESMAP][2][128],
                           int *_nCorresMap,
                           double **_trajData,
                           int *_nBody,
                           int *_nFrame,
                           int *_exportFps,
                           const char *fnRbCfg,
                           const char *fnTraj) {
    FILE *rbCfg = fopen(fnRbCfg, "r");
    printf("Opening trajconf %s...\n", fnRbCfg);
    if (rbCfg == 0) {
        printf("Error - %s opening failure.\n", fnRbCfg);
        return -1;
    }

    int i, j, k;

    int nCorresMap = 0;
    const char delimiters[] = " \n";
    while (1) {
        char *aLine = 0;
        size_t aLineLen = 0;
        ssize_t ret = getline(&aLine, &aLineLen, rbCfg);
        if (ret < 0 || aLineLen <= 1)
            break;
        //printf("%s", aLine);
        char *cp = strdup(aLine);
        char *bodyName = strtok(cp, delimiters);
        char *unusedToken;
        FOR_0(i, 6) unusedToken = strtok (0, delimiters);
        char *anotherName = strtok (0, delimiters);

        //printf("%s ... %s\n", bodyName, anotherName);
        strcpy(corresMap[nCorresMap][0], bodyName);
        strcpy(corresMap[nCorresMap][1], anotherName);
        ++nCorresMap;
        assert(nCorresMap <= MAX_CORRESMAP);
        free(cp);
        free(aLine);
    }
    printf("nCorresMap = %d\n", nCorresMap);

    /* Parse the trajectory file */
    FILE *traj  = fopen(fnTraj, "r");
    printf("Opening trajdata %s...\n", fnTraj);
    if (traj == 0) {
        printf("Error - Trajectory file %s opening failure.\n", fnTraj);
        return -2;
    }
    char *trajHeader = 0;
    size_t trajHeaderLen = 0;
    int lineLen = getline(&trajHeader, &trajHeaderLen, traj);
    assert(lineLen > 0);
    char *endp;
    int nFrame = strtol( trajHeader, &endp, 10 );
    if (!endp || nFrame <= 0) {
    	printf("Error - trajectory data header's 'nFrame' field corrupted.\n");
    	return -1;
    }
    int nBody  = strtol( endp, &endp, 10 );
    if (!endp || nBody <= 0) {
    	printf("Error - trajectory data header's 'nBody' field corrupted.\n");
    	return -2;
    }
    int exportFps = strtol( endp, &endp, 10 );
    if (!endp || exportFps <= 0) {
    	printf("Error - trajectory data header's 'exportFps' field corrupted.\n");
    	return -3;
    }
    printf("Trajectory header information exported from Blender\n");
    printf("  # of frames exported       : %d\n", nFrame);
    printf("  # of rigid bodies exported : %d\n", nBody);
    printf("  Frames per second (FPS)    : %d (%lf sec per frame)\n", exportFps, 1.0/exportFps);
    assert(nFrame > 0 && nBody > 0);
    assert(nBody == nCorresMap);
    double *trajData = (double *)malloc(nFrame*nBody*sizeof(double)*6);
    FOR_0(i, nFrame) {
        FOR_0(j, nBody) {
            char *aLine = 0;
            size_t aLineLen = 0;
            ssize_t ret = getline(&aLine, &aLineLen, traj);
            assert(ret >= 0);

            char *cp = strdup(aLine);
            char *qexp[6];
            qexp[0] = strtok(cp, delimiters);
            qexp[1] = strtok(0, delimiters);
            qexp[2] = strtok(0, delimiters);
            qexp[3] = strtok(0, delimiters);
            qexp[4] = strtok(0, delimiters);
            qexp[5] = strtok(0, delimiters);

            FOR_0(k, 6) {
                const double parsed = strtod(qexp[k], 0);
                trajData[ i*nBody*6 + j*6 + k ] = parsed;
                //printf("  %e", parsed);
            }
            //printf("\n");

            free(cp);
            free(aLine);
        }
    }

    free(trajHeader);
    fclose(rbCfg);
    fclose(traj);
    *_trajData   = trajData;
    *_nBody      = nBody;
    *_nFrame     = nFrame;
    *_exportFps  = exportFps;
    *_nCorresMap = nCorresMap;
    return 0;
}

int PymCorresMapIndexFromCorresMap(int corresMapIndex[],
                                   int nCorresMap,
                                   char corresMap[nCorresMap][2][128],
                                   int nBlenderBody,
                                   pym_config_t *pymCfg,
                                   FILE *dmstreams[PDMTE_COUNT])
{
    int i, j;
    FILE *__dmstream = dmstreams[PDMTE_INIT_RB_CORRESPONDENCE_1];
    fprintf(__dmstream,
            "Simulated body and trajectory body correspondence\n");
    FOR_0(i, pymCfg->nBody) {
        FOR_0(j, nBlenderBody) {
            if (strcmp(pymCfg->body[i].b.name, corresMap[j][1]) == 0) {
                fprintf(__dmstream,
                        "%3d %15s -- %d\n", i, pymCfg->body[i].b.name, j);
                corresMapIndex[i] = j;
                break;
            }
        }
        assert(corresMapIndex[i] >= 0);
    }
    j = 0;
    __dmstream = dmstreams[PDMTE_INIT_RB_CORRESPONDENCE_2];
    FOR_0(i, nCorresMap) {
        fprintf(__dmstream,
                "%20s <----> %-20s", corresMap[i][0], corresMap[i][1]);
        if (strcmp(corresMap[i][1], "*") != 0) {
            fprintf(__dmstream,
                    " (index=%d)\n", corresMapIndex[j]);
            ++j;
        } else {
            fprintf(__dmstream,
                    "\n");
        }
    }
    return 0;
}

int PymSetInitialStateUsingTrajectory(pym_config_t *pymCfg,
                                      int nBlenderBody,
                                      const int corresMapIndex[],
                                      const double *trajData)
{
    /* Set current and previous state according
     * to the initial frame of trajectory data. */
    const int prevFrameIdx = 0;
    const int curFrameIdx  = 1;
    int i, j;
    FOR_0(i, pymCfg->nBody) {
        pym_rb_named_t *rbn = &pymCfg->body[i].b;
        assert(rbn->rotParam == RP_EXP);
        /* previous step */
        memcpy(rbn->p0, trajData + prevFrameIdx*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
        memcpy(rbn->q0, trajData + prevFrameIdx*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
        /* current step */
        memcpy(rbn->p,  trajData + curFrameIdx*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
        memcpy(rbn->q,  trajData + curFrameIdx*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
        /* update discrete velocity based on current and previous step */
        FOR_0(j, 3) {
            rbn->pd[j] = (rbn->p[j] - rbn->p0[j]) / pymCfg->h;
            rbn->qd[j] = (rbn->q[j] - rbn->q0[j]) / pymCfg->h;
        }
    }
    return 0;
}
