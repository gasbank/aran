#ifndef __MUSCLEFIBER_H_
#define __MUSCLEFIBER_H_

enum pym_muscle_type_e {
  PMT_UNKNOWN,
  PMT_ACTUATED_MUSCLE,
  PMT_LIGAMENT,
  PMT_JOINT_MUSCLE,
};

struct pym_mf_named_t {
  double kse, kpe, b, xrest, T, A;
  double fibb_org[3];
  double fibb_ins[3];
  /******************************************************************/
  /* Data declared so far should not be modified for compatibility  */
  /******************************************************************/
  int org;
  int ins;
  double xrest_lower, xrest_upper;
  char name[128];
  pym_muscle_type_e mType;
  bool degenerated;
};

union pym_mf_t
{
    double a[1+1+1+1+1+1+3+3];
    pym_mf_named_t b;
};

void GetMuscleFiberK(double k[3], const pym_mf_t *mfx, const pym_config_t *pymCfg);
void GetMuscleFiberEndpointPositions(double orgpos[3], double inspos[3], const int timeframe, const int mfidx, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg);
double GetMuscleFiberS(const int mfidx, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg);

#endif
