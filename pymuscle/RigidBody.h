#ifndef __RIGIDBODY_H_
#define __RIGIDBODY_H_

typedef enum _RotationParameterization {
    RP_UNKNOWN,
    RP_EULER_XYZ,
    RP_EULER_ZXZ,
    RP_QUAT_WFIRST,
    RP_EXP
} RotationParameterization;

#define MAX_FIBER_PER_RB (128)

typedef struct _RigidBodyNamed {
        /*
         * q,qd indices  0    1    2    3
         * ---------------------------------
         * QUAT_WFIRST:  w    x    y    z
         * EXP        :  x    y    z  ignored
         *
         */
        double p[3];
        double q[4];
        double pd[3];
        double qd[4];
        double m;
        double I[3];
        RotationParameterization rotParam;
        double boxSize[3];
        char name[128];
        char pName[128];
        double Ixyzw[4];
        double corners[8][3];
        double p0[3];
        double q0[4];
        int nFiber; /* Number of muscle fibers attached to this body */
        int fiber[MAX_FIBER_PER_RB];
        double chi_ref[3+4];
} RigidBodyNamed;

typedef union _RigidBody
{
    double a[3+4+3+4+1+3]; /* Old way to access RigidBody */
    RigidBodyNamed b;
} RigidBody;

void SetRigidBodyChi_1(RigidBody *rb, const double Chi_1[3+4]);

#endif
