/*
 * FiberMain.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Entry point
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <libconfig.h>
#include <mosek.h>
#include <umfpack.h>
#include <cholmod.h>
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

void LoadDoublesFromFile(const unsigned int len, double body[len], const char *fileName)
{
	FILE *f = fopen(fileName, "r");
	assert(f);
	unsigned int j;
	for (j = 0; j < len; ++j)
	{
		int ret = fscanf(f, "%lf", &body[j]);
		assert(ret == 1);
	}
	fclose(f);
}

int WriteState(FILE *sr, int curFrame, int nBody, int nMuscle, double body[nBody][18], double muscle[nMuscle][12], double cost, double ustar[nMuscle])
{
    assert(sr);
    fprintf(sr, "%d", curFrame);
    int j;
    for (j = 0; j < nBody; ++j)
    {
        /* Write the angular and linear position to the file. (for plotting) */
        /* Euler angles instead of quaternions used (human friendly) */
        double bjEul[3];
        QuatToEuler(bjEul, &body[j][3]);
        fprintf(sr, " %lf %lf %lf %lf %lf %lf",
                    body[j][0], body[j][1], body[j][2],
                    bjEul[0], bjEul[1], bjEul[2]);
    }
    for (j = 0; j < nMuscle; ++j)
        fprintf(sr, " %lf", muscle[j][4]);
    fprintf(sr, " %lf", cost);
    for (j = 0; j < nMuscle; ++j)
        fprintf(sr, " %lf", ustar[j]);
    fprintf(sr, "\n");
    return 0;
}

void CreateOutputTableHeader(char tableHeader[4096], int nBody, int nMuscle,
                             char bodyName[nBody][128], char muscleName[nBody][128])
{
    strcat(tableHeader, "frame");
    int j;
    for (j = 0; j < nBody; ++j)
    {
        char tableHeaderTemp[128];
        snprintf(tableHeaderTemp, 128, " %s[x] %s[y] %s[z] %s[e1] %s[e2] %s[e3]",
                 bodyName[j], bodyName[j], bodyName[j], bodyName[j], bodyName[j], bodyName[j]);
        strncat(tableHeader, tableHeaderTemp, 4095);
    }
    for (j = 0; j < nMuscle; ++j)
    {
        char tableHeaderTemp[128];
        sprintf(tableHeaderTemp, " %s", muscleName[j]);
        strncat(tableHeader, tableHeaderTemp, 4095);
    }
    strncat(tableHeader, " cost", 4095);
    for (j = 0; j < nMuscle; ++j)
    {
        char tableHeaderTemp[128];
        sprintf(tableHeaderTemp, " %s_A", muscleName[j]);
        strncat(tableHeader, tableHeaderTemp, 4095);
    }
    strncat(tableHeader, "\n", 4095);
}

int main4(int argc, const char **argv)
{
    LCP_exp_test();
    return 0;
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

int PymParseTrajectoryFile(char corresMap[20][2][128],
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
        assert(nCorresMap <= 20);
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
} pym_cmdline_options_t;

int ParseCmdlineOptions(pym_cmdline_options_t *cmdopt, int argc, const char **argv) {
    /* initialize default options first */
    cmdopt->simconf = 0;
    cmdopt->frame = 100;
    cmdopt->trajconf = 0;
    cmdopt->trajdata = 0;
    cmdopt->output = "outputFile.txt";

    assert(argc >= 2);
    cmdopt->simconf = argv[1];
    int i;
    int _trajconf = 0, _trajdata = 0;
    for (i = 2; i < argc; ++i) {
        if (strncmp(argv[i], "--frame=", strlen("--frame=")) == 0) {
            cmdopt->frame = atoi( strchr(argv[i], '=') + 1 );
        } else if (strncmp(argv[i], "--trajconf=", strlen("--trajconf=")) == 0) {
            cmdopt->trajconf = strchr(argv[i], '=') + 1;
            _trajconf = 1;
        } else if (strncmp(argv[i], "--trajdata=", strlen("--trajdata=")) == 0) {
            cmdopt->trajdata = strchr(argv[i], '=') + 1;
            _trajdata = 1;
        } else if (strncmp(argv[i], "--output=", strlen("--output=")) == 0) {
            cmdopt->output = strchr(argv[i], '=') + 1;
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
        printf("      --trajdata=<trajectory data file>");
        printf("\n\n");
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
    const int nb = pymCfg.nBody;
    const int nf = pymCfg.nFiber;

    int i, j, k;

    PymConvertRotParamInPlace(&pymCfg, RP_EXP);

    MSKenv_t    env;
    PymInitializeMosek(&env);


    char corresMap[20][2][128];
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
        assert(nBlenderFrame >= 2);
        FOR_0(i, pymCfg.nBody) {
            FOR_0(j, nBlenderBody) {
                if (strcmp(pymCfg.body[i].b.name, corresMap[j][1]) == 0) {
                    corresMapIndex[i] = j;
                    break;
                }
            }
            assert(corresMapIndex[i] >= 0);
        }
        j = 0;
        FOR_0(i, nCorresMap) {
            printf("%20s <----> %-20s", corresMap[i][0], corresMap[i][1]);
            if (corresMap[i][1][0] != '*') {
                printf(" (index=%d)\n", corresMapIndex[j]);
                ++j;
            } else {
                printf("\n");
            }
        }

        /*
         *  Set current and previous state according
         *  to the initial frame of trajectory data.
         */
        FOR_0(i, pymCfg.nBody) {
            pym_rb_named_t *rbn = &pymCfg.body[i].b;
            assert(rbn->rotParam == RP_EXP);
            double p2[3], q2[3];
            memcpy(rbn->p, trajData + 0*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
            memcpy(rbn->q, trajData + 0*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
            memcpy(p2,     trajData + 1*nBlenderBody*6 + corresMapIndex[i]*6 + 0, sizeof(double)*3);
            memcpy(q2,     trajData + 1*nBlenderBody*6 + corresMapIndex[i]*6 + 3, sizeof(double)*3);
            FOR_0(j, 3) {
                rbn->pd[j] = (p2[j] - rbn->p[j]) / pymCfg.h;
                rbn->qd[j] = (q2[j] - rbn->q[j]) / pymCfg.h;
                rbn->p0[j] = rbn->p[j] - pymCfg.h*rbn->pd[j];
                rbn->q0[j] = rbn->q[j] - pymCfg.h*rbn->qd[j];
            }
        }
    } else {
        PymSetPymCfgChiRefToCurrentState(&pymCfg);
    }


    pymCfg.nSimFrame = cmdopt.frame;

    FILE *outputFile = fopen(cmdopt.output, "w");
    if (!outputFile) {
        printf("Error: Opening the output file %s failed.\n", cmdopt.output);
        return -3;
    }
    fprintf(outputFile, "%d %d\n", pymCfg.nSimFrame, nb);
    /* Let's start the simulation happily :) */
    FOR_0(i, pymCfg.nSimFrame) {

        /* Reparameterize the rotation if needed */
//        FOR_0(j, nb) {
//            pym_rb_named_t *rbn = &pymCfg.body[j].b;
//            double th_0 = sqrt(rbn->q0[0]*rbn->q0[0] + rbn->q0[1]*rbn->q0[1] + rbn->q0[2]*rbn->q0[2]);
//            double th_1 = sqrt(rbn->q [0]*rbn->q [0] + rbn->q [1]*rbn->q [1] + rbn->q [2]*rbn->q [2]);
//
//            while (th_0 > 2*M_PI) {
//                FOR_0(k, 3) rbn->q0[k] = rbn->q0[k]/th_0 * (th_0-2*M_PI);
//                th_0 -= 2*M_PI;
//            }
//            while (th_1 > 2*M_PI) {
//                FOR_0(k, 3) rbn->q[k] = rbn->q[k]/th_1 * (th_1-2*M_PI);
//                th_1 -= 2*M_PI;
//            }
//
//            if (th_0 > M_PI && th_1 > M_PI) {
//                FOR_0(k, 3) rbn->q0[k] = (1-2*M_PI/th_0)*rbn->q0[k];
//                FOR_0(k, 3) rbn->q[k]  = (1-2*M_PI/th_1)*rbn->q [k];
//                FOR_0(k, 3) rbn->qd[k] = (rbn->q[k] - rbn->q0[k]) / pymCfg.h;
//                const double th_0new = sqrt(rbn->q0[0]*rbn->q0[0] + rbn->q0[1]*rbn->q0[1] + rbn->q0[2]*rbn->q0[2]);
//                const double th_1new = sqrt(rbn->q [0]*rbn->q [0] + rbn->q [1]*rbn->q [1] + rbn->q [2]*rbn->q [2]);
//                printf("    NOTE: %s reparameterized. (th_0=%lf --> %lf, th_1=%lf --> %lf)\n", rbn->name, th_0, th_0new, th_1, th_1new);
//            }
//        }

        if (argc == 4 || argc == 5) {
            FOR_0(j, pymCfg.nBody) {
                memcpy(pymCfg.body[j].b.chi_ref  , trajData + (i+1)*nBlenderBody*6 + corresMapIndex[j]*6, sizeof(double)*6);
            }
        }

        pym_rb_statedep_t sd[nb];

        FOR_0(j, nb) {
            PymConstructRbStatedep(sd + j, pymCfg.body + j, &pymCfg, &cc);
        }

        pym_biped_eqconst_t bipEq;
        PymConstructBipedEqConst(&bipEq, sd, &pymCfg, &cc);
        //cholmod_print_sparse(bod.bipMat, "bipMat", &cc);

        //cholmod_drop(1e-16, bod.bipMat, &cc);

        //PrintEntireSparseMatrix(bod.bipMat);
        //__PRINT_VECTOR_VERT(bod.bipEta, bod.bipMat->nrow);
        double xx[bipEq.bipMat->ncol];
        MSKsolstae solsta;
        double cost = PymOptimize(xx, &solsta, &bipEq, sd, &pymCfg, &env, &cc);
        const char *solstaStr;
        if (solsta == MSK_SOL_STA_UNKNOWN) solstaStr = "***UNKNOWN***";
        else if (solsta == MSK_SOL_STA_OPTIMAL) solstaStr = "optimal";
        else if (solsta == MSK_SOL_STA_NEAR_OPTIMAL) solstaStr = "near optimal";
        else assert(0);

        //if (i%10 == 0)
            printf("Frame %5d finished.\n", i);

        deviation_stat_entry dev_stat[nb];
        memset(dev_stat, 0, sizeof(deviation_stat_entry)*nb);
        printf("Results:  %15s (cost=%.10e)\n", solstaStr, cost);
        FOR_0(j, nb) {
            const double *chi_2 = xx + bipEq.Aci[j];
            const pym_rb_named_t *rbn = &pymCfg.body[j].b;

            double chi_1[6], chi_0[6], chi_r[6];
            memcpy(chi_1    , rbn->p,       sizeof(double)*3);
            memcpy(chi_1 + 3, rbn->q,       sizeof(double)*3);
            memcpy(chi_0    , rbn->p0,      sizeof(double)*3);
            memcpy(chi_0 + 3, rbn->q0,      sizeof(double)*3);
            memcpy(chi_r    , rbn->chi_ref, sizeof(double)*6);
            double chi_d[6];
            FOR_0(k, 6) {
                chi_d[k] = chi_2[k] - chi_r[k];
                dev_stat[j].chi_d_norm += chi_d[k] * chi_d[k];
            }
            dev_stat[j].chi_d_norm = sqrt(dev_stat[j].chi_d_norm);
            dev_stat[j].bodyIdx = j;

//            printf("  chi_0  %8s - ", rbn->name); __PRINT_VECTOR(chi_0, 6);
//            printf("     _1  %8s - ", rbn->name); __PRINT_VECTOR(chi_1, 6);
//            printf("     _2  %8s - ", rbn->name); __PRINT_VECTOR(chi_2, 6);
//            printf("  <ref>  %8s - ", rbn->name); __PRINT_VECTOR(chi_r, 6);
//            printf("  <dev>  %8s - ", rbn->name); __PRINT_VECTOR(chi_d, 6);
//            printf("\n");

            FOR_0(k, 6) fprintf(outputFile, "%18.8e", chi_2[k]);
            fprintf(outputFile, "\n");


            /* Update the current state */
            SetRigidBodyChi_1(pymCfg.body + j, chi_2 );
        }

        printf("  Reference trajectory deviation report\n");
        qsort(dev_stat, nb, sizeof(deviation_stat_entry), DevStatCompare);
        FOR_0(j, nb) {
            const pym_rb_named_t *rbn = &pymCfg.body[ dev_stat[j].bodyIdx ].b;
            printf("  %12s", rbn->name);
        }
        printf("\n");
        FOR_0(j, nb) {
            printf("  %12.5e", dev_stat[j].chi_d_norm);
        }
        printf("\n");

        FOR_0(j, nf) {
            pym_mf_named_t *mfn = &pymCfg.fiber[j].b;
            const double T_0        = xx[ bipEq.Aci[nb + 0] + j ];
            const double u_0        = xx[ bipEq.Aci[nb + 1] + j ];
            const double xrest_0    = xx[ bipEq.Aci[nb + 2] + j ];
            mfn->T     = T_0;
            mfn->xrest = xrest_0;
//            printf("%16s -   T = %15.8e     u = %15.8e     xrest = %15.8e\n", mfn->name, T_0, u_0, xrest_0);
        }

        PymDestroyBipedEqconst(&bipEq, &cc);
        FOR_0(j, pymCfg.nBody) {
            PymDestroyRbStatedep(sd + j, &cc);
        }
    }
    fclose(outputFile);

    PymCleanupMosek(&env);
    PymDestoryConfig(&pymCfg);
    cholmod_finish(&cc);
    free(trajData);
    return 0;
}

int mainxx(int argc, const char **argv)
{
    printf("Rigid body and muscle fiber simulator\n");
    printf("  2010 Geoyeob Kim\n\n");
    if (argc != 2)
    {
        /* extract the executable file name only */
        const char *exeName = strrchr(argv[0], '/') + 1;
        printf("Usage) %s configuration_file\n\n", exeName);
        return -20;
    }
    const char *fnConf = argv[1];
    int i, j, k;
    config_t conf;
    config_init(&conf);
    config_set_auto_convert(&conf, 1);
    int confret = config_read_file(&conf, fnConf);
    if (confret != CONFIG_TRUE)
    {
        const char *errText = config_error_text(&conf);
        const char *errFile = config_error_file(&conf);
        const int errLine = config_error_line(&conf);
        printf("Configuration file %s (line %d) %s!\n", errFile, errLine, errText);
        config_destroy(&conf);
        exit(-10);
    }

    const char *ver;
    config_lookup_string(&conf, "version", &ver);
    printf("  Config file version: %s\n", ver);

    /*
     * Parse body configurations
     */
    config_setting_t *bodyConf = config_lookup(&conf, "body");
    assert(bodyConf);
    const unsigned int nBody = config_setting_length(bodyConf);
    double body[nBody][18];
	double extForce[nBody][6];
	char bodyName[nBody][128];
	memset(extForce, 0, sizeof(double)*nBody*6); /* NOTE: No external force for now. */
    for (j = 0; j < nBody; ++j)
    {
        config_setting_t *bConf = config_setting_get_elem(bodyConf, j);
        const char *bName;
        config_setting_lookup_string(bConf, "name", &bName);
        strncpy(bodyName[j], bName, 128);
        printf("    Body %3d : %s\n", j, bName);
        #define csgfe(a,b) config_setting_get_float_elem(a,b)
        config_setting_t *p = config_setting_get_member(bConf, "p"); assert(p);
        body[j][0] = csgfe(p, 0); body[j][1] = csgfe(p, 1); body[j][2] = csgfe(p, 2);
        config_setting_t *q = config_setting_get_member(bConf, "q"); assert(q);
        body[j][3] = csgfe(q, 0); body[j][4] = csgfe(q, 1); body[j][5] = csgfe(q, 2); body[j][6] = csgfe(q, 3);
        config_setting_t *pd = config_setting_get_member(bConf, "pd"); assert(pd);
        body[j][7] = csgfe(pd, 0); body[j][8] = csgfe(pd, 1); body[j][9] = csgfe(pd, 2);
        config_setting_t *qd = config_setting_get_member(bConf, "qd"); assert(qd);
        body[j][10] = csgfe(qd, 0); body[j][11] = csgfe(qd, 1); body[j][12] = csgfe(qd, 2); body[j][13] = csgfe(qd, 3);
        config_setting_t *mass = config_setting_get_member(bConf, "mass"); assert(mass);
        body[j][14] = config_setting_get_float(mass);
        config_setting_t *size = config_setting_get_member(bConf, "size"); assert(size);
        double sx = csgfe(size, 0), sy = csgfe(size, 1), sz = csgfe(size, 2);
        BoxInertiaTensorFromMassAndSize(&body[j][15], body[j][14], sx, sy, sz);
        config_setting_t *cExtForce = config_setting_get_member(bConf, "extForce");
        if (cExtForce)
        {
            extForce[j][0] = csgfe(cExtForce, 0);
            extForce[j][1] = csgfe(cExtForce, 1);
            extForce[j][2] = csgfe(cExtForce, 2);
        }
        config_setting_t *cExtTorque = config_setting_get_member(bConf, "extTorque");
        if (cExtTorque)
        {
            extForce[j][3] = csgfe(cExtTorque, 0);
            extForce[j][4] = csgfe(cExtTorque, 1);
            extForce[j][5] = csgfe(cExtTorque, 2);
        }
        /*
         * Add gravitational force as external force if grav field is true.
         */
        config_setting_t *grav = config_setting_get_member(bConf, "grav");
        if (grav && config_setting_get_bool(grav))
        {
            extForce[j][2] += body[j][14]*(-9.81);
        }
        #undef csgfe
    }
    /*
     * Parse fiber configurations
     */
    config_setting_t *muscleConf = config_lookup(&conf, "muscle");
    assert(muscleConf);
	const unsigned int nMuscle = config_setting_length(muscleConf);
	double muscle[nMuscle][12];
	unsigned int musclePair[nMuscle][2];
	char muscleName[nMuscle][128];
    for (j = 0; j < nMuscle; ++j)
    {
        config_setting_t *mConf = config_setting_get_elem(muscleConf, j);
        const char *mName;
        config_setting_lookup_string(mConf, "name", &mName);
        strncpy(muscleName[j], mName, 128);
        printf("    Fiber %3d : %s\n", j, mName);
        config_setting_t *KSE = config_setting_get_member(mConf, "KSE"); assert(KSE);
        muscle[j][0] = config_setting_get_float(KSE);
        config_setting_t *KPE = config_setting_get_member(mConf, "KPE"); assert(KPE);
        muscle[j][1] = config_setting_get_float(KPE);
        config_setting_t *b = config_setting_get_member(mConf, "b"); assert(b);
        muscle[j][2] = config_setting_get_float(b);
        config_setting_t *xrest = config_setting_get_member(mConf, "xrest"); assert(xrest);
        muscle[j][3] = config_setting_get_float(xrest);
        config_setting_t *T = config_setting_get_member(mConf, "T"); assert(T);
        muscle[j][4] = config_setting_get_float(T);
        config_setting_t *A = config_setting_get_member(mConf, "A"); assert(A);
        muscle[j][5] = config_setting_get_float(A);
        #define csgfe(a,b) config_setting_get_float_elem(a,b)
        config_setting_t *originPos = config_setting_get_member(mConf, "originPos"); assert(originPos);
        muscle[j][6] = csgfe(originPos, 0);
        muscle[j][7] = csgfe(originPos, 1);
        muscle[j][8] = csgfe(originPos, 2);
        config_setting_t *insertionPos = config_setting_get_member(mConf, "insertionPos"); assert(insertionPos);
        muscle[j][9] = csgfe(insertionPos, 0);
        muscle[j][10] = csgfe(insertionPos, 1);
        muscle[j][11] = csgfe(insertionPos, 2);
        #undef csgfe

        const char *orgBodyName, *insBodyName;
        config_setting_lookup_string(mConf, "origin", &orgBodyName);
        config_setting_lookup_string(mConf, "insertion", &insBodyName);
        int boIdx = FindBodyIndex(nBody, bodyName, orgBodyName);
        int biIdx = FindBodyIndex(nBody, bodyName, insBodyName);
        assert(boIdx >= 0 && biIdx >= 0);
        musclePair[j][0] = boIdx;
        musclePair[j][1] = biIdx;
    }

    double h; /* simulation time step */
    confret = config_lookup_float(&conf, "h", &h);
    assert(confret == CONFIG_TRUE);
    int simFrame; /* simulation length */
    confret = config_lookup_int(&conf, "simFrame", &simFrame);
    assert(confret == CONFIG_TRUE);
    int plotSamplingRate; /* Write 1 sample per 'plotSamplingRate' */
    confret = config_lookup_int(&conf, "plotSamplingRate", &plotSamplingRate);
    assert(confret == CONFIG_TRUE);
    char cOutput[128]; /* File name result is written */
    const char *cOutputTemp;
    confret = config_lookup_string(&conf, "output", &cOutputTemp);
    assert(confret == CONFIG_TRUE);
    strncpy(cOutput, cOutputTemp, 128);
    config_destroy(&conf);

    /***************************************************************/
    for (j = 0; j < nBody; ++j)
    {
        /*
         * Normalize the quaternions for each body
         * to ensure they represent rotations.
         */
        NormalizeVector(4, &body[j][3]);
    }
    /***************************************************************/


    unsigned int curFrame;
    printf("  Opening %s for output\n", cOutput);
    FILE *sr = fopen(cOutput, "w");
    assert(sr);
    /*
     * tableHeader looks like this :
     *   frame p1[x] p1[y] p1[z] p1[e1] p1[e2] p1[e3]
     *     .... pn[x] pn[y] pn[z] pn[e1] pn[e2] pn[e3]
     *     T1 ... Tm
     */
    char tableHeader[4096] = { 0 };
    CreateOutputTableHeader(tableHeader, nBody, nMuscle, bodyName, muscleName);
    fprintf(sr, "%s", tableHeader);

    double t0, t1;
    t0 = umfpack_timer ( ) ;

    const unsigned int nd = 3 + 4; /* Degree-of-freedom for a single rigid body */
    const unsigned int nY = 2*nd*nBody + nMuscle;
    /*
     * Desired Y value
     */
    double Ydesired[nY];
    memset(Ydesired, 0, sizeof(double) * nY);
    for (j = 0, i = 0; j < nBody; ++j)
    {
        for (k = 0; k < 14; ++k, ++i)
        {
            Ydesired[i] = body[j][k];
        }
    }
    assert(i == (nY - nMuscle));

    /*
     * Diagonal component of the matrices W_Y and W_u
     */
    double w_y[nY], w_u[nY];
    for (j = 0; j < nY; ++j)
    {
        if (j < 2*nd*nBody)
        {
            w_y[j] = 1e+6;
            w_u[j] = 0;
        }
        else
        {
            w_y[j] = 1e-6;
            w_u[j] = 0;
        }
    }
    /*
     * Construct the constant matrix F
     */
    cholmod_common c ;
    cholmod_start (&c) ;

    cholmod_sparse *Fsp = constructMatrixF(nd, nBody, nMuscle, &c);
    cholmod_sparse *W_Ysp = constructSparseDiagonalMatrix(nY, w_y, &c);
    cholmod_sparse *W_usp = constructSparseDiagonalMatrix(nY, w_u, &c);

    //printf("nY = %d\n", nY);
    //cholmod_print_sparse(W_Ysp, "W_Ysp", &c);
    //exit(0);

    /********* DEBUG **********/
    double fix[14];
    memcpy(fix, body[1], sizeof(double)*14);
    Ydesired[2] += 0.5;
    /*********** START SIMULATION ************/
    double cost = 0, cost2 = 0;
    double ustar[nMuscle];
    memset(ustar, 0, sizeof(double) * nMuscle);
    for (curFrame = 0; curFrame < simFrame; ++curFrame)
    {
        if (curFrame % 100 == 0)
        {
            printf("  - Simulating %5d frame... (%6.2f %%)\n", curFrame, (float)curFrame/simFrame*100);
        }

        int bWriteSample = (curFrame % plotSamplingRate == 0);
        if (bWriteSample)
            WriteState(sr, curFrame, nBody, nMuscle, body, muscle, cost, ustar);

        SimCore(h,          /*   1  */
                nBody,      /*   2  */
                nMuscle,    /*   3  */
                nd,         /*   4  */
                nY,         /*   5  */
                body,       /*   6  */
                extForce,   /*   7  */
                muscle,     /*   8  */
                musclePair, /*   9  */
                &cost,      /*  10  */
                &cost2,     /*  11  */
                ustar,      /*  12  */
                Ydesired,   /*  13  */
                w_y,        /*  14  */
                w_u,        /*  15  */
                W_Ysp,      /*  16  */
                W_usp,      /*  17  */
                &c          /*  19  */
                );

        /******* DEBUG **********/
        memcpy(body[1],fix,sizeof(double)*14);
    }
    t1 = umfpack_timer ( ) ;

    cholmod_free_sparse(&Fsp, &c);
    cholmod_free_sparse(&W_Ysp, &c);
    cholmod_free_sparse(&W_usp, &c);
    cholmod_finish(&c);

    printf("  - Simulating %5d frame... (%6.2f %%)\n", curFrame, (float)curFrame/simFrame*100);

    /* Write the final state */
    WriteState(sr, curFrame, nBody, nMuscle, body, muscle, cost, ustar);
    fclose(sr);
    printf("  Closing %s\n", cOutput);
    printf("  Finished.\n");
    printf("  Time elapsed: %lf sec\n", t1-t0);

	return (0);
}


/*
int SolverTest()
{
    int    n = 5 ;
    int    Ap [ ] = {0, 2, 5, 9, 10, 12} ;
    int    Ai [ ] = { 0, 1, 0,     2, 4, 1, 2, 3,       4, 2, 1, 4} ;
    double Ax [ ] = {2., 3., 3., -1., 4., 4., -3., 1., 2., 2., 6., 1.} ;
    double b [ ] = {8., 45., -3., 3., 19.} ;
    double x [5] ;

    double *null = (double *) NULL ;
    int i ;
    void *Symbolic, *Numeric ;
    (void) umfpack_di_symbolic (n, n, Ap, Ai, Ax, &Symbolic, null, null) ;
    (void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
    umfpack_di_free_symbolic (&Symbolic) ;
    (void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, null, null) ;
    umfpack_di_free_numeric (&Numeric) ;
    for (i = 0 ; i < n ; i++) printf ("x [%d] = %g\n", i, x [i]) ;


    return 0;
}
*/
