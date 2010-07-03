#ifndef __PYMUSCLECONFIG_H_
#define __PYMUSCLECONFIG_H_

typedef struct _PymuscleConfig {
    RigidBody *body; unsigned int nBody;
    MuscleFiber *fiber; unsigned int nFiber;
    double h;
    double mu;
} PymuscleConfig;

void PrintRigidBody(const RigidBody *rb);
int FindBodyIndex(int nBody, char bodyName[nBody][128], const char *bn);
int DeallocConfig(PymuscleConfig *pymCfg);
int AllocConfig(const char *fnConf, PymuscleConfig *pymCfg);
void ConvertRotParamInPlace(PymuscleConfig *pymCfg, RotationParameterization targetRotParam);
void GetBipedMatrixVector(BipedOptimizationData *bod, LPStateDependents sd, const LPPymuscleConfig pymCfg, cholmod_common *cc);
void SetPymCfgChiRefToCurrentState(LPPymuscleConfig pymCfg);
#endif
