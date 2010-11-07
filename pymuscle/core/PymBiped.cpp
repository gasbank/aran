#include "PymPch.h"
#include "PymStruct.h"
#include "PymBiped.h"

void PymDestroyBipedEqconst(pym_biped_eqconst_t *bipEq, cholmod_common *cc) {
    cholmod_free_sparse(&bipEq->bipMat, cc); bipEq->bipMat = 0;
    free(bipEq->bipEta); bipEq->bipEta = 0;
    free(bipEq->Ari); bipEq->Ari = 0;
    free(bipEq->Aci); bipEq->Aci = 0;
    free(bipEq->Airi); bipEq->Airi = 0;
    free(bipEq->Aici); bipEq->Aici = 0;
}
