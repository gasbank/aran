#ifndef __PYMUSCLECONFIG_H_
#define __PYMUSCLECONFIG_H_

#include "PymJointAnchor.h"
#include "PymConfig.h"

enum pym_rot_param_t;
struct pym_biped_eqconst_t;

PYMCORE_API int PymConstructConfig(const char *fnConf, pym_config_t *pymCfg, FILE *verbosestream);
PYMCORE_API void PymConstructBipedEqConst(pym_biped_eqconst_t *bod, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg, cholmod_common *cc);
PYMCORE_API void PymSetPymCfgChiRefToCurrentState(pym_config_t *pymCfg);
PYMCORE_API void PymConvertRotParamInPlace(pym_config_t *pymCfg, pym_rot_param_t targetRotParam);
PYMCORE_API int PymDestoryConfig(pym_config_t *pymCfg);

void PrintRigidBody(const pym_rb_t *rb);
int pym_check_coincided_fibers(int *mf_indices, const pym_rb_statedep_t *sd_all, const pym_config_t *pymCfg);

#endif
