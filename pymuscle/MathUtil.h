#ifndef MATHUTIL_H_INCLUDED
#define MATHUTIL_H_INCLUDED

double NormalizeVector(int dim, double v[dim]);
void VtoQuat(double q[4], const double v[3]);
void quat_rot(double v1[3], const double q[4], const double v0[3]);

#endif // MATHUTIL_H_INCLUDED
