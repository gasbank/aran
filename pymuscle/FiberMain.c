/*
 * FiberMain.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Entry point
 */
#include "PymPch.h"
#include "PymStruct.h"
#include "MathUtil.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PymCmdLineParser.h"

int main(int argc, char *argv[]) {
    printf("Optimization-based tracker      -- 2010 Geoyeob Kim\n");
    if (argc < 2) {
        printf("  Usage:\n");
        printf("    %s config_file <simulation conf file> [Options]\n", strrchr(argv[0], '/') + 1);
        printf("\n");
        printf("  Options:\n");
        printf("    --frame=<integer>     : frame number to simulate>\n");
        printf("    --trajconf=<path>     : input trajectory conf\n");
        printf("    --trajdata=<path>     : input trajectory data(reference)>\n");
        printf("    --output=<path>       : optimization output\n");
        printf("    --slant=<real number> : ground slant degree in radians\n");
        printf("    --notrack             : ignore reference trajectories\n");
        printf("\n");
        printf("    --trajconf should be specified with --trajdata and vice versa.\n\n");
        return -1;
    }
    int ret = 0;
    /* Debug Message Flags */
    int dmflags[PDMTE_COUNT] = {0,};
    dmflags[PDMTE_INIT_MF_FOR_EACH_RB]                    = 0;
    dmflags[PDMTE_INIT_RB_CORRESPONDENCE_1]               = 0;
    dmflags[PDMTE_INIT_RB_CORRESPONDENCE_2]               = 0;
    dmflags[PDMTE_INIT_JOINT_ANCHOR_ATTACH_REPORT]        = 0;
    dmflags[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT]         = 0;
    dmflags[PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT] = 0;

    FILE *dmstreams[PDMTE_COUNT];
    int i, j;
    FILE *devnull = fopen("/dev/null", "w");
    FOR_0(i, PDMTE_COUNT) {
    	if (dmflags[i]) dmstreams[i] = stdout;
    	else dmstreams[i] = devnull;
    }

    pym_cmdline_options_t cmdopt;
    ret = PymParseCmdlineOptions(&cmdopt, argc, argv);
    if (ret < 0) {
        printf("Failed.\n");
        return -2;
    }

    cholmod_common cc ;
    cholmod_start (&cc) ;

    MSKenv_t    env;
    PymInitializeMosek(&env);

    pym_config_t pymCfg;
    ret = PymConstructConfig(cmdopt.simconf, &pymCfg, dmstreams[PDMTE_INIT_MF_FOR_EACH_RB]);
    if (ret < 0) {
        printf("Failed.\n");
        return -1;
    }
    PymConvertRotParamInPlace(&pymCfg, RP_EXP);

    int exportFps = 0;
    int nBlenderFrame = 0;
    int nBlenderBody = 0;
    int corresMapIndex[pymCfg.nBody];
    FOR_0(i, pymCfg.nBody)
        corresMapIndex[i] = -1;
    double *trajData = 0;
    if (cmdopt.trajconf) {
        char corresMap[MAX_CORRESMAP][2][128];
        int nCorresMap = 0;
    	int parseRet = PymParseTrajectoryFile(
    			corresMap,
    			&nCorresMap,
    			&trajData,
    			&nBlenderBody,
    			&nBlenderFrame,
    			&exportFps,
    			cmdopt.trajconf,
    			cmdopt.trajdata);
        assert(parseRet == 0);
    	assert(nCorresMap > 0);
    	/* The simulation time step defined in simconf and
    	 * the frame time (reciprocal of FPS) in trajdata
    	 * should have the same value. If mismatch happens
    	 * we ignore simulation time step in simconf.
    	 */
    	if (fabs(1.0/exportFps - pymCfg.h) > 1e-6) {
    		printf("Warning - simulation time step defined in simconf and\n");
    		printf("          trajectory data do not match.\n");
    		printf("            simconf  : %3d FPS (%lf sec per frame)\n", (int)ceil(1.0/pymCfg.h), pymCfg.h);
    		printf("            trajconf : %3d FPS (%lf sec per frame)\n", exportFps, 1.0/exportFps);
    		printf("          simconf's value will be ignored.\n");
    		pymCfg.h = 1.0/exportFps;
    	}

        PymCorresMapIndexFromCorresMap(corresMapIndex,
                                       nCorresMap,
                                       corresMap,
                                       nBlenderBody,
                                       &pymCfg,
                                       dmstreams);
        char fnJaCfg[128] = {0};
        PymInferJointAnchorConfFileName(fnJaCfg, cmdopt.trajconf);
        pymCfg.na = PymParseJointAnchorFile(pymCfg.pymJa, sizeof(pymCfg.pymJa)/sizeof(pym_joint_anchor_t), fnJaCfg);
        assert(pymCfg.na >= 0);
        printf("Info - # of joint anchors parsed = %d\n", pymCfg.na);

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
        assert(nBlenderFrame >= 3);
        PymSetInitialStateUsingTrajectory(&pymCfg, nBlenderBody, corresMapIndex, trajData);
        PymInitJointAnchors(&pymCfg, dmstreams);
        PymConstructAnchoredJointList(&pymCfg);
    } else {
        PymSetPymCfgChiRefToCurrentState(&pymCfg);
    }

    if (cmdopt.frame >= 0)
        pymCfg.nSimFrame = cmdopt.frame;
    else if (cmdopt.trajconf)
        pymCfg.nSimFrame = nBlenderFrame - 2;
    else
        pymCfg.nSimFrame = 100;

    FILE *outputFile = fopen(cmdopt.output, "w");
    if (!outputFile) {
        printf("Error: Opening the output file %s failed.\n", cmdopt.output);
        return -3;
    }
    fprintf(outputFile, "%d %d\n", pymCfg.nSimFrame, pymCfg.nBody);

    /* shorthand notations */
    const int nb = pymCfg.nBody;
    //const int nf = pymCfg.nFiber;
    const int nj = pymCfg.nJoint;

    /* Let's start the simulation happily :) */
    printf("Starting the tracking simulation...\n");
    double totalPureOptTime = 0;

    pym_rb_named_t *rbnTrunk = 0;
    FOR_0(j, nb) {
        pym_rb_named_t *rbn = &pymCfg.body[j].b;
        if (strcmp(rbn->name, "trunk") == 0) {
            rbnTrunk = rbn;
        }
    }
    assert(rbnTrunk);
    FOR_0(i, pymCfg.nSimFrame) {


        /*
         * TODO [TUNE] External force scenario
         */
//        if (100 <= i && i < 110) {
//            rbnTrunk->extForce[0] = -1100;
//            rbnTrunk->extForce[1] = -1000;
//            rbnTrunk->extForcePos[1] = 0.4;
//        } else if (800 <= i && i < 805) {
//            rbnTrunk->extForce[0] = 1580;
//            rbnTrunk->extForce[1] = 0;
//            rbnTrunk->extForce[2] = -1000;
//            rbnTrunk->extForcePos[1] = 0.4;
//        } else if (1100 <= i && i < 1112) {
//            rbnTrunk->extForce[0] = 1500; //-1500;
//            rbnTrunk->extForce[1] = 0;
//            rbnTrunk->extForce[2] = 0;
//            rbnTrunk->extForcePos[1] = 0.0;
//        } else {
//            rbnTrunk->extForce[0] = 0;
//            rbnTrunk->extForce[1] = 0;
//            rbnTrunk->extForce[2] = 0;
//        }


        ret = PymCheckRotParam(&pymCfg);
        if (ret < 0) {
            printf("Error - Rotation parameterization failure detected.\n");
            break;
        }

        if (cmdopt.trajconf) {
            FOR_0(j, pymCfg.nBody) {
                memcpy(pymCfg.body[j].b.chi_ref,
                       trajData + (i+2)*nBlenderBody*6 + corresMapIndex[j]*6,
                       sizeof(double)*6);
            }
        }

        double pureOptTime = 0;
        const char *solstaStr;
        double cost = 0;
        ret = PymOptimizeFrameMove(&pureOptTime, outputFile,
                                   &pymCfg, dmstreams,
                                   &solstaStr, &cost,
                                   &cc, env);
        if (ret) {
            printf("Error - Something goes wrong during optimization frame move.\n");
            break;
        }
        printf("%5d / %5d  (%6.2lf %%) result - %12s, cost = %.6e\n",
               i, pymCfg.nSimFrame-1, (double)(i+1)/(pymCfg.nSimFrame)*100, solstaStr, cost);
        totalPureOptTime += pureOptTime;
    }
    fclose(outputFile);
    printf("Output written to %s\n", cmdopt.output);

    FILE *dislocFile = fopen("disloc.conf", "w");
    FOR_0(j, nj) {
        const char *aAnchorName = pymCfg.body[ pymCfg.anchoredJoints[j].aIdx ].b.jointAnchorNames[ pymCfg.anchoredJoints[j].aAnchorIdx ];
        char iden[128];
        ExtractAnchorIdentifier(iden, aAnchorName);
        fprintf(dislocFile, "%8s %e\n", iden, pymCfg.anchoredJoints[j].maxDisloc);
    }
    fclose(dislocFile);

    PymCleanupMosek(&env);
    PymDestoryConfig(&pymCfg);
    cholmod_finish(&cc);
    free(trajData);

    if (cmdopt.freeTrajStrings) {
        free(cmdopt.trajconf);
        free(cmdopt.trajdata);
    }
    if (cmdopt.freeOutputStrings) {
        free(cmdopt.output);
    }

    printf("Accumulated pure MOSEK optimizer time : %lf s\n", totalPureOptTime);
    return 0;
}

