#ifndef __STATEDEPENDENTS_H_
#define __STATEDEPENDENTS_H_

struct _pym_rb_statedep_t {
    double W_0[4][4];   /* previous homogeneous transform matrix */
    double W_1[4][4];   /* current homogeneous transform matrix */
    double M[6][6];     /* mass matrix */
    double Minv[6][6];  /* inverse of mass matrix */
    double Cqd[6];      /* Coriolis and other terms vector */
    double f_g[6];      /* generalized gravitational force */
    double f_ext[6];    /* generalized external force */
    /*
     * Three 4x4 matrices
     *
     *   d  W
     *   -----     where n = 1, 2, 3
     *   d v_n
     */
    double dWdchi_tensor[3][4][4];
    double contacts_1[MAX_CONTACTS][3]; /* Contact point position in world coordinate frame */
    int nContacts_1;
    int nContacts_2;
    int contactIndices_2[MAX_CONTACTS]; /* Estimated indices of corner points in next time step */
    double contactsFix_2[MAX_CONTACTS][4];       /* Estimated contact point fixing position in world coordinate frame in next time step */
    double contactsNormal_1[MAX_CONTACTS][4];

    int Asubrows;
    int Asubcols;
    int Ari[ 1 +  9 ]; /* 1 + # of sub row */
    int Aci[ 1 + 14 ]; /* 1 + # of sub col */
  
    /*
     * Z and V : Coefficients for calculating next state based on current state
     * chi^(l+1) = Z chi^(l) + V
     */
    cholmod_sparse *Z  [MAX_CONTACTS];     /* A list of sparse 4x6 matrices (for contact points) */
    double          V  [MAX_CONTACTS][4];  /* for contact points */
    cholmod_sparse *Q [MAX_CONTACTS];      /* A list of sparse 6x5 matrices */

    cholmod_sparse *Za [MAX_JOINTANCHORS];     /* A list of sparse 4x6 matrices (for joint anchors) */
    double          Va [MAX_JOINTANCHORS][4];  /* for joint anchors */
};

int PymConstructRbStatedep(pym_rb_statedep_t *sd, const pym_rb_t *rb,
                           FILE *dmstreams[],
                           const pym_config_t *pymCfg, cholmod_common *cc);
void PymDestroyRbStatedep(pym_rb_statedep_t *sd, pym_rb_named_t *rbn, cholmod_common *cc);

#endif
