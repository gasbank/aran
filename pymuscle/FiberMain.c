/*
 * FiberMain.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Entry point
 */
#include "PymPch.h"
#include "PymStruct.h"
#include "Control.h"
#include "MathUtil.h"
#include "SimCore.h"
#include "lemke.h"
#include "LCP_exp.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"

static const int MAX_CORRESMAP = 50;

int PymMin(int a, int b) {
    if (a<b) return a;
    else return b;
}

int main3(int argc, const char **argv)
{
    cholmod_common c ;
    cholmod_start (&c) ;
    int status = lemkeTester(&c);
    printf("Status = %d\n", status);
    cholmod_finish(&c);
    return 0;
}

int PymParseTrajectoryFile(char corresMap[MAX_CORRESMAP][2][128],
                           int *_nCorresMap,
                           double **_trajData,
                           int *_nBody,
                           int *_nFrame,
                           const char *fnRbCfg,
                           const char *fnTraj) {
    FILE *rbCfg = fopen(fnRbCfg, "r");
    FILE *traj  = fopen(fnTraj, "r");
    if (rbCfg == 0) {
        printf("Rigid body configuration file %s parse failed.\n", fnRbCfg);
        return -1;
    } else if (traj == 0) {
        printf("Trajectory file %s parse failed.\n", fnTraj);
        return -2;
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
    char *trajHeader = 0;
    size_t trajHeaderLen = 0;
    getline(&trajHeader, &trajHeaderLen, traj);
    int nFrame = atoi(trajHeader);
    int nBody  = atoi(strrchr(trajHeader, ' '));
    printf("nFrame = %d   nBody = %d\n", nFrame, nBody);
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
    *_trajData = trajData;
    *_nBody = nBody;
    *_nFrame = nFrame;
    *_nCorresMap = nCorresMap;
    return 0;
}

typedef struct _deviation_stat_entry {
    double chi_d_norm;
    int nContact;
    int bodyIdx;
} deviation_stat_entry;

int DevStatCompare(const void * a, const void * b) {
    deviation_stat_entry *at = (deviation_stat_entry *)a;
    deviation_stat_entry *bt = (deviation_stat_entry *)b;
    double diff = bt->chi_d_norm - at->chi_d_norm;
    if (diff > 0) return 1;
    else if (diff < 0) return -1;
    else return 0;
}

typedef struct _pym_cmdline_options_t {
    const char *simconf;
    int frame;
    const char *trajconf;
    const char *trajdata;
    const char *output;
    double slant;
} pym_cmdline_options_t;

int ParseCmdlineOptions(pym_cmdline_options_t *cmdopt, int argc, const char **argv) {
    /* initialize default options first */
    cmdopt->simconf = 0;
    cmdopt->frame = -1;   /* not specified for now */
    cmdopt->trajconf = 0;
    cmdopt->trajdata = 0;
    cmdopt->output = "outputFile.txt";

    assert(argc >= 2);
    cmdopt->simconf = argv[1];
    int i;
    int _trajconf = 0, _trajdata = 0;
    for (i = 2; i < argc; ++i) {
        if (strncmp(argv[i], "--frame=", strlen("--frame=")) == 0) {
            char *endp;
            cmdopt->frame = strtol( strchr(argv[i], '=') + 1, &endp, 10 );
            if (!(endp && *endp == '\0') || cmdopt->frame <= 0) {
                printf("Error: --frame should be provided as a positive integer.\n");
                return -4;
            }
        } else if (strncmp(argv[i], "--trajconf=", strlen("--trajconf=")) == 0) {
            cmdopt->trajconf = strchr(argv[i], '=') + 1;
            _trajconf = 1;
        } else if (strncmp(argv[i], "--trajdata=", strlen("--trajdata=")) == 0) {
            cmdopt->trajdata = strchr(argv[i], '=') + 1;
            _trajdata = 1;
        } else if (strncmp(argv[i], "--output=", strlen("--output=")) == 0) {
            cmdopt->output = strchr(argv[i], '=') + 1;
        } else if (strncmp(argv[i], "--slant=", strlen("--slant=")) == 0) {
            char *endp;
            cmdopt->slant = strtod(strchr(argv[i], '=') + 1, &endp);
            if (!(endp && *endp == '\0')) {
                printf("Error: --slant should be provided as a real number.\n");
                return -3;
            }
        } else {
            printf("Error: unknown argument provided - %s\n", argv[i]);
            return -2;
        }
    }
    if (_trajconf ^ _trajdata) {
        printf("Error: --trajconf should be specified with --trajdata and vice versa.\n\n");
        return -1;
    }
    return 0;
}

int main(int argc, const char **argv) {
    if (argc < 2) {
        printf("Rigid body and muscle fiber simulator - 2010 Geoyeob Kim\n\n");
        printf("  USAGE\n");
        printf("    %s config_file <simulation config file> [<options>]\n", strrchr(argv[0], '/') + 1);
        printf("\n");
        printf("  OPTIONS\n");
        printf("      --frame=<frame number to simulate>\n");
        printf("      --trajconf=<trajectory config file>\n");
        printf("      --trajdata=<trajectory data file>\n");
        printf("      --slant=<ground slant in radian>\n");
        printf("\n");
        printf("  --trajconf should be specified with --trajdata and vice versa.\n\n");
        return -1;
    }
    int ret = 0;
    pym_cmdline_options_t cmdopt;
    ret = ParseCmdlineOptions(&cmdopt, argc, argv);
    if (ret < 0) {
        printf("Failed.\n");
        return -2;
    }

    cholmod_common cc ;
    cholmod_start (&cc) ;

    pym_config_t pymCfg;
    ret = PymConstructConfig(cmdopt.simconf, &pymCfg);
    if (ret < 0) {
        printf("Failed.\n");
        return -1;
    }

    int i, j, k;

    PymConvertRotParamInPlace(&pymCfg, RP_EXP);

    MSKenv_t    env;
    PymInitializeMosek(&env);


    char corresMap[MAX_CORRESMAP][2][128];
    int corresMapIndex[pymCfg.nBody]; FOR_0(i, pymCfg.nBody) corresMapIndex[i] = -1;
    int nCorresMap = 0;
    int nBlenderBody = 0;
    int nBlenderFrame = 0;
    double *trajData = 0;
    if (cmdopt.trajconf) {
        PymParseTrajectoryFile(corresMap,
                               &nCorresMap,
                               &trajData,
                               &nBlenderBody,
                               &nBlenderFrame,
                               cmdopt.trajconf,
                               cmdopt.trajdata);
        assert(nCorresMap > 0);
        /*
         * We need at least three frames of trajectory data since
         * the first (frame 0) is used as previous step and
         * the second (frame 1) is used as current step and
         * the third (frame 2) is used as 'reference trajectory' for next step
         *
         * We need (nBlenderFrame-2) simulation iteration to complete
         * following the trajectory entirely.
         */
        assert(nBlenderFrame >= 3);
        FOR_0(i, pymCfg.nBody) {
            FOR_0(j, nBlenderBody) {
                if (strcmp(pymCfg.body[i].b.name, corresMap[j][1]) == 0) {
                    printf("%d, %s, %s, %d\n", i, pymCfg.body[i].b.name, corresMap[j][1], j);
                    corresMapIndex[i] = j;
                    break;
                }
            }
            assert(corresMapIndex[i] >= 0);
        }
        j = 0;
        FOR_0(i, nCorresMap) {
            printf("%20s <----> %-20s", corresMap[i][0], corresMap[i][1]);
            if (strcmp(corresMap[i][1], "*") != 0) {
                printf(" (index=%d)\n", corresMapIndex[j]);
                ++j;
            } else {
                printf("\n");
            }
        }

        char fnJaCfg[128] = {0};
        strncat(fnJaCfg, cmdopt.trajconf, 128);
        strncat(fnJaCfg, ".ja", 128);
        pymCfg.na = PymParseJointAnchorFile(pymCfg.pymJa, sizeof(pymCfg.pymJa)/sizeof(pym_joint_anchor_t), fnJaCfg);
        assert(pymCfg.na >= 0);
        printf("Info - # of joint anchors parsed = %d\n", pymCfg.na);
        FOR_0(i, pymCfg.nBody) {
            /*
             *  Set current and previous state according
             *  to the initial frame of trajectory data.
             */
            pym_rb_named_t *rbn = &pymCfg.body[i].b;
            assert(rbn->rotParam == RP_EXP);
            /* previous step */
            memcpy(rbn->p0, trajData + 0*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
            memcpy(rbn->q0, trajData + 0*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
            /* current step */
            memcpy(rbn->p,  trajData + 1*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
            memcpy(rbn->q,  trajData + 1*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
            /* update discrete velocity based on current and previous step */
            FOR_0(j, 3) {
                rbn->pd[j] = (rbn->p[j] - rbn->p0[j]) / pymCfg.h;
                rbn->qd[j] = (rbn->q[j] - rbn->q0[j]) / pymCfg.h;
            }

            /*
             * Set joint anchors for each body
             */
            FOR_0(j, pymCfg.na) {
                if (strcmp(pymCfg.pymJa[j].bodyName, rbn->name) == 0) {
                    strncpy(rbn->jointAnchorNames[rbn->nAnchor], pymCfg.pymJa[j].name, 128);
                    memcpy(rbn->jointAnchors + rbn->nAnchor, pymCfg.pymJa[j].localPos, sizeof(double)*3);
                    rbn->jointAnchors[rbn->nAnchor][3] = 1.0; /* homogeneous component */
                    ++rbn->nAnchor;
                    printf("Joint anchors added: %s on %s. (so far %d)\n", pymCfg.pymJa[j].name, rbn->name, rbn->nAnchor);
                    assert(rbn->nAnchor <= 10);
                }
            }
        }
        assert(pymCfg.na%2 == 0);
        PymConstructAnchoredJointList(&pymCfg);
        printf("Info - # of anchored joints constructed = %d\n", pymCfg.nJoint);
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
    const int nf = pymCfg.nFiber;
    const int nj = pymCfg.nJoint;

    /* Let's start the simulation happily :) */
    FOR_0(i, pymCfg.nSimFrame) {

        FOR_0(j, nb) {
            pym_rb_named_t *rbn = &pymCfg.body[j].b;
            double th_0 = sqrt(rbn->q0[0]*rbn->q0[0] + rbn->q0[1]*rbn->q0[1] + rbn->q0[2]*rbn->q0[2]);
            double th_1 = sqrt(rbn->q [0]*rbn->q [0] + rbn->q [1]*rbn->q [1] + rbn->q [2]*rbn->q [2]);

            /* we do not have any tumbling bodies */
            if (th_0 > 2*M_PI || th_1 > 2*M_PI) {
                printf("Error - %s rotation parameterization failure:\n", rbn->name);
                printf("        th_0 = %e (%e, %e, %e)\n", th_0, rbn->q0[0], rbn->q0[1], rbn->q0[2]);
                printf("        th_1 = %e (%e, %e, %e)\n", th_1, rbn->q[0], rbn->q[1], rbn->q[2]);
                abort();
            }

            /* Re-parameterize the rotation if needed */
//            while (th_0 > 2*M_PI) {
//                printf("th_0 = %lf\n", th_0);
//                FOR_0(k, 3) rbn->q0[k] = rbn->q0[k]/th_0 * (th_0-2*M_PI);
//                th_0 -= 2*M_PI;
//            }
//            while (th_1 > 2*M_PI) {
//                printf("th_1 = %lf\n", th_1);
//                FOR_0(k, 3) rbn->q[k] = rbn->q[k]/th_1 * (th_1-2*M_PI);
//                th_1 -= 2*M_PI;
//            }
//
//            if (th_0 > M_PI && th_1 > M_PI) {
//                FOR_0(k, 3) {
//                    rbn->q0[k] = (1-2*M_PI/th_0)*rbn->q0[k];
//                    rbn->q[k]  = (1-2*M_PI/th_1)*rbn->q [k];
//                    rbn->qd[k] = (rbn->q[k] - rbn->q0[k]) / pymCfg.h;
//                }
//                const double th_0new = sqrt(rbn->q0[0]*rbn->q0[0] + rbn->q0[1]*rbn->q0[1] + rbn->q0[2]*rbn->q0[2]);
//                const double th_1new = sqrt(rbn->q [0]*rbn->q [0] + rbn->q [1]*rbn->q [1] + rbn->q [2]*rbn->q [2]);
//                printf("    NOTE: %s re-parameterized. (th_0=%lf --> %lf, th_1=%lf --> %lf)\n", rbn->name, th_0, th_0new, th_1, th_1new);
//            }
        }

        if (cmdopt.trajconf) {
            FOR_0(j, pymCfg.nBody) {
                memcpy(pymCfg.body[j].b.chi_ref,
                       trajData + (i+2)*nBlenderBody*6 + corresMapIndex[j]*6,
                       sizeof(double)*6);
            }
        }

        pym_rb_statedep_t sd[nb];

        FOR_0(j, nb) {
            PymConstructRbStatedep(sd + j, pymCfg.body + j, &pymCfg, &cc);
        }

        pym_biped_eqconst_t bipEq;
        PymConstructBipedEqConst(&bipEq, sd, &pymCfg, &cc);
        //cholmod_print_sparse(bod.bipMat, "bipMat", &cc);

        // TODO: drop small elements...
        //cholmod_drop(1e-8, bipEq.bipMat, &cc);

        //PrintEntireSparseMatrix(bod.bipMat);
        //__PRINT_VECTOR_VERT(bod.bipEta, bod.bipMat->nrow);
        double xx[bipEq.bipMat->ncol];
        memset(xx, 0, sizeof(xx));
        MSKsolstae solsta;
        double cost = PymOptimize(xx, &solsta, &bipEq, sd, &pymCfg, &env, &cc);
        if (cost == FLT_MAX) {
            printf("Something goes wrong while optimizing.\n");
        }
        const char *solstaStr;
        if (solsta == MSK_SOL_STA_UNKNOWN) solstaStr = "UNKNOWN";
        else if (solsta == MSK_SOL_STA_OPTIMAL) solstaStr = "optimal";
        else if (solsta == MSK_SOL_STA_NEAR_OPTIMAL) solstaStr = "near optimal";
        else solstaStr = "ERROR!";

        printf("Frame %5d finished.\n", i);

        deviation_stat_entry dev_stat[nb];
        memset(dev_stat, 0, sizeof(deviation_stat_entry)*nb);
        printf("Results:  %15s (cost=%.10e)\n", solstaStr, cost);
        int tauOffset;
        for (j = 0, tauOffset = 0; j < nb; tauOffset += sd[j].Aci[ sd[j].Asubcols ], j++) {
            const double *chi_2 = xx + tauOffset;
            const pym_rb_named_t *rbn = &pymCfg.body[j].b;

            double chi_1[6], chi_0[6], chi_r[6], chi_v[6];
            memcpy(chi_1    , rbn->p,       sizeof(double)*3);
            memcpy(chi_1 + 3, rbn->q,       sizeof(double)*3);
            memcpy(chi_0    , rbn->p0,      sizeof(double)*3);
            memcpy(chi_0 + 3, rbn->q0,      sizeof(double)*3);
            memcpy(chi_r    , rbn->chi_ref, sizeof(double)*6);
            memcpy(chi_v    , rbn->pd,      sizeof(double)*3);
            memcpy(chi_v + 3, rbn->qd,      sizeof(double)*3);

            double chi_d[6];
            FOR_0(k, 6) {
                chi_d[k] = chi_2[k] - chi_r[k];
                dev_stat[j].chi_d_norm += chi_d[k] * chi_d[k];
            }
            dev_stat[j].chi_d_norm = sqrt(dev_stat[j].chi_d_norm);
            dev_stat[j].bodyIdx = j;
            dev_stat[j].nContact = sd[j].nContacts_2;

//            printf("  chi_0  %8s - ", rbn->name); __PRINT_VECTOR(chi_0, 6);
//            printf("     _1  %8s - ", rbn->name); __PRINT_VECTOR(chi_1, 6);
//            printf("     _2  %8s - ", rbn->name); __PRINT_VECTOR(chi_2, 6);
//            printf("  <ref>  %8s - ", rbn->name); __PRINT_VECTOR(chi_r, 6);
//            printf("  <dev>  %8s - ", rbn->name); __PRINT_VECTOR(chi_d, 6);
//            printf("  <vel>  %8s - ", rbn->name); __PRINT_VECTOR(chi_v, 6);
//            printf("\n");

            FOR_0(k, 6) fprintf(outputFile, "%18.8e", chi_2[k]);
            fprintf(outputFile, "\n");


            /* Update the current state of rigid bodies */
            SetRigidBodyChi_1(pymCfg.body + j, chi_2, &pymCfg);
        }


        qsort(dev_stat, nb, sizeof(deviation_stat_entry), DevStatCompare);
        printf("Reference trajectory deviation report\n");
        const int itemsPerLine = PymMin(nb, 6);
        int j0 = 0, j1 = itemsPerLine;
        while (j0 < nb && j1 <= nb) {
            for (j = j0; j < j1; ++j) {
                const pym_rb_named_t *rbn = &pymCfg.body[ dev_stat[j].bodyIdx ].b;
                printf("  %9s", rbn->name);
            }
            printf("\n");
            for (j = j0; j < j1; ++j) {
                printf("  %9.3e", dev_stat[j].chi_d_norm);
            }
            printf("\n");
            for (j = j0; j < j1; ++j) {
                printf("  %9d", dev_stat[j].nContact);
            }
            printf("\n");
            j0 = PymMin(nb, j0 + itemsPerLine);
            j1 = PymMin(nb, j1 + itemsPerLine);
        }

        FOR_0(j, nf) {
            pym_mf_named_t *mfn = &pymCfg.fiber[j].b;
            const double T_0        = xx[ bipEq.Aci[1] + j ];
            //const double u_0        = xx[ bipEq.Aci[nb + 1] + j ];
            const double xrest_0    = xx[ bipEq.Aci[3] + j ];
            /* Update the current state of muscle fibers */
            mfn->T     = T_0;
            mfn->xrest = xrest_0;
//            printf("%16s -   T = %15.8e     u = %15.8e     xrest = %15.8e\n", mfn->name, T_0, u_0, xrest_0);
        }

        FOR_0(j, nj) {
            const double *dAj = xx + bipEq.Aci[6] + 4*j;
            const double disloc = PymNorm(4, dAj);
            const char *aAnchorName = pymCfg.body[ pymCfg.anchoredJoints[j].aIdx ].b.jointAnchorNames[ pymCfg.anchoredJoints[j].aAnchorIdx ];
            char iden[128];
            ExtractAnchorIdentifier(iden, aAnchorName);
            printf("%12s disloc = %e", iden, disloc);
            if (j%2) printf("\n");

            if (pymCfg.anchoredJoints[j].maxDisloc < disloc)
                pymCfg.anchoredJoints[j].maxDisloc = disloc;
        }
        if (nj%2) printf("\n");

        PymDestroyBipedEqconst(&bipEq, &cc);
        FOR_0(j, pymCfg.nBody) {
            PymDestroyRbStatedep(sd + j, &cc);
        }

        if (cost == FLT_MAX) /* no meaning to process further */
        {
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            system("cat /tmp/pymoptimize_log");
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            break;
        }
    }
    fclose(outputFile);

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
    return 0;
}

