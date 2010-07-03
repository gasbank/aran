/*
 * dRdv_real.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Rigid body equations (single)
 *
 * Rotation is parameterized by an exponential map technique
 */
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Config.h"

#define p1  p [0]
#define p2  p [1]
#define p3  p [2]
#define pd1 pd[0]
#define pd2 pd[1]
#define pd3 pd[2]
#define v1  v [0]
#define v2  v [1]
#define v3  v [2]
#define vd1 vd[0]
#define vd2 vd[1]
#define vd3 vd[2]
#define Ixx I[0]
#define Iyy I[1]
#define Izz I[2]
#define Iww I[3]
#define Fr1  Fr [0]
#define Fr2  Fr [1]
#define Fr3  Fr [2]
#define r1    r [0]
#define r2    r [1]
#define r3    r [2]



void __dRdv(double dRdv1_s[3][3], double dRdv2_s[3][3], double dRdv3_s[3][3], const double v[3], const double th) {
    /*;
    ################################################;
    #      Variable: dRdv1_s;
    ################################################;
    */;
    double _x1 = pow(th,-1);
    double _x2 = 0.5*th;
    double _x3 = cos(_x2);
    double _x4 = sin(_x2);
    double _x5 = -1.0*_x1*_x3*_x4*v1;
    double _x6 = pow(th,-2);
    double _x7 = pow(_x4,2);
    double _x8 = pow(th,-3);
    double _x9 = pow(v1,3);
    double _x10 = pow(th,-4);
    double _x11 = pow(v2,2);
    double _x12 = -1.0*_x8*_x3*_x4*v1*_x11;
    double _x13 = 2*_x10*_x7*v1*_x11;
    double _x14 = pow(v3,2);
    double _x15 = -1.0*_x8*_x3*_x4*v1*_x14;
    double _x16 = 2*_x10*_x7*v1*_x14;
    double _x17 = 2*_x6*_x7*v2;
    double _x18 = pow(v1,2);
    double _x19 = 2.0*_x8*_x3*_x4*_x18*v2;
    double _x20 = -4*_x10*_x7*_x18*v2;
    double _x21 = pow(_x3,2);
    double _x22 = 2*_x6*_x7*v3;
    double _x23 = 2.0*_x8*_x3*_x4*_x18*v3;
    double _x24 = -4*_x10*_x7*_x18*v3;
    double _x25 = -2*_x6*_x7*v1;
    double _x26 = -1.0*_x8*_x3*_x4*_x9;
    double _x27 = 2*_x10*_x7*_x9;
    double _x28 = 2.0*_x8*_x3*_x4*v1*v2*v3;
    double _x29 = -4*_x10*_x7*v1*v2*v3;
    dRdv1_s[  0][  0] = _x16+_x15+_x13+_x12-2*_x10*_x7*_x9+1.0*_x8*_x3*_x4*_x9+2*_x6*_x7*v1+_x5;
    dRdv1_s[  0][  1] = 1.0*_x6*_x7*v1*v3+2*_x8*_x3*_x4*v1*v3-1.0*_x6*_x21*v1*v3+_x20+_x19+_x17;
    dRdv1_s[  0][  2] = _x24+_x23+_x22-1.0*_x6*_x7*v1*v2-2*_x8*_x3*_x4*v1*v2+1.0*_x6*_x21*v1*v2;
    dRdv1_s[  1][  0] = -1.0*_x6*_x7*v1*v3-2*_x8*_x3*_x4*v1*v3+1.0*_x6*_x21*v1*v3+_x20+_x19+_x17;
    dRdv1_s[  1][  1] = _x16+_x15-2*_x10*_x7*v1*_x11+1.0*_x8*_x3*_x4*v1*_x11+_x27+_x26+_x25+_x5;
    dRdv1_s[  1][  2] = _x29+_x28+1.0*_x6*_x7*_x18+2*_x8*_x3*_x4*_x18-1.0*_x6*_x21*_x18-2*_x1*_x3*_x4;
    dRdv1_s[  2][  0] = _x24+_x23+_x22+1.0*_x6*_x7*v1*v2+2*_x8*_x3*_x4*v1*v2-1.0*_x6*_x21*v1*v2;
    dRdv1_s[  2][  1] = _x29+_x28-1.0*_x6*_x7*_x18-2*_x8*_x3*_x4*_x18+1.0*_x6*_x21*_x18+2*_x1*_x3*_x4;
    dRdv1_s[  2][  2] = -2*_x10*_x7*v1*_x14+1.0*_x8*_x3*_x4*v1*_x14+_x13+_x12+_x27+_x26+_x25+_x5;
    /*;
    ################################################;
    #      Variable: dRdv2_s;
    ################################################;
    */;
    _x1 = pow(th,-1);
    _x2 = 0.5*th;
    _x3 = cos(_x2);
    _x4 = sin(_x2);
    _x5 = -1.0*_x1*_x3*_x4*v2;
    _x6 = pow(th,-2);
    _x7 = pow(_x4,2);
    _x8 = -2*_x6*_x7*v2;
    _x9 = pow(th,-3);
    _x10 = pow(v1,2);
    _x11 = pow(th,-4);
    _x12 = pow(v2,3);
    _x13 = -1.0*_x9*_x3*_x4*_x12;
    _x14 = 2*_x11*_x7*_x12;
    _x15 = pow(v3,2);
    _x16 = -1.0*_x9*_x3*_x4*v2*_x15;
    _x17 = 2*_x11*_x7*v2*_x15;
    _x18 = 2*_x6*_x7*v1;
    _x19 = pow(v2,2);
    _x20 = 2.0*_x9*_x3*_x4*v1*_x19;
    _x21 = -4*_x11*_x7*v1*_x19;
    _x22 = pow(_x3,2);
    _x23 = 2.0*_x9*_x3*_x4*v1*v2*v3;
    _x24 = -4*_x11*_x7*v1*v2*v3;
    _x25 = -1.0*_x9*_x3*_x4*_x10*v2;
    _x26 = 2*_x11*_x7*_x10*v2;
    _x27 = 2*_x6*_x7*v3;
    _x28 = 2.0*_x9*_x3*_x4*_x19*v3;
    _x29 = -4*_x11*_x7*_x19*v3;
    dRdv2_s[  0][  0] = _x17+_x16+_x14+_x13-2*_x11*_x7*_x10*v2+1.0*_x9*_x3*_x4*_x10*v2+_x8+_x5;
    dRdv2_s[  0][  1] = 1.0*_x6*_x7*v2*v3+2*_x9*_x3*_x4*v2*v3-1.0*_x6*_x22*v2*v3+_x21+_x20+_x18;
    dRdv2_s[  0][  2] = _x24+_x23-1.0*_x6*_x7*_x19-2*_x9*_x3*_x4*_x19+1.0*_x6*_x22*_x19+2*_x1*_x3*_x4;
    dRdv2_s[  1][  0] = -1.0*_x6*_x7*v2*v3-2*_x9*_x3*_x4*v2*v3+1.0*_x6*_x22*v2*v3+_x21+_x20+_x18;
    dRdv2_s[  1][  1] = _x17+_x16-2*_x11*_x7*_x12+1.0*_x9*_x3*_x4*_x12+_x26+_x25+2*_x6*_x7*v2+_x5;
    dRdv2_s[  1][  2] = _x29+_x28+_x27+1.0*_x6*_x7*v1*v2+2*_x9*_x3*_x4*v1*v2-1.0*_x6*_x22*v1*v2;
    dRdv2_s[  2][  0] = _x24+_x23+1.0*_x6*_x7*_x19+2*_x9*_x3*_x4*_x19-1.0*_x6*_x22*_x19-2*_x1*_x3*_x4;
    dRdv2_s[  2][  1] = _x29+_x28+_x27-1.0*_x6*_x7*v1*v2-2*_x9*_x3*_x4*v1*v2+1.0*_x6*_x22*v1*v2;
    dRdv2_s[  2][  2] = -2*_x11*_x7*v2*_x15+1.0*_x9*_x3*_x4*v2*_x15+_x14+_x13+_x26+_x25+_x8+_x5;
    /*;
    ################################################;
    #      Variable: dRdv3_s;
    ################################################;
    */;
    _x1 = pow(th,-1);
    _x2 = 0.5*th;
    _x3 = cos(_x2);
    _x4 = sin(_x2);
    _x5 = -1.0*_x1*_x3*_x4*v3;
    _x6 = pow(th,-2);
    _x7 = pow(_x4,2);
    _x8 = -2*_x6*_x7*v3;
    _x9 = pow(th,-3);
    _x10 = pow(v1,2);
    _x11 = pow(th,-4);
    _x12 = pow(v2,2);
    _x13 = -1.0*_x9*_x3*_x4*_x12*v3;
    _x14 = 2*_x11*_x7*_x12*v3;
    _x15 = pow(v3,3);
    _x16 = -1.0*_x9*_x3*_x4*_x15;
    _x17 = 2*_x11*_x7*_x15;
    _x18 = 2.0*_x9*_x3*_x4*v1*v2*v3;
    _x19 = -4*_x11*_x7*v1*v2*v3;
    _x20 = pow(_x3,2);
    _x21 = pow(v3,2);
    _x22 = 2*_x6*_x7*v1;
    _x23 = 2.0*_x9*_x3*_x4*v1*_x21;
    _x24 = -4*_x11*_x7*v1*_x21;
    _x25 = -1.0*_x9*_x3*_x4*_x10*v3;
    _x26 = 2*_x11*_x7*_x10*v3;
    _x27 = 2*_x6*_x7*v2;
    _x28 = 2.0*_x9*_x3*_x4*v2*_x21;
    _x29 = -4*_x11*_x7*v2*_x21;
    dRdv3_s[  0][  0] = _x17+_x16+_x14+_x13-2*_x11*_x7*_x10*v3+1.0*_x9*_x3*_x4*_x10*v3+_x8+_x5;
    dRdv3_s[  0][  1] = 1.0*_x6*_x7*_x21+2*_x9*_x3*_x4*_x21-1.0*_x6*_x20*_x21+_x19+_x18-2*_x1*_x3*_x4;
    dRdv3_s[  0][  2] = _x24+_x23-1.0*_x6*_x7*v2*v3-2*_x9*_x3*_x4*v2*v3+1.0*_x6*_x20*v2*v3+_x22;
    dRdv3_s[  1][  0] = -1.0*_x6*_x7*_x21-2*_x9*_x3*_x4*_x21+1.0*_x6*_x20*_x21+_x19+_x18+2*_x1*_x3*_x4;
    dRdv3_s[  1][  1] = _x17+_x16-2*_x11*_x7*_x12*v3+1.0*_x9*_x3*_x4*_x12*v3+_x26+_x25+_x8+_x5;
    dRdv3_s[  1][  2] = _x29+_x28+1.0*_x6*_x7*v1*v3+2*_x9*_x3*_x4*v1*v3-1.0*_x6*_x20*v1*v3+_x27;
    dRdv3_s[  2][  0] = _x24+_x23+1.0*_x6*_x7*v2*v3+2*_x9*_x3*_x4*v2*v3-1.0*_x6*_x20*v2*v3+_x22;
    dRdv3_s[  2][  1] = _x29+_x28-1.0*_x6*_x7*v1*v3-2*_x9*_x3*_x4*v1*v3+1.0*_x6*_x20*v1*v3+_x27;
    dRdv3_s[  2][  2] = -2*_x11*_x7*_x15+1.0*_x9*_x3*_x4*_x15+_x14+_x13+_x26+_x25+2*_x6*_x7*v3+_x5;
}

void __dRdv0(double dRdv1_s0[3][3], double dRdv2_s0[3][3], double dRdv3_s0[3][3], const double v[3], const double th) {
    /*;
    ################################################;
    #      Variable: dRdv1_s0;
    ################################################;
    */;
    double _x1 = pow(v1,3);
    double _x2 = pow(v2,2);
    double _x3 = -_x2;
    double _x4 = pow(v3,2);
    double _x5 = -_x4;
    double _x6 = pow(th,2);
    double _x7 = -15*v2;
    double _x8 = pow(v1,2);
    double _x9 = 2*_x8*v2;
    double _x10 = -6*v2;
    double _x11 = _x8*v2;
    double _x12 = -6*v3;
    double _x13 = _x8*v3;
    double _x14 = -15*v3;
    double _x15 = 2*_x8*v3;
    double _x16 = 4*_x8;
    double _x17 = -v1*v2*v3;
    double _x18 = 6*_x8;
    double _x19 = v1*v2*v3;
    dRdv1_s0[  0][  0] = _x6*(v1*(_x5+_x3+15)+_x1)/360-(v1*(_x5+_x3)+_x1)/24;
    dRdv1_s0[  0][  1] = _x6*(-12*v1*v3+_x9+_x7)/360-(-4*v1*v3+_x11+_x10)/12;
    dRdv1_s0[  0][  2] = _x6*(_x15+_x14+12*v1*v2)/360-(_x13+_x12+4*v1*v2)/12;
    dRdv1_s0[  1][  0] = _x6*(12*v1*v3+_x9+_x7)/360-(4*v1*v3+_x11+_x10)/12;
    dRdv1_s0[  1][  1] = (v1*(_x4+_x3-24)+_x1)/24-_x6*(v1*(_x4+_x3-45)+_x1)/360;
    dRdv1_s0[  1][  2] = (_x17+_x16-12)/12-_x6*(_x17+_x18-30)/180;
    dRdv1_s0[  2][  0] = _x6*(_x15+_x14-12*v1*v2)/360-(_x13+_x12-4*v1*v2)/12;
    dRdv1_s0[  2][  1] = _x6*(_x19+_x18-30)/180-(_x19+_x16-12)/12;
    dRdv1_s0[  2][  2] = (v1*(_x5+_x2-24)+_x1)/24-_x6*(v1*(_x5+_x2-45)+_x1)/360;
    /*;
    ################################################;
    #      Variable: dRdv2_s0;
    ################################################;
    */;
    _x1 = pow(th,2);
    _x2 = pow(v2,3);
    _x3 = pow(v1,2);
    _x4 = -_x3;
    _x5 = pow(v3,2);
    _x6 = -15*v1;
    _x7 = pow(v2,2);
    _x8 = 2*v1*_x7;
    _x9 = -6*v1;
    _x10 = v1*_x7;
    _x11 = 4*_x7;
    _x12 = v1*v2*v3;
    _x13 = 6*_x7;
    _x14 = -_x5;
    _x15 = -6*v3;
    _x16 = _x7*v3;
    _x17 = -15*v3;
    _x18 = 2*_x7*v3;
    _x19 = -v1*v2*v3;
    dRdv2_s0[  0][  0] = (v2*(_x5+_x4-24)+_x2)/24-_x1*(v2*(_x5+_x4-45)+_x2)/360;
    dRdv2_s0[  0][  1] = _x1*(-12*v2*v3+_x8+_x6)/360-(-4*v2*v3+_x10+_x9)/12;
    dRdv2_s0[  0][  2] = _x1*(_x12+_x13-30)/180-(_x12+_x11-12)/12;
    dRdv2_s0[  1][  0] = _x1*(12*v2*v3+_x8+_x6)/360-(4*v2*v3+_x10+_x9)/12;
    dRdv2_s0[  1][  1] = _x1*(v2*(_x14+_x4+15)+_x2)/360-(v2*(_x14+_x4)+_x2)/24;
    dRdv2_s0[  1][  2] = _x1*(_x18+_x17-12*v1*v2)/360-(_x16+_x15-4*v1*v2)/12;
    dRdv2_s0[  2][  0] = (_x19+_x11-12)/12-_x1*(_x19+_x13-30)/180;
    dRdv2_s0[  2][  1] = _x1*(_x18+_x17+12*v1*v2)/360-(_x16+_x15+4*v1*v2)/12;
    dRdv2_s0[  2][  2] = (v2*(_x14+_x3-24)+_x2)/24-_x1*(v2*(_x14+_x3-45)+_x2)/360;
    /*;
    ################################################;
    #      Variable: dRdv3_s0;
    ################################################;
    */;
    _x1 = pow(th,2);
    _x2 = pow(v1,2);
    _x3 = -_x2;
    _x4 = pow(v2,2);
    _x5 = pow(v3,3);
    _x6 = -v1*v2*v3;
    _x7 = pow(v3,2);
    _x8 = 4*_x7;
    _x9 = 6*_x7;
    _x10 = -6*v1;
    _x11 = v1*_x7;
    _x12 = -15*v1;
    _x13 = 2*v1*_x7;
    _x14 = v1*v2*v3;
    _x15 = -_x4;
    _x16 = -6*v2;
    _x17 = v2*_x7;
    _x18 = -15*v2;
    _x19 = 2*v2*_x7;
    dRdv3_s0[  0][  0] = (_x5+(_x4+_x3-24)*v3)/24-_x1*(_x5+(_x4+_x3-45)*v3)/360;
    dRdv3_s0[  0][  1] = (_x8+_x6-12)/12-_x1*(_x9+_x6-30)/180;
    dRdv3_s0[  0][  2] = _x1*(_x13+12*v2*v3+_x12)/360-(_x11+4*v2*v3+_x10)/12;
    dRdv3_s0[  1][  0] = _x1*(_x9+_x14-30)/180-(_x8+_x14-12)/12;
    dRdv3_s0[  1][  1] = (_x5+(_x15+_x2-24)*v3)/24-_x1*(_x5+(_x15+_x2-45)*v3)/360;
    dRdv3_s0[  1][  2] = _x1*(_x19-12*v1*v3+_x18)/360-(_x17-4*v1*v3+_x16)/12;
    dRdv3_s0[  2][  0] = _x1*(_x13-12*v2*v3+_x12)/360-(_x11-4*v2*v3+_x10)/12;
    dRdv3_s0[  2][  1] = _x1*(_x19+12*v1*v3+_x18)/360-(_x17+4*v1*v3+_x16)/12;
    dRdv3_s0[  2][  2] = _x1*(_x5+(_x15+_x3+15)*v3)/360-(_x5+(_x15+_x3)*v3)/24;
}

void dRdv(double dRdv1_s[3][3], double dRdv2_s[3][3], double dRdv3_s[3][3], const double v[3]) {
    double th = sqrt(v1*v1+v2*v2+v3*v3);
    if (th < THETA) __dRdv0(dRdv1_s, dRdv2_s, dRdv3_s, v, th);
    else __dRdv(dRdv1_s, dRdv2_s, dRdv3_s, v, th);
}


void __RotationMatrixFromV(double rotMat[3][3], const double v[3], const double th) {
    /*
    ################################################
    #      Variable: rotMat
    ################################################
    */
    double _x1 = pow(v1,2);
    double _x2 = pow(v2,2);
    double _x3 = pow(v3,2);
    double _x4 = _x3+_x2+_x1;
    double _x5 = pow(_x4,1./2);
    double _x6 = 0.5*_x5;
    double _x7 = cos(_x6);
    double _x8 = pow(_x7,2);
    double _x9 = pow(_x4,-1);
    double _x10 = sin(_x6);
    double _x11 = pow(_x10,2);
    double _x12 = -_x2*_x9*_x11;
    double _x13 = -_x3*_x9*_x11;
    double _x14 = pow(_x5,-1);
    double _x15 = 2*v1*v2*_x9*_x11;
    double _x16 = 2*v1*v3*_x9*_x11;
    double _x17 = -_x1*_x9*_x11;
    double _x18 = 2*v2*v3*_x9*_x11;
    rotMat[  0][  0] = _x13+_x12+_x1*_x9*_x11+_x8;
    rotMat[  0][  1] = _x15-2*v3*_x14*_x7*_x10;
    rotMat[  0][  2] = _x16+2*v2*_x14*_x7*_x10;
    rotMat[  1][  0] = _x15+2*v3*_x14*_x7*_x10;
    rotMat[  1][  1] = _x13+_x2*_x9*_x11+_x17+_x8;
    rotMat[  1][  2] = _x18-2*v1*_x14*_x7*_x10;
    rotMat[  2][  0] = _x16-2*v2*_x14*_x7*_x10;
    rotMat[  2][  1] = _x18+2*v1*_x14*_x7*_x10;
    rotMat[  2][  2] = _x3*_x9*_x11+_x12+_x17+_x8;
}
void __RotationMatrixFromV0(double rotMat0[3][3], const double v[3], const double th) {
    /*
    ################################################
    #      Variable: rotMat0
    ################################################
    */
    double _x1 = pow(v1,2);
    double _x2 = pow(v2,2);
    double _x3 = -_x2;
    double _x4 = pow(v3,2);
    double _x5 = -_x4;
    double _x6 = pow(th,2);
    double _x7 = v1*v2;
    double _x8 = v1*v3;
    double _x9 = 2*v1;
    double _x10 = -v2*v3;
    double _x11 = 4*v1;
    double _x12 = v2*v3;
    rotMat0[  0][  0] = (_x5+_x3+_x1+4)/4-_x6*(_x5+_x3+_x1+12)/48;
    rotMat0[  0][  1] = (_x7-2*v3)/2-_x6*(_x7-4*v3)/24;
    rotMat0[  0][  2] = (_x8+2*v2)/2-_x6*(_x8+4*v2)/24;
    rotMat0[  1][  0] = (2*v3+_x7)/2-_x6*(4*v3+_x7)/24;
    rotMat0[  1][  1] = _x6*(_x4+_x3+_x1-12)/48-(_x4+_x3+_x1-4)/4;
    rotMat0[  1][  2] = _x6*(_x10+_x11)/24-(_x10+_x9)/2;
    rotMat0[  2][  0] = (_x8-2*v2)/2-_x6*(_x8-4*v2)/24;
    rotMat0[  2][  1] = (_x12+_x9)/2-_x6*(_x12+_x11)/24;
    rotMat0[  2][  2] = _x6*(_x5+_x2+_x1-12)/48-(_x5+_x2+_x1-4)/4;
 }
void RotationMatrixFromV(double R[3][3], double v[3]) {
    double th = sqrt(v1*v1+v2*v2+v3*v3);
    if (th < THETA) __RotationMatrixFromV0(R, v, th);
    else __RotationMatrixFromV(R, v, th);
}


void __GeneralizedForce(double Q[6], const double v[3], const double th, const double Fr[3], const double r[3]) {
    /*
    ################################################
    #      Variable: Q
    ################################################
    */
    double _x1 = pow(th,-2);
    double _x2 = 0.5*th;
    double _x3 = cos(_x2);
    double _x4 = pow(_x3,2);
    double _x5 = -1.0*_x1*_x4*v1*v2;
    double _x6 = pow(th,-3);
    double _x7 = sin(_x2);
    double _x8 = 2*_x6*_x3*_x7*v1*v2;
    double _x9 = pow(_x7,2);
    double _x10 = 1.0*_x1*_x9*v1*v2;
    double _x11 = 2*_x1*_x9*v3;
    double _x12 = pow(v1,2);
    double _x13 = 2.0*_x6*_x3*_x7*_x12*v3;
    double _x14 = pow(th,-4);
    double _x15 = -4*_x14*_x9*_x12*v3;
    double _x16 = pow(th,-1);
    double _x17 = 2*_x16*_x3*_x7;
    double _x18 = 2.0*_x6*_x3*_x7*v1*v2*v3;
    double _x19 = -4*_x14*_x9*v1*v2*v3;
    double _x20 = -1.0*_x16*_x3*_x7*v1;
    double _x21 = -2*_x1*_x9*v1;
    double _x22 = pow(v1,3);
    double _x23 = -1.0*_x6*_x3*_x7*_x22;
    double _x24 = 2*_x14*_x9*_x22;
    double _x25 = pow(v2,2);
    double _x26 = -1.0*_x6*_x3*_x7*v1*_x25;
    double _x27 = 2*_x14*_x9*v1*_x25;
    double _x28 = pow(v3,2);
    double _x29 = 2*_x1*_x9*v2;
    double _x30 = 2.0*_x6*_x3*_x7*_x12*v2;
    double _x31 = -4*_x14*_x9*_x12*v2;
    double _x32 = 1.0*_x1*_x4*v1*v3;
    double _x33 = -2*_x6*_x3*_x7*v1*v3;
    double _x34 = -1.0*_x1*_x9*v1*v3;
    double _x35 = -2*_x16*_x3*_x7;
    double _x36 = -1.0*_x6*_x3*_x7*v1*_x28;
    double _x37 = 2*_x14*_x9*v1*_x28;
    double _x38 = -1.0*_x1*_x4*v1*v3;
    double _x39 = 2*_x6*_x3*_x7*v1*v3;
    double _x40 = 1.0*_x1*_x9*v1*v3;
    double _x41 = 1.0*_x1*_x4*v1*v2;
    double _x42 = -2*_x6*_x3*_x7*v1*v2;
    double _x43 = -1.0*_x1*_x9*v1*v2;
    double _x44 = 2*_x1*_x9*v1;
    double _x45 = 2.0*_x6*_x3*_x7*_x25*v3;
    double _x46 = -4*_x14*_x9*_x25*v3;
    double _x47 = -1.0*_x16*_x3*_x7*v2;
    double _x48 = -2*_x1*_x9*v2;
    double _x49 = -1.0*_x6*_x3*_x7*_x12*v2;
    double _x50 = 2*_x14*_x9*_x12*v2;
    double _x51 = pow(v2,3);
    double _x52 = -1.0*_x6*_x3*_x7*_x51;
    double _x53 = 2*_x14*_x9*_x51;
    double _x54 = 2.0*_x6*_x3*_x7*v1*_x25;
    double _x55 = -4*_x14*_x9*v1*_x25;
    double _x56 = 1.0*_x1*_x4*v2*v3;
    double _x57 = -2*_x6*_x3*_x7*v2*v3;
    double _x58 = -1.0*_x1*_x9*v2*v3;
    double _x59 = -1.0*_x6*_x3*_x7*v2*_x28;
    double _x60 = 2*_x14*_x9*v2*_x28;
    double _x61 = -1.0*_x1*_x4*v2*v3;
    double _x62 = 2*_x6*_x3*_x7*v2*v3;
    double _x63 = 1.0*_x1*_x9*v2*v3;
    double _x64 = 2.0*_x6*_x3*_x7*v1*_x28;
    double _x65 = -4*_x14*_x9*v1*_x28;
    double _x66 = 2.0*_x6*_x3*_x7*v2*_x28;
    double _x67 = -4*_x14*_x9*v2*_x28;
    double _x68 = -1.0*_x16*_x3*_x7*v3;
    double _x69 = -1.0*_x6*_x3*_x7*_x12*v3;
    double _x70 = 2*_x14*_x9*_x12*v3;
    double _x71 = -1.0*_x6*_x3*_x7*_x25*v3;
    double _x72 = 2*_x14*_x9*_x25*v3;
    double _x73 = pow(v3,3);
    double _x74 = -2*_x1*_x9*v3;
    double _x75 = -1.0*_x6*_x3*_x7*_x73;
    double _x76 = 2*_x14*_x9*_x73;
    Q[  0] = Fr1;
    Q[  1] = Fr2;
    Q[  2] = Fr3;
    Q[  3] = Fr1*(r1*(_x37+_x36+_x27+_x26-2*_x14*_x9*_x22+1.0*_x6*_x3*_x7*_x22+_x44+_x20)+r3*(_x15+_x13+_x11+_x43+_x42+_x41)+r2*(_x40+_x39+_x38+_x31+_x30+_x29))+Fr2*(r2*(_x37+_x36-2*_x14*_x9*v1*_x25+1.0*_x6*_x3*_x7*v1*_x25+_x24+_x23+_x21+_x20)+r3*(_x19+_x18+1.0*_x1*_x9*_x12+2*_x6*_x3*_x7*_x12-1.0*_x1*_x4*_x12+_x35)+r1*(_x34+_x33+_x32+_x31+_x30+_x29))+Fr3*(r3*(-2*_x14*_x9*v1*_x28+1.0*_x6*_x3*_x7*v1*_x28+_x27+_x26+_x24+_x23+_x21+_x20)+r2*(_x19+_x18-1.0*_x1*_x9*_x12-2*_x6*_x3*_x7*_x12+1.0*_x1*_x4*_x12+_x17)+r1*(_x15+_x13+_x11+_x10+_x8+_x5));
    Q[  4] = Fr1*(r1*(_x60+_x59+_x53+_x52-2*_x14*_x9*_x12*v2+1.0*_x6*_x3*_x7*_x12*v2+_x48+_x47)+r3*(_x19+_x18-1.0*_x1*_x9*_x25-2*_x6*_x3*_x7*_x25+1.0*_x1*_x4*_x25+_x17)+r2*(_x63+_x62+_x61+_x55+_x54+_x44))+Fr2*(r2*(_x60+_x59-2*_x14*_x9*_x51+1.0*_x6*_x3*_x7*_x51+_x50+_x49+_x29+_x47)+r3*(_x46+_x45+_x11+_x10+_x8+_x5)+r1*(_x58+_x57+_x56+_x55+_x54+_x44))+Fr3*(r3*(-2*_x14*_x9*v2*_x28+1.0*_x6*_x3*_x7*v2*_x28+_x53+_x52+_x50+_x49+_x48+_x47)+r2*(_x46+_x45+_x11+_x43+_x42+_x41)+r1*(_x19+_x18+1.0*_x1*_x9*_x25+2*_x6*_x3*_x7*_x25-1.0*_x1*_x4*_x25+_x35));
    Q[  5] = Fr1*(r1*(_x76+_x75+_x72+_x71-2*_x14*_x9*_x12*v3+1.0*_x6*_x3*_x7*_x12*v3+_x74+_x68)+r3*(_x65+_x64+_x58+_x57+_x56+_x44)+r2*(1.0*_x1*_x9*_x28+2*_x6*_x3*_x7*_x28-1.0*_x1*_x4*_x28+_x19+_x18+_x35))+Fr2*(r2*(_x76+_x75-2*_x14*_x9*_x25*v3+1.0*_x6*_x3*_x7*_x25*v3+_x70+_x69+_x74+_x68)+r3*(_x67+_x66+_x40+_x39+_x38+_x29)+r1*(-1.0*_x1*_x9*_x28-2*_x6*_x3*_x7*_x28+1.0*_x1*_x4*_x28+_x19+_x18+_x17))+Fr3*(r3*(-2*_x14*_x9*_x73+1.0*_x6*_x3*_x7*_x73+_x72+_x71+_x70+_x69+_x11+_x68)+r2*(_x67+_x66+_x34+_x33+_x32+_x29)+r1*(_x65+_x64+_x63+_x62+_x61+_x44));
}

void __GeneralizedForce0(double Q0[6], const double v[3], const double th, const double Fr[3], const double r[3]) {
    /*
    ################################################
    #      Variable: Q0
    ################################################
    */
    double _x1 = pow(th,2);
    double _x2 = pow(v1,3);
    double _x3 = -r1*_x2;
    double _x4 = 15*r2;
    double _x5 = pow(v1,2);
    double _x6 = -2*r2*_x5;
    double _x7 = pow(v2,2);
    double _x8 = r1*v1*_x7;
    double _x9 = 15*r3*v3;
    double _x10 = -2*r3*_x5*v3;
    double _x11 = 12*r2*v3;
    double _x12 = pow(v3,2);
    double _x13 = r1*_x12;
    double _x14 = r2*_x2;
    double _x15 = -r2*v1*_x7;
    double _x16 = 15*r1;
    double _x17 = -2*r1*_x5;
    double _x18 = -2*r3*v1*v3;
    double _x19 = -12*r1*v3;
    double _x20 = r2*_x12;
    double _x21 = r3*_x2;
    double _x22 = r3*v1*_x7;
    double _x23 = -2*r1*v3;
    double _x24 = _x23-12*r2;
    double _x25 = 12*r1;
    double _x26 = -2*r2*v3;
    double _x27 = _x26+_x25;
    double _x28 = -45*r3;
    double _x29 = -r3*_x12;
    double _x30 = 12*r2;
    double _x31 = 12*r3*v3;
    double _x32 = 8*r2*v3;
    double _x33 = -24*r3;
    double _x34 = -24*r2;
    double _x35 = -8*r1*v3;
    double _x36 = 12*r1*v3;
    double _x37 = _x23-8*r2;
    double _x38 = _x26+8*r1;
    double _x39 = -2*r2*v1;
    double _x40 = pow(v2,3);
    double _x41 = r1*_x40;
    double _x42 = -r1*_x5;
    double _x43 = -r2*_x40;
    double _x44 = _x7*(-2*r3*v3-2*r1*v1);
    double _x45 = r2*_x5;
    double _x46 = r3*_x40;
    double _x47 = r3*_x5;
    double _x48 = -24*r1;
    double _x49 = -r1*_x5*v3;
    double _x50 = r1*_x7*v3;
    double _x51 = -2*r2*v1*v3;
    double _x52 = 12*r3;
    double _x53 = -2*r3*_x12;
    double _x54 = pow(v3,3);
    double _x55 = r1*_x54;
    double _x56 = r2*_x5*v3;
    double _x57 = -r2*_x7*v3;
    double _x58 = -2*r1*v1*v3;
    double _x59 = r2*_x54;
    double _x60 = r3*_x5*v3;
    double _x61 = r3*_x7*v3;
    double _x62 = -2*r1*_x12;
    double _x63 = -2*r2*_x12;
    double _x64 = -r3*_x54;
    double _x65 = 15*r3;
    Q0[  0] = Fr1;
    Q0[  1] = Fr2;
    Q0[  2] = Fr3;
    Q0[  3] = (Fr3*(v1*(_x29+_x33)+v1*v2*_x38+_x5*_x37+_x36+_x22+_x21+24*r2)+Fr2*(v1*(_x20+_x35+_x34)+v2*(_x18+_x17+_x25)+_x15+_x14+8*r3*_x5+_x33)+Fr1*(v1*(_x13+_x32)+_x10+_x31+_x8+(_x6-8*r3*v1+_x30)*v2+_x3))/24-_x1*(Fr3*(v1*(_x29+_x28)+v1*v2*_x27+_x5*_x24+15*r1*v3+_x22+_x21+60*r2)+Fr2*(v1*(_x20+_x19-45*r2)+v2*(_x18+_x17+_x16)+_x15+_x14+12*r3*_x5-60*r3)+Fr1*(v1*(_x13+_x11-15*r1)+_x10+_x9+_x8+(_x6-12*r3*v1+_x4)*v2+_x3))/360;
    Q0[  4] = (Fr3*(v2*(_x29+v1*_x37+_x47+_x33)+_x7*_x38+_x11+_x46+_x48)+Fr2*(v2*(_x20+_x35+_x45+8*r3*v1)+_x44+_x31+_x43+12*r1*v1)+Fr1*(v2*(_x13+_x18+_x32+_x42+_x48)+_x41+(_x39-8*r3)*_x7+12*r2*v1+24*r3))/24-_x1*(Fr3*(v2*(_x29+v1*_x24+_x47+_x28)+_x7*_x27+15*r2*v3+_x46-60*r1)+Fr2*(v2*(_x20+_x19+_x45+12*r3*v1-15*r2)+_x44+_x9+_x43+15*r1*v1)+Fr1*(v2*(_x13+_x18+_x11+_x42-45*r1)+_x41+(_x39-12*r3)*_x7+15*r2*v1+60*r3))/360;
    Q0[  5] = (Fr3*(_x64+v2*(_x63+8*r1*v3+_x30)+v1*(_x62-8*r2*v3+_x25)+_x61+_x60)+Fr2*(_x59+v2*(_x53+_x58+_x52)-8*r1*_x12+_x57+_x56+8*r3*v1*v3-24*r2*v3+24*r1)+Fr1*(_x55+v1*(_x53+_x52)+8*r2*_x12+v2*(_x51-8*r3*v3)+_x50+_x49-24*r1*v3+_x34))/24-_x1*(Fr3*(_x64+v2*(_x63+_x36+_x4)+v1*(_x62-12*r2*v3+_x16)+_x61+_x60-15*r3*v3)+Fr2*(_x59+v2*(_x53+_x58+_x65)-12*r1*_x12+_x57+_x56+12*r3*v1*v3-45*r2*v3+60*r1)+Fr1*(_x55+v1*(_x53+_x65)+12*r2*_x12+v2*(_x51-12*r3*v3)+_x50+_x49-45*r1*v3-60*r2))/360;
}

void GeneralizedForce(double Q[6], const double v[3], const double Fr[3], const double r[3]) {
    double th = sqrt(v1*v1+v2*v2+v3*v3);
    if (th < THETA)
        return __GeneralizedForce0(Q, v, th, Fr, r);
    else
        return __GeneralizedForce(Q, v, th, Fr, r);
}
