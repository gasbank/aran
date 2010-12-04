#ifndef __DRDV_REAL_H_
#define __DRDV_REAL_H_

// Generalized force computation when exponential rotation parameterization used
// Q  -> generalized force
// v  -> exp rot orientation
// Fr -> force
// r  -> point on the body in local coordinates where Fr exerted
void GeneralizedForce(double Q[3+3], const double v[3], const double Fr[3], const double r[3]);
void GeneralizedForceQuat(double Q[3+4], const double q[4], const double Fr[3], const double r[3]);
// d R    ... 3x3 matrix
// ---  ==> 3x3x3 tensor matrix
// d v    ... 3x1 vector (exprot)
void dRdv(double dRdv1_s[3][3], double dRdv2_s[3][3], double dRdv3_s[3][3], const double v[3]);
// d R    ... 3x3 matrix
// ---  ==> 4x3x3 tensor matrix
// d q    ... 4x1 vector (quaternion)
void dRdq(double dRdqw_s[3][3], double dRdqx_s[3][3], double dRdqy_s[3][3], double dRdqz_s[3][3], const double q[4]);
// 3x3 rotation matrix from v (exprot)
void RotationMatrixFromV(double R[3][3], const double v[3]);
void RotationMatrixFromQuat(double R[3][3], const double q[4]);

#endif
