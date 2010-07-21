#ifndef __PYMUSCLECONFIG_H_
#define __PYMUSCLECONFIG_H_

#include "PymJointAnchor.h"

struct _pym_config_t {
    pym_rb_t *body; unsigned int nBody;
    pym_mf_t *fiber; unsigned int nFiber;
    unsigned int nJoint;
    double h;
    double mu;
    int nSimFrame;
    double slant;

    pym_joint_anchor_t pymJa[100];
    int na;
};


void PrintRigidBody(const pym_rb_t *rb);
int FindBodyIndex(int nBody, char bodyName[nBody][128], const char *bn);
int PymDestoryConfig(pym_config_t *pymCfg);
int PymConstructConfig(const char *fnConf, pym_config_t *pymCfg);
void PymConvertRotParamInPlace(pym_config_t *pymCfg, pym_rot_param_t targetRotParam);
void PymConstructBipedEqConst(pym_biped_eqconst_t *bod, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg, cholmod_common *cc);
void PymSetPymCfgChiRefToCurrentState(pym_config_t *pymCfg);
#endif
