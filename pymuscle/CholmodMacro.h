#ifndef __CHOLMODMACRO_H_
#define __CHOLMODMACRO_H_

#define SET_TRIPLET_RCV(choltrip, _r, _c, _v) \
{\
    unsigned int *r = (unsigned int *)(choltrip->i) + choltrip->nnz; \
    unsigned int *c = (unsigned int *)(choltrip->j) + choltrip->nnz; \
    double       *v = (double       *)(choltrip->x) + choltrip->nnz; \
    *r = (_r); \
    *c = (_c); \
    *v = (_v); \
    ++choltrip->nnz; \
}
#define SET_TRIPLET_RCV_SUBBLOCK(choltrip, substruct, _subr, _subc, _r, _c, _v) \
    SET_TRIPLET_RCV(choltrip, substruct->Ari[_subr] + _r, substruct->Aci[_subc] + _c, _v)
#define SET_TRIPLET_RCV_SUBBLOCK2(choltrip, _subr, _subc, _r, _c, _v) \
    SET_TRIPLET_RCV(choltrip, Ari[_subr] + _r, Aci[_subc] + _c, _v)

#endif
