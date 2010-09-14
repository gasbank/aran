#include "PymPch.h"
#include "Config.h"
#include "MathUtil.h"
#include "dRdv_real.h"

int PymMin(int a, int b) {
    if (a<b) return a;
    else return b;
}

double Dot33(const double a[3], const double b[3]) {
    return Dot(3, a, b);
}
double Dot(const unsigned int n, const double a[n], const double b[n]) {
    int i; double r = 0;
    for (i=0;i<n;++i) r+=a[i]*b[i];
    return r;
}

double PymNormSq(int dim, const double v[dim])
{
    double lenSq = 0;
    int i;
    for (i = 0; i < dim; ++i)
        lenSq += v[i]*v[i];
    return lenSq;
}

double PymNorm(int dim, const double v[dim])
{
    return sqrt(PymNormSq(dim, v));
}

float PymNormf(int dim, const float v[dim])
{
    float len = 0;
    int i;
    for (i = 0; i < dim; ++i)
        len += v[i]*v[i];
    len = sqrt(len);
    return len;
}

/* Normalize vector in-place */
double NormalizeVector(int dim, double v[dim])
{
    double len = PymNorm(dim, v);
    int i; FOR_0(i, dim) v[i] /= len;
    return len;
}

/* Normalize vector in-place */
float NormalizeVectorf(int dim, float v[dim])
{
    float len = PymNormf(dim, v);
    int i; FOR_0(i, dim) v[i] /= len;
    return len;
}

void VtoQuat(double q[4], const double v[3]) {
    const double th = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    double a;
	if (th < THETA)
		a = 0.5-th*th/48;
	else
		a = sin(th/2)/th;
	q[0] = cos(th/2);
	q[1] = a*v[0];
	q[2] = a*v[1];
	q[3] = a*v[2];
}

void quat_mult(double q[4], const double q1[4], const double q2[4]) {
    #define a1 q1[0]
    #define b1 q1[1]
    #define c1 q1[2]
    #define d1 q1[3]
    #define a2 q2[0]
    #define b2 q2[1]
    #define c2 q2[2]
    #define d2 q2[3]
	q[0] = a1*a2 - b1*b2 - c1*c2 - d1*d2;
    q[1] = a1*b2 + b1*a2 + c1*d2 - d1*c2;
    q[2] = a1*c2 - b1*d2 + c1*a2 + d1*b2;
    q[3] = a1*d2 + b1*c2 - c1*b2 + d1*a2;
    #undef a1
    #undef b1
    #undef c1
    #undef d1
    #undef a2
    #undef b2
    #undef c2
    #undef d2
}

void quat_rot(double v1[3], const double q[4], const double v0[3]) {
    DECLARE_COPY_VECTOR(qconj, 4, q);
    qconj[1] *= -1; qconj[2] *= -1; qconj[3] *= -1;
    double v0x[4]; v0x[0] = 0; memcpy(v0x+1, v0, sizeof(double)*3);
    double v0x_qconj[4];
    double q_v0x_qconj[4];
    quat_mult(v0x_qconj, v0x, qconj);
    quat_mult(q_v0x_qconj, q, v0x_qconj);
    memcpy(v1, q_v0x_qconj+1, sizeof(double)*3);
}

void BoxInertiaTensorFromMassAndSize(double Ixyz[3], double m, double sx, double sy, double sz)
{
    Ixyz[0] = m*(sy*sy + sz*sz)/12.;
    Ixyz[1] = m*(sx*sx + sz*sz)/12.;
    Ixyz[2] = m*(sx*sx + sy*sy)/12.;
}

void QuatToEuler(double eul[3], double q[4])
{
    eul[0] = atan2(2*(q[0]*q[1]+q[2]*q[3]), 1.-2*(q[1]*q[1]+q[2]*q[2]));
    /* asin(x) need x in range of [-1.,1.] */
    double clamped;
    clamped = PYM_MIN(1., 2*(q[0]*q[2]-q[3]*q[1]));
    clamped = PYM_MAX(-1., clamped);
    eul[1] = asin(clamped);
    eul[2] = atan2(2*(q[0]*q[3]+q[1]*q[2]), 1.-2*(q[2]*q[2]+q[3]*q[3]));
}

void Invert3x3Matrixf(float Minv[3][3], const float M[3][3])
{
    double determinant =+M[0][0]*(M[1][1]*M[2][2]-M[2][1]*M[1][2])
                        -M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
                        +M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
    double invdet = 1./determinant;
    Minv[0][0] =  (M[1][1]*M[2][2]-M[2][1]*M[1][2])*invdet;
    Minv[1][0] = -(M[0][1]*M[2][2]-M[0][2]*M[2][1])*invdet;
    Minv[2][0] =  (M[0][1]*M[1][2]-M[0][2]*M[1][1])*invdet;
    Minv[0][1] = -(M[1][0]*M[2][2]-M[1][2]*M[2][0])*invdet;
    Minv[1][1] =  (M[0][0]*M[2][2]-M[0][2]*M[2][0])*invdet;
    Minv[2][1] = -(M[0][0]*M[1][2]-M[1][0]*M[0][2])*invdet;
    Minv[0][2] =  (M[1][0]*M[2][1]-M[2][0]*M[1][1])*invdet;
    Minv[1][2] = -(M[0][0]*M[2][1]-M[2][0]*M[0][1])*invdet;
    Minv[2][2] =  (M[0][0]*M[1][1]-M[1][0]*M[0][1])*invdet;
}
void Invert3x3Matrixd(double Minv[3][3], const double M[3][3])
{
    double determinant =+M[0][0]*(M[1][1]*M[2][2]-M[2][1]*M[1][2])
                        -M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
                        +M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
    double invdet = 1./determinant;
    Minv[0][0] =  (M[1][1]*M[2][2]-M[2][1]*M[1][2])*invdet;
    Minv[1][0] = -(M[0][1]*M[2][2]-M[0][2]*M[2][1])*invdet;
    Minv[2][0] =  (M[0][1]*M[1][2]-M[0][2]*M[1][1])*invdet;
    Minv[0][1] = -(M[1][0]*M[2][2]-M[1][2]*M[2][0])*invdet;
    Minv[1][1] =  (M[0][0]*M[2][2]-M[0][2]*M[2][0])*invdet;
    Minv[2][1] = -(M[0][0]*M[1][2]-M[1][0]*M[0][2])*invdet;
    Minv[0][2] =  (M[1][0]*M[2][1]-M[2][0]*M[1][1])*invdet;
    Minv[1][2] = -(M[0][0]*M[2][1]-M[2][0]*M[0][1])*invdet;
    Minv[2][2] =  (M[0][0]*M[1][1]-M[1][0]*M[0][1])*invdet;
}

void Invert6x6MassMatrix(double Minv[6][6], const double M[6][6])
{
    /*
     *        M                     M^-1 (Minv)
     *
     *   /         \          /              \
     *   |  mI   0 |          |  I/m    0    |
     *   |         |    ===>  |              |
     *   |  0    H |          |   0     H^-1 |
     *   \         /          \              /
     *
     */
    memset(Minv, 0, sizeof(double)*6*6);
    assert(M[0][0] == M[1][1]);
    assert(M[1][1] == M[2][2]);
    Minv[0][0] = 1./M[0][0];
    Minv[1][1] = 1./M[1][1];
    Minv[2][2] = 1./M[2][2];
    double determinant =+M[3][3]*(M[4][4]*M[5][5]-M[5][4]*M[4][5])
                        -M[3][4]*(M[4][3]*M[5][5]-M[4][5]*M[5][3])
                        +M[3][5]*(M[4][3]*M[5][4]-M[4][4]*M[5][3]);
    double invdet = 1./determinant;
    Minv[3][3] =  (M[4][4]*M[5][5]-M[5][4]*M[4][5])*invdet;
    Minv[4][3] = -(M[3][4]*M[5][5]-M[3][5]*M[5][4])*invdet;
    Minv[5][3] =  (M[3][4]*M[4][5]-M[3][5]*M[4][4])*invdet;
    Minv[3][4] = -(M[4][3]*M[5][5]-M[4][5]*M[5][3])*invdet;
    Minv[4][4] =  (M[3][3]*M[5][5]-M[3][5]*M[5][3])*invdet;
    Minv[5][4] = -(M[3][3]*M[4][5]-M[4][3]*M[3][5])*invdet;
    Minv[3][5] =  (M[4][3]*M[5][4]-M[5][3]*M[4][4])*invdet;
    Minv[4][5] = -(M[3][3]*M[5][4]-M[5][3]*M[3][4])*invdet;
    Minv[5][5] =  (M[3][3]*M[4][4]-M[4][3]*M[3][4])*invdet;
}

void GetWFrom6Dof(double W[4][4], const double chiexp[6]) {
    double R[3][3];
    RotationMatrixFromV(R, &chiexp[3]);
    int i,j;
    for (i=0;i<3;++i) {
        for(j=0;j<3;++j) W[i][j] = R[i][j];
        W[i][3] = chiexp[i];
        W[3][i] = 0;
    }
    W[3][3] = 1.0;
}

void _TransformPoint(double pt[3], const double W[4][4], const double p[3], int affineAssert) {
    /*
     *    /                   \ /      \     /       \
     *    |  W00 W01 W02 W03  | |  p0  |     |  pt0  |
     *    |  W10 W11 W12 W13  | |  p1  |  =  |  pt1  |
     *    |  W20 W21 W22 W23  | |  p2  |     |  pt2  |
     *    |  W30 W31 W32 W33  | | 1.0  |     |  1.0  |
     *    \                   / \      /     \       /
     */
    int i, j;
    //i=3;
    //FOR_0(j, 4) printf("W[%d][%d] = %lf\n", i, j, W[i][j]);
    if (affineAssert)
        assert(W[3][0] == 0 && W[3][1] == 0 && W[3][2] == 0 && W[3][3] == 1.0);

    FOR_0(i, 3) {
        double val = 0;
        FOR_0(j, 3) {
            //printf("p[%d]=%lf\n", j, p[j]);
            //printf("W[%d][%d]=%lf\n", i, j, W[i][j]);
            val += W[i][j]*p[j];
        }
        val += W[i][3]*1.0; /* Assume p[3] is 1.0 */
        pt[i] = val;
    }
}

void AffineTransformPoint(double pt[3], const double W[4][4], const double p[3]) {
    _TransformPoint(pt, W, p, 1);
}

void TransformPoint(double pt[3], const double W[4][4], const double p[3]) {
    _TransformPoint(pt, W, p, 0);
}

void QuatToV(double v[3], const double q[4]) {
	v[0] = q[1];
	v[1] = q[2];
	v[2] = q[3];
	const double qv_mag = sqrt(q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
	double scaleFactor;
	if (qv_mag < THETA) scaleFactor = 1/(0.5-qv_mag*qv_mag/48);
	else scaleFactor = 2*acos(q[0])/qv_mag;
	int i; for (i=0;i<3;++i) v[i] *= scaleFactor;
}
