#ifndef ONERBIMP_H_INCLUDED
#define ONERBIMP_H_INCLUDED


int OneRbImp(const Double_18  body,
             const Double_6   extForce,
             const int        bClearVariable,
             Double_14        yd_R,
             /* Sparse matrix structure of 'dyd_RdY' */
             Uint_196x2       dyd_RdY_keys,
             Double_196       dyd_RdY_values);

#endif // ONERBIMP_H_INCLUDED
