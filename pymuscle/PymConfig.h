#ifndef PYMCONFIG_H_INCLUDED
#define PYMCONFIG_H_INCLUDED

struct _pym_config_t {
    pym_rb_t *body; unsigned int nBody;
    pym_mf_t *fiber; unsigned int nFiber;
    unsigned int nJoint;
    double h;
    double mu;
    int nSimFrame;
    double slant;

    pym_joint_anchor_t pymJa[100];
    int na;
    pym_anchored_joint_t anchoredJoints[50];
    //int ja : not needed. use nJoint instead.
};

#endif // PYMCONFIG_H_INCLUDED
