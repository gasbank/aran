#include <math.h>
#include <assert.h>
#include <cholmod.h>
#include "PymStruct.h"
#include "Config.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "MathUtil.h"
#include "MuscleFiber.h"

void GetMuscleFiberK(double k[3], const MuscleFiber *mfx, const LPPymuscleConfig pymCfg) {
    const double h = pymCfg->h;
    const MuscleFiberNamed *mf = &mfx->b;
    const double kse_sq = mf->kse*mf->kse;
    k[0] = mf->kse * mf->b + kse_sq * h + mf->kse * h * mf->kpe;
    k[1] = -kse_sq * h;
    k[2] = kse_sq * h * mf->kpe;
}

void GetMuscleFiberEndpointPositions(double orgpos[3], double inspos[3], const int timeframe, const int mfidx, const LPStateDependents sd, const LPPymuscleConfig pymCfg) {
    const MuscleFiberNamed *mf = &pymCfg->fiber[mfidx].b;
    const StateDependents *orgSd = sd + mf->org;
    const StateDependents *insSd = sd + mf->ins;
    const double (*orgW)[4], (*insW)[4];

    if (timeframe == 0) {
        orgW = orgSd->W_0;
        insW = insSd->W_0;
    } else if (timeframe == 1) {
        orgW = orgSd->W_1;
        insW = insSd->W_1;
    } else {
        assert (0);
    }
    //printf ("mf kse kpe = %lf %lf\n", mf->kse, mf->kpe);
    AffineTransformPoint(orgpos, orgW, mf->fibb_org);
    AffineTransformPoint(inspos, insW, mf->fibb_ins);
}

double GetMuscleFiberS(const int mfidx, const LPStateDependents sd, const LPPymuscleConfig pymCfg) {
    double orgpos0[3], inspos0[3], diff0[3], x0 = 0;
    double orgpos1[3], inspos1[3], diff1[3], x1 = 0;
    GetMuscleFiberEndpointPositions(orgpos0, inspos0, 0, mfidx, sd, pymCfg);
    GetMuscleFiberEndpointPositions(orgpos1, inspos1, 1, mfidx, sd, pymCfg);
    int i;
    FOR_0(i, 3) {
        diff0[i] = orgpos0[i] - inspos0[i];
        diff1[i] = orgpos1[i] - inspos1[i];
        x0 += diff0[i]*diff0[i];
        x1 += diff1[i]*diff1[i];
    }
    x0 = sqrtf(x0);
    x1 = sqrtf(x1);
    const double h = pymCfg->h;
    const MuscleFiberNamed *mf = &pymCfg->fiber[mfidx].b;
    const double kse_sq = mf->kse * mf->kse;
    const double s = mf->kse * mf->b * mf->T + kse_sq * h * mf->kpe * x1 + kse_sq * mf->b * (x1 - x0);

    //printf("s[%d] = %lf    x0 = %lf      x1 = %lf\n", mfidx, s, x0, x1);
    return s;
}