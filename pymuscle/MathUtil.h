#ifndef MATHUTIL_H_INCLUDED
#define MATHUTIL_H_INCLUDED

#define PYM_MIN(a,b) (a)>(b)?(b):(a)
#define PYM_MAX(a,b) (b)>(a)?(b):(a)

void PymCrossToMat(double mat[3][3], const double v[3]);
double PymDist(int dim, const double *const v1, const double *const v2);
int PymMin(int a, int b);
double PymNormSq(int dim, const double *v);
double PymNorm(int dim, const double *v);
double NormalizeVector(int dim, double *v);
float NormalizeVectorf(int dim, float *v);
void VtoQuat(double q[4], const double v[3]);
void quat_rot(double v1[3], const double q[4], const double v0[3]);
void BoxInertiaTensorFromMassAndSize(double Ixyz[3], double m, double sx, double sy, double sz);
void QuatToEuler(double eul[3], double q[4]);
void Invert3x3Matrixf(float Minv[3][3], float M[3][3]);
void Invert3x3Matrixd(double Minv[3][3], const double M[3][3]);
void Invert6x6MassMatrix(double Minv[6][6], const double M[6][6]);
void GetWFrom6Dof(double W[4][4], const double chiexp[6]);
void AffineTransformPoint(double pt[3], const double W[4][4], const double p[3]);
void TransformPoint(double pt[3], const double W[4][4], const double p[3]);
void QuatToV(double v[3], const double q[4]);
double Dot33(const double a[3], const double b[3]);
double Dot(const unsigned int n, const double *a, const double *b);
#endif // MATHUTIL_H_INCLUDED
