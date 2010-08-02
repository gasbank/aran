/*
 * Optimize.h
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */
#ifndef __OPTIMIZE_H_
#define __OPTIMIZE_H_

typedef struct _deviation_stat_entry {
    double chi_d_norm;
    int nContact;
    int bodyIdx;
} deviation_stat_entry;

void PymInitializeMosek(MSKenv_t *env);
void PymCleanupMosek(MSKenv_t *env);

double PymOptimize(double *xx,
                   MSKsolstae *_solsta, /* MOSEK solution status */
                   double *opttime,
                   const pym_biped_eqconst_t *bod,
                   const pym_rb_statedep_t *sd,
                   const pym_config_t *pymCfg,
                   MSKenv_t *pEnv, cholmod_common *cc);

int PymOptimizeFrameMove(double *pureOptTime, FILE *outputFile,
                         pym_config_t *pymCfg, FILE *dmstreams[],
                         const char **_solstaStr, double *_cost,
                         cholmod_common *cc, MSKenv_t env);
#endif
