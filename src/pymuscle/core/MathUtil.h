#ifndef MATHUTIL_H_INCLUDED
#define MATHUTIL_H_INCLUDED

#define PYM_MIN(a,b) (a)>(b)?(b):(a)
#define PYM_MAX(a,b) (b)>(a)?(b):(a)

// Convert 3-Dim vector cross product operator to matrix
// i.e., v x a == [v]x a, where [v]x is 3-by-3 matrix.
PYMCORE_API void PymCrossToMat(double mat[3][3], const double v[3]);
// Distance between two n-Dim vectors
PYMCORE_API double PymDist(int dim, const double *const v1, const double *const v2);
// Min of int types
PYMCORE_API int PymMin(int a, int b);
PYMCORE_API int PymMax(int a, int b);
// Get n-Dim double vector norm squared
PYMCORE_API double PymNormSq(int dim, const double *v);
// Get n-Dim double vector norm
PYMCORE_API double PymNorm(int dim, const double *v);
// n-Dim double vector inplace normalization
PYMCORE_API double NormalizeVector(int dim, double *v);
// n-Dim float vector inplace normalization
PYMCORE_API float NormalizeVectorf(int dim, float *v);
// Convert exponential orientation to quaternion orientation
PYMCORE_API void VtoQuat(double q[4], const double v[3]);
// Rotate a vector using a quaternion
PYMCORE_API void quat_rot(double v1[3], const double q[4], const double v0[3]);
// Compute diagonal elements of inertia tensor when mass and size of a box is given. Uniform mass distribution.
PYMCORE_API void BoxInertiaTensorFromMassAndSize(double Ixyz[3], double m, double sx, double sy, double sz);
// Quaternion to Euler angles
PYMCORE_API void QuatToEuler(double eul[3], double q[4]);
// Invert 3x3 matrix (float)
PYMCORE_API void Invert3x3Matrixf(float Minv[3][3], float M[3][3]);
// Invert 3x3 matrix (double)
PYMCORE_API void Invert3x3Matrixd(double Minv[3][3], const double M[3][3]);
// Get invert matrix of 6x6 mass matrix (specialized form)
PYMCORE_API void Invert6x6MassMatrix(double Minv[6][6], const double M[6][6]);
// Compute transform matrix from 6DOF state(lin+exprot)
PYMCORE_API void GetWFrom6Dof(double W[4][4], const double chiexp[6]);
// Compute transform matrix from 7DOF state(lin+quat)
PYMCORE_API void GetWFrom7Dof(double W[4][4], const double chiexp[7]);
// Affine transform point or vector using 4x4 transform matrix
PYMCORE_API void AffineTransformPoint(double pt[3], const double W[4][4], const double p[3]);
// Transform point to point using 4x4 transform matrix
PYMCORE_API void TransformPoint(double pt[3], const double W[4][4], const double p[3]);
// Rotate point to point using 3x3 rotation matrix
PYMCORE_API void RotatePoint(double pt[3], const double R[3][3], const double p[3]);
// Quaternion to exponential rotation
PYMCORE_API void QuatToV(double v[3], const double q[4]);
// 3-Dim vector dot product
PYMCORE_API double Dot33(const double a[3], const double b[3]);
// n-Dim vector dot product
PYMCORE_API double Dot(const int n, const double *a, const double *b);
#endif // MATHUTIL_H_INCLUDED
