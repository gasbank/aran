#ifndef __MUSCLEFIBER_H_
#define __MUSCLEFIBER_H_

typedef struct _MuscleFiberNamed {
    double kse, kpe, b, xrest, T, A;
    double fibb_org[3];
    double fibb_ins[3];
    int org;
    int ins;
    double xrest_lower, xrest_upper;
    char name[128];
} MuscleFiberNamed;

typedef union _MuscleFiber
{
    double a[1+1+1+1+1+1+3+3];
    MuscleFiberNamed b;
} MuscleFiber;


void GetMuscleFiberK(double k[3], const MuscleFiber *mfx, const LPPymuscleConfig pymCfg);
void GetMuscleFiberEndpointPositions(double orgpos[3], double inspos[3], const int timeframe, const int mfidx, const LPStateDependents sd, const LPPymuscleConfig pymCfg);
double GetMuscleFiberS(const int mfidx, const LPStateDependents sd, const LPPymuscleConfig pymCfg);

#endif
