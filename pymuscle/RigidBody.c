#include "Config.h"
#include "RigidBody.h"


void SetRigidBodyChi_1(RigidBody *rb, const double Chi_1[3+4]) {
    RigidBodyNamed *rbn = &rb->b;
    int i;
    FOR_0(i, 3) {
        rbn->p0[i] = rbn->p[i];
        rbn->p[i] = Chi_1[i];
    }
    FOR_0(i, 4) {
        rbn->q0[i] = rbn->q[i];
        rbn->q[i] = Chi_1[i+3];
    }
}
