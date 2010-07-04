/*
 * Optimize.h
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */

void InitializeMosek(MSKenv_t *env);
void CleanupMosek(MSKenv_t *env);

double PymOptimize(double *xx,
                   MSKsolstae *_solsta, /* MOSEK solution status */
                 const BipedOptimizationData *bod,
                 const LPStateDependents sd,
                 const LPPymuscleConfig pymCfg,
                 MSKenv_t *pEnv, cholmod_common *cc);
