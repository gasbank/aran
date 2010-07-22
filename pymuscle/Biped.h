#ifndef __BIPED_H_
#define __BIPED_H_

struct _pym_biped_eqconst_t {
    cholmod_sparse *bipMat;
    double *bipEta; /* dynamically allocated array */
    int *Ari; /* dynamic array of size (1+Asubrows) */
    int *Aci; /* dynamic array of size (1+Asubcols) */
    int Asubrows;
    int Asubcols;

    int *Airi; /* dynamic array of size (1+nb) */
    int *Aici; /* dynamic array of size (1+nb) */
};

void PymDestroyBipedEqconst(pym_biped_eqconst_t *bipEq, cholmod_common *cc);

#endif
