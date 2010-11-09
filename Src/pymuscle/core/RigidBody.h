#ifndef __RIGIDBODY_H_
#define __RIGIDBODY_H_

#include "PymRbPod.h"

PYMCORE_API void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+3], const pym_config_t *const pymCfg);
PYMCORE_API int PymCheckRotParam(pym_config_t *pymCfg);

void GetAMatrix(cholmod_triplet **AMatrix, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void GetEta(double **_eta, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void PymReparameterizeRotParam(pym_rb_named_t *rbn, const pym_config_t *pymCfg);

#endif
