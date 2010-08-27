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

typedef struct _pym_opt_t {
    double *xx;                        /* Preallocated solution vector space
                                          (size = bod->bipMat->ncol) */
    MSKsolstae _solsta;                /* MOSEK solution status */
    double opttime;
    double cost;
    const pym_biped_eqconst_t *const bod;
    const pym_rb_statedep_t *const sd;
    const pym_config_t *const pymCfg;
    MSKenv_t *pEnv;
    cholmod_common *cc;
} pym_opt_t;

void PymInitializeMosek(MSKenv_t *env);
void PymCleanupMosek(MSKenv_t *env);

pym_opt_t PymNewOptimization();
void PymDelOptimization(pym_opt_t *pymOpt);
void PymOptimize(pym_opt_t *pymOpt);
int PymOptimizeFrameMove(double *pureOptTime, FILE *outputFile,
                         pym_config_t *pymCfg, FILE *dmstreams[],
                         const char **_solstaStr, double *_cost,
                         cholmod_common *cc, MSKenv_t env);
#endif
