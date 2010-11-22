#ifndef __BIPED_H_
#define __BIPED_H_

struct pym_biped_eqconst_t {
    cholmod_sparse *bipMat;
    double *bipEta; /* dynamically allocated array */
    int Ari[1+8]; /* dynamic array of size (1+Asubrows) */
    int Aci[1+17]; /* dynamic array of size (1+Asubcols) */

    int *Airi; /* dynamic array of size (1+nb) */
    int *Aici; /* dynamic array of size (1+nb) */
};

PYMCORE_API void PymDestroyBipedEqconst(pym_biped_eqconst_t *bipEq, cholmod_common *cc);

#endif
