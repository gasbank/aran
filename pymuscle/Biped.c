#include <cholmod.h>
#include "Biped.h"


void ReleaseBipedMatrixVector(BipedOptimizationData *bod, cholmod_common *cc) {
    cholmod_free_sparse(&bod->bipMat, cc);
    bod->bipMat = 0;
    free(bod->bipEta);
    bod->bipEta = 0;
}
