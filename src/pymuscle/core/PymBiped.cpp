#include "PymCorePch.h"
#include "PymStruct.h"
#include "PymBiped.h"
#include "ConvexHullCapi.h"
#include "RigidBody.h"
#include "PymConfig.h"

void pym_reset_com0( pym_config_t *pymCfg )
{
  double com[3] = {0,};
  double tot_mass = 0;
  for (int i = 0; i < pymCfg->nBody; ++i) {
    for (int j = 0; j < 3; ++j) {
      com[j] += pymCfg->body[i].b.m * (pymCfg->body[i].b.p[j] - pymCfg->h * pymCfg->body[i].b.pd[j]);
    }
    tot_mass += pymCfg->body[i].b.m;
  }
  for (int i = 0; i < 3; ++i) {
    com[i] /= tot_mass;
  }
  memcpy(pymCfg->bipCom0, com, sizeof(double)*3);
}

void pym_update_com( pym_config_t *pymCfg )
{
  double com[3] = {0,};
  double tot_mass = 0;
  for (int i = 0; i < pymCfg->nBody; ++i) {
    for (int j = 0; j < 3; ++j) {
      com[j] += pymCfg->body[i].b.m * pymCfg->body[i].b.p[j];
    }
    tot_mass += pymCfg->body[i].b.m;
  }
  for (int i = 0; i < 3; ++i) {
    com[i] /= tot_mass;
  }
  memcpy(pymCfg->bipCom0, pymCfg->bipCom, sizeof(double)*3);
  memcpy(pymCfg->bipCom, com, sizeof(double)*3);
}

void PymDestroyBipedEqconst(pym_biped_eqconst_t *bipEq, cholmod_common *cc) {
    cholmod_free_sparse(&bipEq->bipMat, cc); bipEq->bipMat = 0;
    free(bipEq->bipEta); bipEq->bipEta = 0;
    free(bipEq->Airi); bipEq->Airi = 0;
    free(bipEq->Aici); bipEq->Aici = 0;
}
