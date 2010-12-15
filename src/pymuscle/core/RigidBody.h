#ifndef __RIGIDBODY_H_
#define __RIGIDBODY_H_

union pym_rb_t;
struct pym_config_t;
struct pym_rb_statedep_t;
struct pym_rb_named_t;

#define MAX_CONTACTS     (8)
#define MAX_JOINTANCHORS (10)

enum pym_rot_param_t {
    RP_UNKNOWN,
    RP_EULER_XYZ,
    RP_EULER_ZXZ,
    RP_QUAT_WFIRST,
    RP_QUAT_WLAST,
    RP_EXP
};

#define MAX_FIBER_PER_RB (256)

struct pym_rb_named_t {
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
    int nFiber; /* Number of muscle fibers attached to this body */
    int fiber[MAX_FIBER_PER_RB];
    double chi_ref[3+4];
    int nAnchor;
    char jointAnchorNames[MAX_JOINTANCHORS][128];
    double jointAnchors[MAX_JOINTANCHORS][4];
    double extForce[3];       /* External force applied to this body */
    double extForcePos[3];    /* External force applied position in local coord. */

    /* tilde t_{c,i}^{l+1} : Being calculated during optimization */
    int nContacts_1;
    double contactsPoints_1[MAX_CONTACTS][3];
    double contactsPoints_2[MAX_CONTACTS][3]; /* output of optimizer (estimated) */
    double contactsForce_2[MAX_CONTACTS][3]; /* output of optimizer (exerted on 'contactPoints_1') */

    /* Original(initial) state parsed from .sim.conf */
    double p_simconf[3];
    double q_simconf[4];
    double pd_simconf[3];
    double qd_simconf[4];
    bool track;
};

union pym_rb_t
{
    double a[3+4+3+4+1+3]; /* Old way to access RigidBody */
    pym_rb_named_t b;
};

PYMCORE_API void SetRigidBodyChi_1(pym_rb_t *rb, const double Chi_1[3+3], const pym_config_t *const pymCfg);
PYMCORE_API int PymCheckRotParam(pym_config_t *pymCfg);
PYMCORE_API std::ostream &operator << (std::ostream &s, const pym_rb_t &rb);
PYMCORE_API std::ostream &pym_print_detailed_rb_state(std::ostream &s, const pym_rb_t &rb);

typedef struct cholmod_triplet_struct cholmod_triplet;
typedef struct cholmod_common_struct cholmod_common;

void GetAMatrix(cholmod_triplet **AMatrix, const pym_rb_statedep_t *const sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void GetEta(double **_eta, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);


#endif
