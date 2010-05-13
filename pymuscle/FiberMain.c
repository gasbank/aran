/*
 * FiberEffectImpAll.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Fiber effect equations (all fiber)
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <libconfig.h>

#include "TripletMatrix.h"
#include "FiberEffectImpAll.h"
#include "umfpack.h"

#define PYM_MIN(a,b) (a)>(b)?(b):(a)
#define PYM_MAX(a,b) (b)>(a)?(b):(a)

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

void QuatToEuler(double eul[3], double q[4])
{
    eul[0] = atan2(2*(q[0]*q[1]+q[2]*q[3]), 1.-2*(q[1]*q[1]+q[2]*q[2]));
    /* asin(x) need x in range of [-1.,1.] */
    double clamped;
    clamped = PYM_MIN(1., 2*(q[0]*q[2]-q[3]*q[1]));
    clamped = PYM_MAX(-1., clamped);
    eul[1] = asin(clamped);
    eul[2] = atan2(2*(q[0]*q[3]+q[1]*q[2]), 1.-2*(q[2]*q[2]+q[3]*q[3]));
}
void BoxInertiaTensorFromMassAndSize(double Ixyz[3], double m, double sx, double sy, double sz)
{
    Ixyz[0] = m*(sy*sy + sz*sz)/12.;
    Ixyz[1] = m*(sx*sx + sz*sz)/12.;
    Ixyz[2] = m*(sx*sx + sy*sy)/12.;
}

int FindBodyIndex(int nBody, char bodyName[nBody][128], const char *bn)
{
    int i;
    for (i = 0; i < nBody; ++i)
    {
        if (strncmp(bodyName[i], bn, 128) == 0)
            return i;
    }
    return -1;
}

double NormalizeVector(int dim, double v[dim])
{
    double len = 0;
    int i;
    for (i = 0; i < dim; ++i)
        len += v[i]*v[i];
    len = sqrt(len);
    for (i = 0; i < dim; ++i)
        v[i] /= len;
    return len;
}

int main()
{
    int j, k;
    config_t conf;
    config_init(&conf);
    config_set_auto_convert(&conf, 1);
    int confret = config_read_file(&conf, "pymuscle.conf");
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
    printf("Version: %s\n", ver);

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
        printf("Body %d name: %s\n", j, bName);
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
        printf("Muscle %d name: %s\n", j, mName);
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

    config_destroy(&conf);

    /***************************************************************/

    unsigned int curFrame;
    const unsigned int matSize = 14*nBody + nMuscle;
    FILE *sr = fopen("simresult.txt", "w");
    assert(sr);
    fprintf(sr, "# curFrame p1[x] p1[y] p1[z] p1[e1] p1[e2] p1[e3] ... pn[x] pn[y] pn[z] pn[e1] pn[e2] pn[e3] T1 ... Tm\n");
    for (curFrame = 0; curFrame < simFrame; ++curFrame)
    {
        int writeSample = (curFrame % plotSamplingRate == 0);

        TripletMatrix *dfdY_R[nBody]; /* be allocated by the following function */
        TripletMatrix *dfdY_Q[nMuscle]; /* be allocated by the following function */
        double f[nBody*14 + nMuscle];

        if (writeSample)
            fprintf(sr, "%d", curFrame);
        for (j = 0; j < nBody; ++j)
        {
            /* Renormalize quaternions for each body */
            NormalizeVector(4, &body[j][3]);

            /* Write the angular and linear position to the file. (for plotting) */
            /* Euler angles instead of quaternions used (human friendly) */
            double bjEul[3];
            QuatToEuler(bjEul, &body[j][3]);
            if (writeSample)
                fprintf(sr, " %lf %lf %lf %lf %lf %lf",
                            body[j][0], body[j][1], body[j][2],
                            bjEul[0], bjEul[1], bjEul[2]);
        }
        for (j = 0; j < nMuscle; ++j)
            if (writeSample)
                fprintf(sr, " %lf", muscle[j][4]);
        if (writeSample)
            fprintf(sr, "\n");


        ImpAll(nBody, nMuscle, dfdY_R, dfdY_Q, f, body, extForce, muscle, musclePair, h);


        if (curFrame % 100 == 0)
        {
            printf("  - Simulating %5d frame... (%6.2f %%)\n", curFrame, (float)curFrame/simFrame*100);
        }
        /*
        for (j = 0; j < dfdY_Q[0]->nz; ++j)
        {
            printf("dfdY_Q[0] (%d, %d) = %lf\n", dfdY_Q[0]->Ti[j], dfdY_Q[0]->Tj[j], dfdY_Q[0]->Tx[j]);
        }
        */



        TripletMatrix *dfdY_RQ_merged[2] = { 0, 0 };
        dfdY_RQ_merged[0] = tm_merge(nBody, dfdY_R);
        dfdY_RQ_merged[1] = tm_merge(nMuscle, dfdY_Q);
        /* dfdY_R and dfdY_Q are unnecessary */
        for (j = 0; j < nBody; ++j)
            dfdY_R[j] = tm_free(dfdY_R[j]);
        for (j = 0; j < nMuscle; ++j)
            dfdY_Q[j] = tm_free(dfdY_Q[j]);
        TripletMatrix *dfdY_merged;
        if (nMuscle)
            dfdY_merged = tm_merge(2, dfdY_RQ_merged);
        else
            dfdY_merged = tm_merge(1, dfdY_RQ_merged);
        /* dfdY_RQ_merged is unnecessary */
        dfdY_RQ_merged[0] = tm_free(dfdY_RQ_merged[0]);
        dfdY_RQ_merged[1] = tm_free(dfdY_RQ_merged[1]);

        int Ap[dfdY_merged->n_col + 1];
        int Ai[dfdY_merged->nz];
        int status;
        int *Map = 0;
        double Ax[dfdY_merged->nz];
        status = umfpack_di_triplet_to_col (dfdY_merged->n_row,
                                            dfdY_merged->n_col,
                                            dfdY_merged->nz,
                                            dfdY_merged->Ti,
                                            dfdY_merged->Tj,
                                            dfdY_merged->Tx,
                                            Ap, Ai, Ax, Map);
        assert(status == UMFPACK_OK);
        /* Ap[n_col] or Ap[matSize] is the number of nonzeros in the sparse matrix. */
        //printf("dfdY_merged nz = %d\n", Ap[matSize]);

        double Control [UMFPACK_CONTROL];
        Control[UMFPACK_PRL] = 5;
        /*
        umfpack_di_report_matrix(matSize, matSize, Ap, Ai, Ax, 1, Control);
        */


        /*
         * Now we have f, dfdY.
         * Let's start implicit integration step.
         * Build the following matrix.
         *
         *               (identity matrix)        d f
         * tripletA  =  -------------------  -  ------
         *                      h                 d Y
         */
        int tripletA_nzMax = matSize + Ap[matSize];
        TripletMatrix *tripletA = tm_allocate(matSize, matSize, tripletA_nzMax);
        for (j = 0; j < matSize; ++j)
        {
            tm_add_entry(tripletA, j, j, 1./h);
        }
        int Tj[Ap[matSize]];
        status = umfpack_di_col_to_triplet (matSize, Ap, Tj) ;
        assert(status == UMFPACK_OK);
        /* We can access dfdY by using Ai (row indices), Tj (column indices) and Ax (numerical values). */
        /* Number of nonzero values is Ap[matSize]. */
        for (j = 0; j < Ap[matSize]; ++j)
        {
            tm_add_entry(tripletA, Ai[j], Tj[j], - Ax[j]);
        }
        /* tripletA is constructed. Convert it to column-compressed format. */
        int tripAp[matSize + 1];
        int tripAi[tripletA_nzMax];
        int *tripMap = 0;
        double tripAx[tripletA_nzMax];
        status = umfpack_di_triplet_to_col (tripletA->n_row,
                                            tripletA->n_col,
                                            tripletA->nz,
                                            tripletA->Ti,
                                            tripletA->Tj,
                                            tripletA->Tx,
                                            tripAp, tripAi, tripAx, tripMap);
        assert(status == UMFPACK_OK);
        //printf("tripletA nonzeros = %d\n", tripAp[matSize]);

        //umfpack_di_report_matrix(matSize, matSize, tripAp, tripAi, tripAx, 1, Control);

        tripletA = tm_free(tripletA);
        dfdY_merged = tm_free(dfdY_merged);

        /*
         * Linear system solving
         *
         * A x = b
         *
         * A := tripletA
         * x := delta Y
         * b := f(Y0)
         */
        double *null = (double *) NULL ;

        void *Symbolic, *Numeric ;
        int n = matSize;
        double *b = f;
        double x[matSize];
        (void) umfpack_di_symbolic (n, n, tripAp, tripAi, tripAx, &Symbolic, null, null) ;
        (void) umfpack_di_numeric (tripAp, tripAi, tripAx, Symbolic, &Numeric, null, null) ;
        umfpack_di_free_symbolic (&Symbolic) ;
        (void) umfpack_di_solve (UMFPACK_A, tripAp, tripAi, tripAx, x, b, Numeric, null, null) ;
        umfpack_di_free_numeric (&Numeric) ;
        //for (j = 0 ; j < 3 ; j++) printf ("x [%d] = %g  ", j, x [j]) ;

        /* Since x := deltaY, we should update our state vector accordingly. */
        for (k = 0; k < nBody; ++k)
        {
            /* Update p(3), q(4), pd(3), qd(4) for each body */
            for (j = 0; j < 14; ++j)
            {
                body[k][j] += x[k*14 + j];
            }
        }
        for (k = 0; k < nMuscle; ++k)
        {
            muscle[k][4 /* watch out! */] += x[nBody*14 + k];
        }
    }
    fclose(sr);
	return (0);
}

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
