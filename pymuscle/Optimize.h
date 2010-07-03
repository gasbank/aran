/*
 * Optimize.h
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */

void InitializeMosek(MSKenv_t *env);

void PymOptimize(const BipedOptimizationData *bod,
                 const LPStateDependents sd,
                 const LPPymuscleConfig pymCfg,
                 MSKenv_t *pEnv, cholmod_common *cc);
