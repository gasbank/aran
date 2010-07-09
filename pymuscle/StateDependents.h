#ifndef __STATEDEPENDENTS_H_
#define __STATEDEPENDENTS_H_
#define NUM_DOF        (3 + 3)
#define MAX_CONTACTS   (8)


struct _pym_rb_statedep_t {
    double W_0[4][4];   /* previous homogeneous transform matrix */
    double W_1[4][4];   /* current homogeneous transform matrix */
    double M[6][6];     /* mass matrix */
    double Minv[6][6];  /* inverse of mass matrix */
    double Cqd[6];      /* Coriolis and other terms vector */
    double f_g[6];      /* generalized gravitational force */
    /*
     * Three 4x4 matrices
     *
     *   d  W
     *   -----     where n = 1, 2, 3
     *   d v_n
     */
    double dWdchi_tensor[3][4][4];
    double contacts_1[MAX_CONTACTS][3]; /* Contact point position in world coordinate frame */
    unsigned int nContacts_1;
    unsigned int nContacts_2;
    unsigned int contactIndices_2[MAX_CONTACTS]; /* Estimated indices of corner points in next time step */
    double contactsFix_2[MAX_CONTACTS][4];       /* Estimated contact point fixing position in world coordinate frame in next time step */
    int Asubrows;
    int Asubcols;
    int Ari[1+6];
    int Aci[1+10];
    cholmod_sparse *Z [MAX_CONTACTS]; /* A list of sparse 4x6 matrices */
    double          V [MAX_CONTACTS][4];
    cholmod_sparse *Q [MAX_CONTACTS]; /* A list of sparse 6x5 matrices */

};



int PymConstructRbStatedep(pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void GetAMatrix(cholmod_triplet **AMatrix, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void GetEta(double **_eta, const pym_rb_statedep_t *sd, const pym_rb_t *rb, const pym_config_t *pymCfg, cholmod_common *cc);
void PymDestroyRbStatedep(pym_rb_statedep_t *sd, cholmod_common *cc);

#endif
