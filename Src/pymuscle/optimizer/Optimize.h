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

/* MOSEK dependent optimization data structure */
typedef struct _pym_opt_t {
  /*** Inputs ***/
  /* lo <= Ax <= hi constraints */
  MSKboundkeye *bkc;
  double *blc;
  double *buc;
  /* lo <= x <= hi constraints */
  MSKboundkeye *bkx;
  double *blx;
  double *bux;
  /* cost function coefficients */
  double *c;
  /*** Outputs ***/
  double *xx;                        /* Preallocated solution vector space
                                          (size = bod->bipMat->ncol) */
  MSKsolstae _solsta;                /* MOSEK solution status */
  double opttime;
  double cost;
  /* Biped-wide state-dependent values */
  int *nplist;      /* Stores # of CPs for each RB (size=nb) */
  int nptotal;      /* Total # of CPs (sum of nplist[0..nb-1]) */
  const pym_biped_eqconst_t *bod;
  const pym_rb_statedep_t *sd;
  const pym_config_t *pymCfg;
  MSKenv_t *pEnv;
  cholmod_common *cc;
} pym_opt_t;


PYMOPTIMIZER_API void PymInitializeMosek(MSKenv_t *env);
PYMOPTIMIZER_API void PymCleanupMosek(MSKenv_t *env);

void PymOptimize(pym_opt_t *pymOpt);
PYMOPTIMIZER_API int PymOptimizeFrameMove(double *pureOptTime, FILE *outputFile,
                         pym_config_t *pymCfg, pym_rb_statedep_t *sd,
                         FILE *dmstreams[],
                         const char **_solstaStr, double *_cost,
                         cholmod_common *cc, MSKenv_t env);
#endif
