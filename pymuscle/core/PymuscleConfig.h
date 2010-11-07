#ifndef __PYMUSCLECONFIG_H_
#define __PYMUSCLECONFIG_H_

#include "PymJointAnchor.h"
#include "PymConfig.h"

void PrintRigidBody(const pym_rb_t *rb);
int PymDestoryConfig(pym_config_t *pymCfg);
int PymConstructConfig(const char *fnConf, pym_config_t *pymCfg, FILE *verbosestream);
void PymConvertRotParamInPlace(pym_config_t *pymCfg, pym_rot_param_t targetRotParam);
void PymConstructBipedEqConst(pym_biped_eqconst_t *bod, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg, cholmod_common *cc);
void PymSetPymCfgChiRefToCurrentState(pym_config_t *pymCfg);
#endif
