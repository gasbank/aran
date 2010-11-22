#ifndef __QUATERNIONEOM_H_
#define __QUATERNIONEOM_H_

PYMCORE_API void quaternion_eom_test();
PYMCORE_API AranMath::Quaternion quaternion_eom(AranMath::Quaternion q0, AranMath::Quaternion q1, double h);
PYMCORE_API VectorR3 calc_angular_velocity(const AranMath::Quaternion &q1, const AranMath::Quaternion &q0, const double h);
PYMCORE_API void quaternion_eom_av(AranMath::Quaternion &q1, ArnVec3 &omega1, AranMath::Quaternion q0, ArnVec3 omega0, double h);
PYMCORE_API void quaternion_eom_av_rkn4(AranMath::Quaternion &q1, VectorR3 &omega1, AranMath::Quaternion q0, VectorR3 omega0, double t, double h);
PYMCORE_API void quaternion_eom_av_rk4(AranMath::Quaternion &q1, VectorR3 &omega1, AranMath::Quaternion q0, VectorR3 omega0, double t, double h);
#endif // __QUATERNIONEOM_H_