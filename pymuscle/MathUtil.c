#include <math.h>
#include <string.h>
#include "Config.h"

double NormalizeVector(int dim, double v[dim])
{
    double len = 0;
    int i;
    for (i = 0; i < dim; ++i)
        len += v[i]*v[i];
    len = sqrt(len);
    for (i = 0; i < dim; ++i)
        v[i] /= len;
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

