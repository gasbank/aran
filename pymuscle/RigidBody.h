#ifndef __RIGIDBODY_H_
#define __RIGIDBODY_H_

#define NUM_DOF          (3 + 3)
#define MAX_CONTACTS     (8)
#define MAX_JOINTANCHORS (10)

enum _pym_rot_param_t {
    RP_UNKNOWN,
    RP_EULER_XYZ,
    RP_EULER_ZXZ,
    RP_QUAT_WFIRST,
    RP_EXP
};

#define MAX_FIBER_PER_RB (128)

struct _pym_rb_named_t {
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
        /******************************************************************/
        /* Data declared so far should not be modified for compatibility  */
        /******************************************************************/
        pym_rot_param_t rotParam;
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
        int nAnchor;
        char jointAnchorNames[MAX_JOINTANCHORS][128];
        double jointAnchors[MAX_JOINTANCHORS][4];
};

union _pym_rb_t
{
    double a[3+4+3+4+1+3]; /* Old way to access RigidBody */
    pym_rb_named_t b;
};

void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+3], const pym_config_t *const pymCfg);

#endif
