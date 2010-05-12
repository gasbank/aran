/*
 * FiberEffectImpAll.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Fiber effect equations (all fiber)
 */
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "TypeDefs.h"
#include "OneRbImp.h"
#include "FiberEffectImp.h"
#include "TripletMatrix.h"

#define ORIGIN    (0)
#define INSERTION (1)

int ImpAll( const unsigned int nBody,
                       const unsigned int nMuscle,
                       TripletMatrix *dfdY_R[nBody],
                       TripletMatrix *dfdY_Q[nMuscle],
                       double body[nBody][18],
                       double extForce[nBody][6],
                       double muscle[nMuscle][12],
                       unsigned int musclePair[nMuscle][2])
{
	int matSize = nBody*14 + nMuscle;

	int j, k;

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

        dfdY_R[j] = tm_allocate(matSize, matSize, dyd_RdY_count);
        for (k = 0; k < dyd_RdY_count; ++k)
        {
            int r = j*14 + dyd_RdY_keys[k][0];
            int c = j*14 + dyd_RdY_keys[k][1];
            tm_add_entry(dfdY_R[j], r, c, dyd_RdY_values[k]);
        }
        printf("dfdY_R[%d] nonzeros = %d\n", j, dfdY_R[j]->nz);
	}


	for (j = 0; j < nMuscle; ++j)
	{
		const double       *mj = muscle[j];
		const unsigned int boIdx = musclePair[j][ORIGIN];
		const unsigned int biIdx = musclePair[j][INSERTION];
		const double       *bo = body[ boIdx ];
		const double       *bi = body[ biIdx ];

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


		int dyd_Q_orginsdy_orgins_count = FiberEffectImp(
                                 input,
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
        assert(dyd_Q_orginsdy_orgins_count >= 0);
        const int dfdY_j_maxNnz = dyd_Q_orginsdy_orgins_count+14+14+14+14+1;
        dfdY_Q[j] = tm_allocate(matSize, matSize, dfdY_j_maxNnz);

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
        for (k = 0; k < 14; ++k)
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
        printf("dfdY_Q[%d] nonzeros = %d\n", j, dfdY_Q[j]->nz);
    }


	return 0;
}
