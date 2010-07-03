#ifndef __BIPED_H_
#define __BIPED_H_

typedef struct _BipedOptimizationData {
    cholmod_sparse *bipMat;
    double *bipEta; /* dynamically allocated array */
    int *Ari; /* dynamic array of size (1+Asubrows) */
    int *Aci; /* dynamic array of size (1+Asubcols) */
    int Asubrows;
    int Asubcols;
} BipedOptimizationData;

void ReleaseBipedMatrixVector(BipedOptimizationData *bod, cholmod_common *cc);

#endif
