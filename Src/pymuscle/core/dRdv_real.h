#ifndef __DRDV_REAL_H_
#define __DRDV_REAL_H_

void GeneralizedForce(double Q[6], const double v[3], const double Fr[3], const double r[3]);
void dRdv(double dRdv1_s[3][3], double dRdv2_s[3][3], double dRdv3_s[3][3], const double v[3]);
void RotationMatrixFromV(double R[3][3], const double v[3]);

#endif
