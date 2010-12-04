#include "PymCorePch.h"
#include "PymStruct.h"
#include "Config.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "MathUtil.h"
#include "MuscleFiber.h"

void GetMuscleFiberK(double k[3], const pym_mf_t *mfx, const pym_config_t *pymCfg) {
    const double h = pymCfg->h;
    const pym_mf_named_t *mf = &mfx->b;
    assert(mf->kse != 0);
    k[0] = mf->b + mf->kse * h + h * mf->kpe;
    k[1] = -mf->kse * h;
    k[2] = mf->kse * h * mf->kpe;
}

void GetMuscleFiberEndpointPositions(double orgpos[3], double inspos[3], const int timeframe, const int mfidx, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg) {
    const pym_mf_named_t *mf = &pymCfg->fiber[mfidx].b;
    const pym_rb_statedep_t *orgSd = sd + mf->org;
    const pym_rb_statedep_t *insSd = sd + mf->ins;
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

double GetMuscleFiberS(const int mfidx, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg) {
    double orgpos0[3], inspos0[3], diff0[3], x0 = 0;
    double orgpos1[3], inspos1[3], diff1[3], x1 = 0;
	int i;
	const double h = pymCfg->h;
	const pym_mf_named_t *mf = &pymCfg->fiber[mfidx].b;
	double s = 0;
	GetMuscleFiberEndpointPositions(orgpos0, inspos0, 0, mfidx, sd, pymCfg);
    GetMuscleFiberEndpointPositions(orgpos1, inspos1, 1, mfidx, sd, pymCfg);
    FOR_0(i, 3) {
        diff0[i] = orgpos0[i] - inspos0[i];
        diff1[i] = orgpos1[i] - inspos1[i];
        x0 += diff0[i]*diff0[i];
        x1 += diff1[i]*diff1[i];
    }
    x0 = sqrt(x0);
    x1 = sqrt(x1);
    s = mf->b * mf->T + mf->kse * h * mf->kpe * x1 + mf->kse * mf->b * (x1 - x0);
    //printf("s[%3d] %20s = % -15.8e    x0 = %-15.8e    x1 = %-15.8e    T0 = % -15.8e\n", mfidx, mf->name, s, x0, x1, mf->T);
    return s;
}
