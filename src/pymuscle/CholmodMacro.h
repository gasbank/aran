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
  const int r = substruct->Ari[_subr] + _r;
  const int c = substruct->Aci[_subc] + _c;
  assert(0 <= _subr && _subr < substruct->Asubrows);
  assert(0 <= _subc && _subc < substruct->Asubcols);
  assert(0 <= r && r < substruct->Ari[substruct->Asubrows]);
  assert(0 <= c && c < substruct->Aci[substruct->Asubcols]);
  SET_TRIPLET_RCV(choltrip, r, c, _v);
}


#endif
