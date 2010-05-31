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
