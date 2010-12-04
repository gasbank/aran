#ifndef __CHOLMODMACRO_H_
#define __CHOLMODMACRO_H_

struct pym_rb_statedep_t;

inline void SET_TRIPLET_RCV(cholmod_triplet *choltrip, int _r, int _c, double _v)
{
  unsigned int *r = (unsigned int *)(choltrip->i) + choltrip->nnz;
  unsigned int *c = (unsigned int *)(choltrip->j) + choltrip->nnz;
  double       *v = (double       *)(choltrip->x) + choltrip->nnz;
  *r = (_r);
  *c = (_c);
  *v = (_v);
  ++choltrip->nnz;
}

inline void SET_TRIPLET_RCV_SUBBLOCK(cholmod_triplet *choltrip, const pym_rb_statedep_t *const substruct, int _subr, int _subc, int _r, int _c, double _v)
{
  SET_TRIPLET_RCV(choltrip, substruct->Ari[_subr] + _r, substruct->Aci[_subc] + _c, _v);
}
inline void SET_TRIPLET_RCV_SUBBLOCK2(cholmod_triplet *choltrip, const int *const Ari, const int *const Aci, int _subr, int _subc, int _r, int _c, double _v)
{
  SET_TRIPLET_RCV(choltrip, Ari[_subr] + _r, Aci[_subc] + _c, _v);
}

#endif
