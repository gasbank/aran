/*
 * Optimize.h
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */

void PymInitializeMosek(MSKenv_t *env);
void PymCleanupMosek(MSKenv_t *env);

double PymOptimize(double *xx,
                   MSKsolstae *_solsta, /* MOSEK solution status */
                 const pym_biped_eqconst_t *bod,
                 const pym_rb_statedep_t *sd,
                 const pym_config_t *pymCfg,
                 MSKenv_t *pEnv, cholmod_common *cc);
