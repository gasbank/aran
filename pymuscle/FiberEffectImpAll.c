/*
 * FiberEffectImpAll.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Fiber effect equations (all fiber)
 */
#include "PymPch.h"
#include "TypeDefs.h"
#include "OneRbImp.h"
#include "FiberEffectImp.h"
#include "TripletMatrix.h"

#define ORIGIN    (0)
#define INSERTION (1)

double mac_FibDiffW(double fibDiffW[3], const double input[18+18+12]);

int ImpAll(
           const unsigned int nBody,
                       const unsigned int nMuscle,
                       TripletMatrix *dfdY_R[nBody],
                       TripletMatrix *dfdY_Q[nMuscle],
                       double f[nBody*14 + nMuscle],
                       double body[nBody][18],
                       double extForce[nBody][6],
                       double muscle[nMuscle][12],
                       unsigned int musclePair[nMuscle][2],
                       const double h)
{
	const unsigned int nY = nBody*14 + nMuscle;
    memset(f, 0, sizeof(double)*nY); /* f(Y0) */

	int j;

    #pragma omp parallel for schedule(dynamic) shared(dfdY_R, body, extForce) private(j)
	for (j = 0; j < nBody; ++j)
	{
        int bClearVariable = 1;
        double       yd_R[14];
        /* Sparse matrix structure of 'dyd_RdY' */
        unsigned int dyd_RdY_keys[14*14][2];
        double       dyd_RdY_values[14*14];

        int dyd_RdY_count = OneRbImp(body[j], extForce[j], bClearVariable,
                                     yd_R, dyd_RdY_keys, dyd_RdY_values);
        assert(dyd_RdY_count >= 0);
        int k;
        for (k = 0; k < 14; ++k)
        {
            f[j*14 + k] = yd_R[k];
        }

        dfdY_R[j] = tm_allocate(nY, nY, dyd_RdY_count);
        for (k = 0; k < dyd_RdY_count; ++k)
        {
            int r = j*14 + dyd_RdY_keys[k][0];
            int c = j*14 + dyd_RdY_keys[k][1];
            tm_add_entry(dfdY_R[j], r, c, dyd_RdY_values[k]);
        }
        //printf("dfdY_R[%d] nonzeros = %d\n", j, dfdY_R[j]->nz);
	}


    #pragma omp parallel for schedule(dynamic) shared(f, dfdY_Q, body, muscle, musclePair) private(j)
	for (j = 0; j < nMuscle; ++j)
	{
		const double       *mj = muscle[j];
		const unsigned int boIdx = musclePair[j][ORIGIN];
		const unsigned int biIdx = musclePair[j][INSERTION];
		const double       *bo = body[ boIdx ];
		const double       *bi = body[ biIdx ];

		assert(muscle[j][5 /* A */] == 0) ;

        /*
         * The structure of the vector 'input'
         *
         *   - BODY_ORG RELATED VARIABLES (18 x 1 vector)
         *   - BODY_INS RELATED VARIABLES (18 x 1 vector)
         *   - FIBER RELATED VARIABLES (12 x 1 vector)
         */
		double       input[18+18+12];
		int          bClearVariable = 1;
		double       dTddy_orgins[2][14];
		double       yd_Q_orgins[2][14];
		double       dyd_Q_orginsdT[2][14];
		double       Td;
		double       dTddT;
		/* Sparse matrix structure of 'dyd_Q_orginsdy_orgins' */
		unsigned int dyd_Q_orginsdy_orgins_keys[(14*4)*14][2];
		double       dyd_Q_orginsdy_orgins_values[(14*4)*14];

		memcpy(input      , bo, sizeof(double)*18);
		memcpy(input+18   , bi, sizeof(double)*18);
		memcpy(input+18+18, mj, sizeof(double)*12);

        double fibDiffW[3];
        mac_FibDiffW(fibDiffW, input);
        const double len = sqrt(fibDiffW[0]*fibDiffW[0]
                                +fibDiffW[1]*fibDiffW[1]
                                +fibDiffW[2]*fibDiffW[2]);
        int bDegenerated = 0;
        //printf("Muscle fiber #%d len = %lg.\n", j, len);
		if (len < 1e-8)
		{
		    printf("   - Muscle fiber #%d degenerated due to its length = %lg.\n", j, len);
		    bDegenerated = 1;
		}

		int dyd_Q_orginsdy_orgins_count = 0;
		if (bDegenerated == 0) {
		    dyd_Q_orginsdy_orgins_count = FiberEffectImp(input,
		                                                 bClearVariable,
		                                                 /* Y_i part -----------------------------*/
		                                                 yd_Q_orgins, /* 14 x 2 */
		                                                 &Td, /* a scalar */
		                                                 /* dfdY_i part ------------------------- */
		                                                 dTddy_orgins, /* 14 x 2 */
		                                                 dyd_Q_orginsdT, /* 14 x 2 */
		                                                 &dTddT, /* a scalar */
		                                                 /* nnz (maximum 14x14x4) */
		                                                 dyd_Q_orginsdy_orgins_keys,
		                                                 dyd_Q_orginsdy_orgins_values);
		} else {
		    /* Degenerate case */
		    memset(yd_Q_orgins, 0, sizeof(double)*14*2);

		    /* Td := KSE/b*(KPE*(-xrest)-(1+KPE/KSE)*T) */
		    const double KSE   = muscle[j][0];
		    const double KPE   = muscle[j][1];
		    const double b     = muscle[j][2];
		    const double xrest = muscle[j][3];
		    const double T     = muscle[j][4];
		    const double A     = muscle[j][5];
		    Td = KSE/b*(KPE*(-xrest)-(1+KPE/KSE)*T+A);
		    memset(dTddy_orgins, 0, sizeof(double)*14*2);
		    memset(dyd_Q_orginsdT, 0, sizeof(double)*14*2);
		    dTddT = (KSE+KPE)/b;
		    dyd_Q_orginsdy_orgins_count = 0;

		    /* DEBUG */
		    printf("Degenerate fiber.\n");
		    exit(-1);
		}

        assert(dyd_Q_orginsdy_orgins_count >= 0);
        int k;
        #pragma omp critical
        {
            for (k = 0; k < 14; ++k)
            {
                f[boIdx*14 + k] += yd_Q_orgins[ORIGIN][k];
                f[biIdx*14 + k] += yd_Q_orgins[INSERTION][k];
                f[nBody*14 + j] = Td;
            }
        }


        const unsigned int dfdY_j_maxNnz = dyd_Q_orginsdy_orgins_count+14+14+14+14+1;
        dfdY_Q[j] = tm_allocate(nY, nY, dfdY_j_maxNnz);

        /*
        # (1) Starting from the easiest...
        #
        #   .
        # d T
        # ---
        # d T
        #
        */
        tm_add_entry(dfdY_Q[j], nBody*14 + j, nBody*14 + j, dTddT);

        /*
        # (2)(3)
        #
        #   .
        # d T
        # ------
        # d y
        #    {org,ins}
        #
        */
        for (k = 0; k < 14; ++k)
        {
            tm_add_entry(dfdY_Q[j], nBody*14 + j, boIdx*14 + k, dTddy_orgins[ORIGIN][k]);
            tm_add_entry(dfdY_Q[j], nBody*14 + j, biIdx*14 + k, dTddy_orgins[INSERTION][k]);
        }
        /*
        # (4)(5)
        #
        #   . {org,ins}
        # d y
        #     Q
        # ------
        # d T
        #
        #
        # Seem to be this kind of assignment has a problem:
        #
        # dYd_QidY[orgIdx*14:(orgIdx+1)*14, nBody*14 + mIdx ] = dyd_Q_orgdT
        #
        */
        for (k = 7 /* STARTING FROM 7! */; k < 14; ++k)
        {
            tm_add_entry(dfdY_Q[j], boIdx*14 + k, nBody*14 + j, dyd_Q_orginsdT[ORIGIN][k]);
            tm_add_entry(dfdY_Q[j], biIdx*14 + k, nBody*14 + j, dyd_Q_orginsdT[INSERTION][k]);
        }
        /*
        # (6)(7)(8)(9)
        #
        #    . {org/ins}
        #  d y
        #      Q
        # ---------
        #  d y
        #     org
        #
        #
        #    . {org/ins}
        #  d y
        #      Q
        # ---------
        #  d y
        #     ins
        #
        */
        for (k = 0; k < dyd_Q_orginsdy_orgins_count; ++k)
        {
            /* Triplet */
            unsigned int r = dyd_Q_orginsdy_orgins_keys[k][0];
            unsigned int c = dyd_Q_orginsdy_orgins_keys[k][1];
            double       v = dyd_Q_orginsdy_orgins_values[k];

            if (r < 14)
            {
                r = boIdx*14 + r;
                c = boIdx*14 + c;
            }
            else if (r < 28)
            {
                r = boIdx*14 + r - 14;
                c = biIdx*14 + c;
            }
            else if (r < 42)
            {
                r = biIdx*14 + r - 28;
                c = boIdx*14 + c;
            }
            else
            {
                r = biIdx*14 + r - 42;
                c = biIdx*14 + c;
            }

            tm_add_entry(dfdY_Q[j], r, c, v);
        }
        //printf("dfdY_Q[%d] nonzeros = %d\n", j, dfdY_Q[j]->nz);
    }


	return 0;
}
