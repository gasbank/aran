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
#include "PymStruct.h"
#include "umfpack.h"
#include "cholmod.h"
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
/*
int main()
{

    double a[5][5] = { {1, 0, 0, 4, 5},
                       {0, 0, 8, 9, 0},
                       {0, 7, 0,-0, 1e-4},
                       {1e-15,0, 8, 9, 0},
                       {0, 7,-8, 9, 0} };
    ToSparse(5,5,a);

    return 0;
}
*/


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


int main(int argc, const char **argv) {
    if (argc != 2) {
        printf("Rigid body and muscle fiber simulator                2010 Geoyeob Kim\n\n");
        printf("  Usage\n");
        printf("    %s [config file]\n\n", strrchr(argv[0], '/') + 1);
        exit(-123);
    }
    cholmod_common cc ;
    cholmod_start (&cc) ;

    PymuscleConfig pymCfg; AllocConfig(argv[1], &pymCfg);
    ConvertRotParamInPlace(&pymCfg, RP_EXP);
    const int nb = pymCfg.nBody;
    const int nf = pymCfg.nFiber;
    SetPymCfgChiRefToCurrentState(&pymCfg);
    MSKenv_t    env;
    InitializeMosek(&env);
    int i, j;
    FOR_0(i, pymCfg.nSimFrame) {
        StateDependents sd[nb];

        FOR_0(j, nb) {
            UpdateCurrentStateDependentValues(sd + j, pymCfg.body + j, &pymCfg, &cc);
        }

        BipedOptimizationData bod;
        GetBipedMatrixVector(&bod, sd, &pymCfg, &cc);
        //cholmod_print_sparse(bod.bipMat, "bipMat", &cc);

        //cholmod_drop(1e-16, bod.bipMat, &cc);

        //PrintEntireSparseMatrix(bod.bipMat);
        //__PRINT_VECTOR_VERT(bod.bipEta, bod.bipMat->nrow);
        double xx[bod.bipMat->ncol];
        MSKsolstae solsta;
        double cost = PymOptimize(xx, &solsta, &bod, sd, &pymCfg, &env, &cc);
        const char *solstaStr;
        if (solsta == MSK_SOL_STA_UNKNOWN) solstaStr = "***UNKNOWN***";
        else if (solsta == MSK_SOL_STA_OPTIMAL) solstaStr = "optimal";
        else if (solsta == MSK_SOL_STA_NEAR_OPTIMAL) solstaStr = "near optimal";
        else assert(0);

        //if (i%10 == 0)
            printf("Optimization iteration %5d finished. %15s (cost=%lf)\n", i, solstaStr, cost);


        printf("Results:\n");
        FOR_0(j, nb) {
            const double *chi_j_1 = xx + bod.Aci[j];
            SetRigidBodyChi_1(pymCfg.body + j, chi_j_1 );

            const RigidBodyNamed *rbn = &pymCfg.body[j].b;
            double chi_1[6], chi_0[6];
            chi_1[0] = rbn->p[0]; chi_1[1] = rbn->p[1]; chi_1[2] = rbn->p[2];
            chi_1[3] = rbn->q[0]; chi_1[4] = rbn->q[1]; chi_1[5] = rbn->q[2];
            chi_0[0] = rbn->p0[0]; chi_0[1] = rbn->p0[1]; chi_0[2] = rbn->p0[2];
            chi_0[3] = rbn->q0[0]; chi_0[4] = rbn->q0[1]; chi_0[5] = rbn->q0[2];
            //printf("%8s - ", rbn->name); __PRINT_VECTOR(chi_1, 6);
            //PRINT_VECTOR(chi_0, 6);
        }
        FOR_0(j, nf) {
            MuscleFiberNamed *mfn = &pymCfg.fiber[j].b;
            const double T_0        = xx[ bod.Aci[nb + 0] + j ];
            const double u_0        = xx[ bod.Aci[nb + 1] + j ];
            const double xrest_0    = xx[ bod.Aci[nb + 2] + j ];
            mfn->T     = T_0;
            mfn->xrest = xrest_0;
            //printf("%16s -   T = %12lf   u = %12lf   xrest = %12lf\n", mfn->name, T_0, u_0, xrest_0);

        }

        ReleaseBipedMatrixVector(&bod, &cc);
        ReleaseStateDependents(sd, &pymCfg, &cc);
    }


    CleanupMosek(&env);

    DeallocConfig(&pymCfg);

    cholmod_finish(&cc);
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
