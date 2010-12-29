#ifndef __BIPED_H_
#define __BIPED_H_

struct pym_biped_eqconst_t {
    cholmod_sparse *bipMat;
    double *bipEta; /* dynamically allocated array */
    int Ari[1+14]; /* dynamic array of size (1+Asubrows) */
    int Aci[1+27]; /* dynamic array of size (1+Asubcols) */

    int *Airi; /* dynamic array of size (1+nb) */
    int *Aici; /* dynamic array of size (1+nb) */
};

struct pym_config_t;

PYMCORE_API void PymDestroyBipedEqconst(pym_biped_eqconst_t *bipEq, cholmod_common *cc);
PYMCORE_API void pym_update_com( pym_config_t *pymCfg );
PYMCORE_API void pym_reset_com0( pym_config_t *pymCfg );
#endif
