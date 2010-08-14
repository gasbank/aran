/*
 * PhysicsThreadMain.c: Dedicated physics simulation thread
 *                      for realtime simulator
 * 2010 Geoyeob Kim
 */
#include "PymPch.h"
#include <pthread.h>
#include "PymStruct.h"
#include "PymConfig.h"
#include "PymCmdLineParser.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "PymCmdLineParser.h"
#include "PhysicsThreadMain.h"
#include "Optimize.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
extern pthread_mutex_t count_mutex;
extern pthread_mutex_t main_mutex;
extern pthread_cond_t count_threshold_cv;
extern pthread_cond_t physics_thread_finished;

void *PhysicsThreadMain(void *t)
{
    pym_physics_thread_context_t *cxt    = (pym_physics_thread_context_t *)t;
    pym_config_t                 *pymCfg = cxt->pymCfg;
    pym_cmdline_options_t        *cmdopt = cxt->cmdopt;
    const int             nBlenderBody   = cxt->nBlenderBody;
    const int *const      corresMapIndex = cxt->corresMapIndex;
    const double *const   trajData       = cxt->trajData;
    FILE *outputFile = 0; /* TODO: Output file of realtime simulator */
    FILE **dmstreams = cxt->dmstreams;

    /* Initilize CHOLMOD library */
    cholmod_common cc ;
    cholmod_start (&cc) ;
    /* Initialize MOSEK instance */
    MSKenv_t    env;
    PymInitializeMosek(&env);

    int i = 0, j;
    /* Get trunk RB address for later external force testing */
    pym_rb_named_t *rbnTrunk = 0;
    FOR_0(j, pymCfg->nBody) {
        pym_rb_named_t *rbn = &pymCfg->body[j].b;
        if (strcmp(rbn->name, "trunk") == 0) {
            rbnTrunk = rbn;
        }
    }
    assert(rbnTrunk);
    int resetFlag = 0;
    while (1) {
        if ((i >= pymCfg->nSimFrame) || resetFlag) {
            PymSetInitialStateUsingTrajectory(pymCfg, nBlenderBody,
                                              corresMapIndex, trajData);
            i = 0;
            resetFlag = 0;
        }
        /*
         * A very long and time-consuming simulation task goes here ...
         */
        int ret = PymCheckRotParam(pymCfg);
        if (ret < 0) {
            printf("Error - Rotation parameterization failure detected.\n");
            resetFlag = 1;
            continue;
        }

        if (cmdopt->trajconf) {
            FOR_0(j, pymCfg->nBody) {
                memcpy(pymCfg->body[j].b.chi_ref,
                       trajData + (i+2)*nBlenderBody*6 + corresMapIndex[j]*6,
                       sizeof(double)*6);
            }
        }

        double pureOptTime = 0;
        const char *solstaStr;
        double cost = 0;
        ret = PymOptimizeFrameMove(&pureOptTime, outputFile, pymCfg, dmstreams,
                                   &solstaStr, &cost, &cc, env);
        if (ret) {
            printf("Error - Something goes wrong "
                   "during optimization frame move.\n");
            resetFlag = 1;
            continue;
        }
        pthread_mutex_lock(&main_mutex); {
            if (cxt->stop) {
                /* stop flag signaled from the main thread */
                break;
            }
            /* Write external force excertion */
            memcpy(rbnTrunk->extForce, cxt->trunkExternalForce,
                   sizeof(double)*3);
            rbnTrunk->extForcePos[2] = 0.2;
            memset(cxt->trunkExternalForce, 0, sizeof(double)*3);
        } pthread_mutex_unlock(&main_mutex);

        const double percent = (double)(i+1)/(pymCfg->nSimFrame)*100;
        printf("%5d / %5d  (%6.2lf %%) result - %12s, cost = %.6e\n",
               i, pymCfg->nSimFrame-1, percent, solstaStr, cost);
        cxt->totalPureOptTime += pureOptTime;
        ++i;
    }
    PymCleanupMosek(&env);
    cholmod_finish(&cc);
    printf("Physics thread terminated.\n");
    return 0;
}
