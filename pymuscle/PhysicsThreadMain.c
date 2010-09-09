/*
 * PhysicsThreadMain.c: Dedicated physics simulation thread
 *                      for realtime simulator
 * 2010 Geoyeob Kim
 */
#include "PymPch.h"
#include <pthread.h>
#include "include/PrsGraphCapi.h"
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
#include "DebugPrintDef.h"
extern pthread_mutex_t count_mutex;
extern pthread_mutex_t main_mutex;
extern pthread_cond_t count_threshold_cv;
extern pthread_cond_t physics_thread_finished;

extern const int zdatasize;
extern double zdata[100];
extern int zdatastart;
extern int zdataend;

void *PhysicsThreadMain(void *t)
{
    pym_physics_thread_context_t *phyCon = (pym_physics_thread_context_t *)t;
    pym_config_t                 *pymCfg = phyCon->pymCfg;
    pym_cmdline_options_t        *cmdopt = phyCon->cmdopt;
    const int             nBlenderBody   = phyCon->nBlenderBody;
    const int *const      corresMapIndex = phyCon->corresMapIndex;
    const double *const   trajData       = phyCon->trajData;
    FILE *outputFile = 0; /* TODO: Output file of realtime simulator */
    FILE **dmstreams = phyCon->dmstreams;

    /* Initilize CHOLMOD library */
    cholmod_common cc ;
    cholmod_start (&cc) ;
    /* Initialize MOSEK instance */
    MSKenv_t    env;
    PymInitializeMosek(&env);

    int i = 0, j, k;
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
    while (1) { /* CAUTION: local variable 'i' is a loop variable effectively. */
        if ((i >= pymCfg->nSimFrame) || resetFlag) {
            pthread_mutex_lock(&main_mutex);
            PymSetInitialStateUsingTrajectory(pymCfg, nBlenderBody,
                                              corresMapIndex, trajData);
            pthread_mutex_unlock(&main_mutex);
            i = 0;
            zdatastart = 0;
            resetFlag = 0;
        }
        /*
         * A very long and time-consuming simulation task goes here ...
         */
        int ret = PymCheckRotParam(pymCfg);
        if (ret < 0) {
            printf("Error - Rotation parameterization failure detected.\n"
                   "Press R to reset.\n");
            pthread_cond_wait(&count_threshold_cv, &main_mutex);
            printf("Reset signal detected.\n");
            resetFlag = 1;
            pthread_mutex_unlock(&main_mutex);
            //continue;
        }
        /* Set reference */
        if (cmdopt->trajconf) {
            FOR_0(j, pymCfg->nBody) {
                memcpy(pymCfg->body[j].b.chi_ref,
                       trajData + (i+2)*nBlenderBody*6 + corresMapIndex[j]*6,
                       sizeof(double)*6);
            }
        }
        /* Compute reference COM */
        double refCom[3] = {0,};
        double totMass = 0;
        FOR_0(j, pymCfg->nBody)
            totMass += pymCfg->body[j].b.m;
        FOR_0(j, pymCfg->nBody) {
            FOR_0(k, 3)
                refCom[k] += pymCfg->body[j].b.m * pymCfg->body[j].b.chi_ref[k];
        }
        FOR_0(j, 3)
            refCom[j] /= totMass;





        double pureOptTime = 0;
        const char *solstaStr;
        double cost = 0;
        ret = PymOptimizeFrameMove(&pureOptTime, outputFile, pymCfg, dmstreams,
                                   &solstaStr, &cost, &cc, env);
        if (ret) {
            printf("Error - Something goes wrong "
                   "during optimization frame move.\n"
                   "Press R to reset.\n");
            pthread_cond_wait(&count_threshold_cv, &main_mutex);
            printf("Reset signal detected.\n");
            resetFlag = 1;
            pthread_mutex_unlock(&main_mutex);
            //continue;
        }

        pthread_mutex_lock(&main_mutex); {
            if (phyCon->stop) {
                /* stop flag signaled from the main thread */
                break;
            }

            PrsGraphPushBackTo(phyCon->comGraph, PCG_SIM_COM, pymCfg->bipCom[2]);
            PrsGraphPushBackTo(phyCon->comGraph, PCG_REF_COM, refCom[2]);

            /* Copy RB and MF data to renderer-accessable memory area */
            memcpy(phyCon->renBody,   pymCfg->body,      sizeof(pym_rb_t)*pymCfg->nBody);
            memcpy(phyCon->renFiber,  pymCfg->fiber,     sizeof(pym_mf_t)*pymCfg->nFiber);
            /* Simulated biped COM position */
            memcpy(phyCon->bipCom,    pymCfg->bipCom,    sizeof(double)*3);
            /* Reference biped COM position (note that pymCfg, not phyCon!) */
            memcpy(pymCfg->bipRefCom, refCom, sizeof(double)*3);

            /* Write external force excertion */
            memcpy(rbnTrunk->extForce, phyCon->trunkExternalForce,
                   sizeof(double)*3);
            rbnTrunk->extForcePos[2] = 0.2;
            memset(phyCon->trunkExternalForce, 0, sizeof(double)*3);
        } pthread_mutex_unlock(&main_mutex);

        const double percent = (double)(i+1)/(pymCfg->nSimFrame)*100;
        printf("%5d / %5d  (%6.2lf %%) result - %12s, cost = %.6e\n",
               i, pymCfg->nSimFrame-1, percent, solstaStr, cost);
        phyCon->totalPureOptTime += pureOptTime;
        ++i;
    }
    PymCleanupMosek(&env);
    cholmod_finish(&cc);
    printf("Physics thread terminated.\n");
    return 0;
}
