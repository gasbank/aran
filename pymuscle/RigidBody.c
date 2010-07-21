#include "PymPch.h"
#include "PymStruct.h"
#include "Config.h"
#include "RigidBody.h"
#include "PymuscleConfig.h"

void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+3], const pym_config_t *const pymCfg) {
    pym_rb_named_t *rbn = &rb->b;
    assert(rbn->rotParam == RP_EXP);
    int i, j;
    FOR_0(i, 3) {
        rbn->p0[i] = rbn->p[i];
        rbn->p[i]  = Chi_1[i];
    }
    FOR_0(i, 3) {
        rbn->q0[i] = rbn->q[i];
        rbn->q[i]  = Chi_1[i+3];
    }
    /* update discrete velocity based on current and previous step */
    FOR_0(j, 3) {
        rbn->pd[j] = (rbn->p[j] - rbn->p0[j]) / pymCfg->h;
        rbn->qd[j] = (rbn->q[j] - rbn->q0[j]) / pymCfg->h;
    }
}
