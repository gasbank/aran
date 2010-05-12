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
#include "TripletMatrix.h"
#include "FiberEffectImpAll.h"
#include "umfpack.h"

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

int main()
{
	const unsigned int nBody = 2;
	const unsigned int nMuscle = 1;
	double body[nBody][18];
	double extForce[nBody][6];
	double muscle[nMuscle][12];
	unsigned int musclePair[nMuscle][2];
	int j;
	for (j = 0; j < nMuscle; ++j)
	{
		musclePair[j][0] = 0;
		musclePair[j][1] = 1;
		LoadDoublesFromFile(12, muscle[j], "muscle0.txt");
	}

	LoadDoublesFromFile(18, body[0], "body0.txt");
	LoadDoublesFromFile(18, body[1], "body1.txt");

	memset(extForce, 0, sizeof(extForce));

    TripletMatrix *dfdY_R[nBody]; /* be allocated by the following function */
    TripletMatrix *dfdY_Q[nMuscle]; /* be allocated by the following function */
	ImpAll(nBody, nMuscle, dfdY_R, dfdY_Q, body, extForce, muscle, musclePair);

    /*
	for (j = 0; j < dfdY_Q[0]->nz; ++j)
	{
	    printf("dfdY_Q[0] (%d, %d) = %lf\n", dfdY_Q[0]->Ti[j], dfdY_Q[0]->Tj[j], dfdY_Q[0]->Tx[j]);
	}
	*/

	const unsigned int matSize = 14*nBody + nMuscle;

    TripletMatrix *dfdY_RQ_merged[2];
    dfdY_RQ_merged[0] = tm_merge(nBody, dfdY_R);
    dfdY_RQ_merged[1] = tm_merge(nMuscle, dfdY_Q);
    /* dfdY_R and dfdY_Q are unnecessary */
    for (j = 0; j < nBody; ++j)
        dfdY_R[j] = tm_free(dfdY_R[j]);
    for (j = 0; j < nMuscle; ++j)
        dfdY_Q[j] = tm_free(dfdY_Q[j]);
    TripletMatrix *dfdY_merged;
    dfdY_merged = tm_merge(2, dfdY_RQ_merged);
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

    printf("dfdY_merged nz = %d\n", Ap[dfdY_merged->n_col]);
    double Control [UMFPACK_CONTROL];
    Control[UMFPACK_PRL] = 5;
    umfpack_di_report_matrix(matSize, matSize, Ap, Ai, Ax, 1, Control);

    dfdY_merged = tm_free(dfdY_merged);

	return 0;
}
