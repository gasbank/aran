/*
 * FiberEffectImp.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 *
 * Rigid body equations (single)
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "TypeDefs.h"

/* BODY RELATED VARIABLES (2*nd + 4 = 18) */
#define px body[0]
#define py body[1]
#define pz body[2]
#define qw body[3]
#define qx body[4]
#define qy body[5]
#define qz body[6]
#define pdx body[7]
#define pdy body[8]
#define pdz body[9]
#define qdw body[10]
#define qdx body[11]
#define qdy body[12]
#define qdz body[13]
#define m body[14]
#define Ixx body[15]
#define Iyy body[16]
#define Izz body[17]
/* EXTERNAL FORCE RELATED VARIABLES (6) START INDEX AGAIN!!! */
#define Frx extForce[0]
#define Fry extForce[1]
#define Frz extForce[2]
#define Trx extForce[3]
#define Try extForce[4]
#define Trz extForce[5]


int OneRbImp(const Double_18  body,
             const Double_6   extForce,
             const int        bClearVariable,
             Double_14        yd_R,
             /* Sparse matrix structure of 'dyd_RdY' */
             Uint_196x2       dyd_RdY_keys,
             Double_196       dyd_RdY_values)
{
    if (bClearVariable)
	{
		memset(yd_R, 0, sizeof(Double_14));
		memset(dyd_RdY_keys, 0, sizeof(Uint_196x2));
		memset(dyd_RdY_values, 0, sizeof(Double_196));
    }
    /*;
    ################################################;
    #      Variable: yd_R;
    ################################################;
    */
    double _x1 = pow(m,-1);
    double _x2 = -qdw*qz+qdx*qy-qdy*qx+qdz*qw;
    double _x3 = -qdx*qz-qdw*qy+qdz*qx+qdy*qw;
    double _x4 = qdy*qz-qdz*qy-qdw*qx+qdx*qw;
    double _x5 = qdz*qz+qdy*qy+qdx*qx+qdw*qw;
    double _x6 = pow(Ixx,-1);
    double _x7 = Trx-4*(Izz*_x2*_x3-Iyy*_x2*_x3);
    double _x8 = pow(Iyy,-1);
    double _x9 = Try-4*(Ixx*_x2*_x4-Izz*_x2*_x4);
    double _x10 = pow(Izz,-1);
    double _x11 = Trz-4*(Iyy*_x3*_x4-Ixx*_x3*_x4);
    yd_R[  0] = pdx;
    yd_R[  1] = pdy;
    yd_R[  2] = pdz;
    yd_R[  3] = qdw;
    yd_R[  4] = qdx;
    yd_R[  5] = qdy;
    yd_R[  6] = qdz;
    yd_R[  7] = Frx*_x1;
    yd_R[  8] = Fry*_x1;
    yd_R[  9] = Frz*_x1;
    yd_R[ 10] = 0.5*(-_x10*qz*_x11-_x8*qy*_x9-_x6*qx*_x7)+qdw*_x5-qdx*_x4-qdy*_x3-qdz*_x2;
    yd_R[ 11] = 0.5*(_x10*qy*_x11-_x8*qz*_x9+_x6*qw*_x7)+qdx*_x5+qdw*_x4-qdz*_x3+qdy*_x2;
    yd_R[ 12] = 0.5*(-_x10*qx*_x11+_x8*qw*_x9+_x6*qz*_x7)+qdy*_x5+qdz*_x4+qdw*_x3-qdx*_x2;
    yd_R[ 13] = 0.5*(_x10*qw*_x11+_x8*qx*_x9-_x6*qy*_x7)+qdz*_x5-qdy*_x4+qdx*_x3+qdw*_x2;
    /*;
    ################################################;
    #      Variable: dyd_RdY;
    ################################################;
    */

    _x2 = pow(qdw,2);
    _x3 = pow(qdx,2);
    _x4 = -_x3;
    _x5 = pow(qdy,2);
    _x6 = -_x5;
    _x7 = pow(qdz,2);
    _x8 = -_x7;
    _x9 = pow(Ixx,-1);
    _x10 = -qdw*qz+qdx*qy-qdy*qx+qdz*qw;
    _x11 = -qdx*qz-qdw*qy+qdz*qx+qdy*qw;
    double _x12 = -Iyy*qdz*_x11;
    double _x13 = Izz*qdz*_x11+_x12+Izz*qdy*_x10-Iyy*qdy*_x10;
    double _x14 = pow(Izz,-1);
    double _x15 = qdy*qz-qdz*qy-qdw*qx+qdx*qw;
    double _x16 = -Ixx*qdy*_x15;
    double _x17 = Iyy*qdy*_x15+_x16+Iyy*qdx*_x11-Ixx*qdx*_x11;
    double _x18 = pow(Iyy,-1);
    double _x19 = -Izz*qdx*_x10;
    double _x20 = -Izz*qdz*_x15+Ixx*qdz*_x15+_x19+Ixx*qdx*_x10;
    double _x21 = 2*qdw*qdx;
    double _x22 = Izz*qdz*_x10;
    double _x23 = Iyy*qdy*_x11;
    double _x24 = -Izz*qdy*_x11+_x23+_x22-Iyy*qdz*_x10;
    double _x25 = Izz*qdy*_x15+_x16+Izz*qdw*_x10-Ixx*qdw*_x10;
    double _x26 = Iyy*qdz*_x15-Ixx*qdz*_x15-Iyy*qdw*_x11+Ixx*qdw*_x11;
    double _x27 = Trx-4*(Izz*_x10*_x11-Iyy*_x10*_x11);
    double _x28 = -_x9*_x27;
    double _x29 = 2*qdw*qdy;
    double _x30 = Izz*qdx*_x11-Iyy*qdx*_x11-Izz*qdw*_x10+Iyy*qdw*_x10;
    double _x31 = -Iyy*qdw*_x15+Ixx*qdw*_x15+_x12+Ixx*qdz*_x11;
    double _x32 = Ixx*qdx*_x15;
    double _x33 = -Izz*qdx*_x15+_x32+_x22-Ixx*qdz*_x10;
    double _x34 = Try-4*(Ixx*_x10*_x15-Izz*_x10*_x15);
    double _x35 = -_x18*_x34;
    double _x36 = 2*qdw*qdz;
    double _x37 = -Izz*qdw*_x11+Iyy*qdw*_x11+_x19+Iyy*qdx*_x10;
    double _x38 = Izz*qdw*_x15-Ixx*qdw*_x15-Izz*qdy*_x10+Ixx*qdy*_x10;
    double _x39 = -Iyy*qdx*_x15+_x32+_x23-Ixx*qdy*_x11;
    double _x40 = Trz-4*(Iyy*_x11*_x15-Ixx*_x11*_x15);
    double _x41 = -_x14*_x40;
    double _x42 = 2*qdw*qw;
    double _x43 = 2*qdx*qx;
    double _x44 = 2*qdy*qy;
    double _x45 = 2*qdz*qz;
    double _x46 = Iyy*qz*_x11;
    double _x47 = -Izz*qz*_x11+_x46-Izz*qy*_x10+Iyy*qy*_x10;
    double _x48 = Ixx*qy*_x15;
    double _x49 = -Iyy*qy*_x15+_x48-Iyy*qx*_x11+Ixx*qx*_x11;
    double _x50 = Izz*qx*_x10;
    double _x51 = Izz*qz*_x15-Ixx*qz*_x15+_x50-Ixx*qx*_x10;
    double _x52 = -Izz*qz*_x10;
    double _x53 = -Iyy*qy*_x11;
    double _x54 = Izz*qy*_x11+_x53+_x52+Iyy*qz*_x10;
    double _x55 = -Izz*qy*_x15+_x48-Izz*qw*_x10+Ixx*qw*_x10;
    double _x56 = -Iyy*qz*_x15+Ixx*qz*_x15+Iyy*qw*_x11-Ixx*qw*_x11;
    double _x57 = -Izz*qx*_x11+Iyy*qx*_x11+Izz*qw*_x10-Iyy*qw*_x10;
    double _x58 = Iyy*qw*_x15-Ixx*qw*_x15+_x46-Ixx*qz*_x11;
    double _x59 = -Ixx*qx*_x15;
    double _x60 = Izz*qx*_x15+_x59+_x52+Ixx*qz*_x10;
    double _x61 = Izz*qw*_x11-Iyy*qw*_x11+_x50-Iyy*qx*_x10;
    double _x62 = -Izz*qw*_x15+Ixx*qw*_x15+Izz*qy*_x10-Ixx*qy*_x10;
    double _x63 = Iyy*qx*_x15+_x59+_x53+Ixx*qy*_x11;
    double _x64 = _x9*_x27;
    double _x65 = -_x2;
    double _x66 = 2*qdx*qdy;
    double _x67 = _x14*_x40;
    double _x68 = 2*qdx*qdz;
    double _x69 = _x18*_x34;
    double _x70 = 2*qdy*qdz;

    unsigned int dyd_RdY_count = 0;
    #define dyd_RdY_assign(r,c,v)    \
		dyd_RdY_keys[dyd_RdY_count][0] = r; \
		dyd_RdY_keys[dyd_RdY_count][1] = c; \
		dyd_RdY_values[dyd_RdY_count] = v; \
		++dyd_RdY_count;

    dyd_RdY_assign(  0,  7, 1);
    dyd_RdY_assign(  1,  8, 1);
    dyd_RdY_assign(  2,  9, 1);
    dyd_RdY_assign(  3, 10, 1);
    dyd_RdY_assign(  4, 11, 1);
    dyd_RdY_assign(  5, 12, 1);
    dyd_RdY_assign(  6, 13, 1);

    dyd_RdY_assign( 10,  3, 0.5*(4*_x18*qy*_x20+4*_x14*qz*_x17+4*_x9*qx*_x13)+_x8+_x6+_x4+_x2);
    dyd_RdY_assign( 10,  4, 0.5*(_x28+4*_x14*qz*_x26+4*_x18*qy*_x25+4*_x9*qx*_x24)+_x21);
    dyd_RdY_assign( 10,  5, 0.5*(_x35+4*_x18*qy*_x33+4*_x14*qz*_x31+4*_x9*qx*_x30)+_x29);
    dyd_RdY_assign( 10,  6, 0.5*(_x41+4*_x14*qz*_x39+4*_x18*qy*_x38+4*_x9*qx*_x37)+_x36);
    dyd_RdY_assign( 10, 10, 0.5*(4*_x18*qy*_x51+4*_x14*qz*_x49+4*_x9*qx*_x47)+_x45+_x44+_x43+_x42);
    dyd_RdY_assign( 10, 11, 0.5*(4*_x14*qz*_x56+4*_x18*qy*_x55+4*_x9*qx*_x54)+2*qdw*qx-2*qdx*qw);
    dyd_RdY_assign( 10, 12, 0.5*(4*_x18*qy*_x60+4*_x14*qz*_x58+4*_x9*qx*_x57)+2*qdw*qy-2*qdy*qw);
    dyd_RdY_assign( 10, 13, 0.5*(4*_x14*qz*_x63+4*_x18*qy*_x62+4*_x9*qx*_x61)+2*qdw*qz-2*qdz*qw);
    dyd_RdY_assign( 11,  3, 0.5*(_x64+4*_x18*qz*_x20-4*_x14*qy*_x17-4*_x9*qw*_x13)+_x21);
    dyd_RdY_assign( 11,  4, 0.5*(-4*_x14*qy*_x26+4*_x18*qz*_x25-4*_x9*qw*_x24)+_x8+_x6+_x3+_x65);
    dyd_RdY_assign( 11,  5, 0.5*(_x67+4*_x18*qz*_x33-4*_x14*qy*_x31-4*_x9*qw*_x30)+_x66);
    dyd_RdY_assign( 11,  6, 0.5*(_x35-4*_x14*qy*_x39+4*_x18*qz*_x38-4*_x9*qw*_x37)+_x68);
    dyd_RdY_assign( 11, 10, 0.5*(4*_x18*qz*_x51-4*_x14*qy*_x49-4*_x9*qw*_x47)-2*qdw*qx+2*qdx*qw);
    dyd_RdY_assign( 11, 11, 0.5*(-4*_x14*qy*_x56+4*_x18*qz*_x55-4*_x9*qw*_x54)+_x45+_x44+_x43+_x42);
    dyd_RdY_assign( 11, 12, 0.5*(4*_x18*qz*_x60-4*_x14*qy*_x58-4*_x9*qw*_x57)+2*qdx*qy-2*qdy*qx);
    dyd_RdY_assign( 11, 13, 0.5*(-4*_x14*qy*_x63+4*_x18*qz*_x62-4*_x9*qw*_x61)+2*qdx*qz-2*qdz*qx);
    dyd_RdY_assign( 12,  3, 0.5*(_x69-4*_x18*qw*_x20+4*_x14*qx*_x17-4*_x9*qz*_x13)+_x29);
    dyd_RdY_assign( 12,  4, 0.5*(_x41+4*_x14*qx*_x26-4*_x18*qw*_x25-4*_x9*qz*_x24)+_x66);
    dyd_RdY_assign( 12,  5, 0.5*(-4*_x18*qw*_x33+4*_x14*qx*_x31-4*_x9*qz*_x30)+_x8+_x5+_x4+_x65);
    dyd_RdY_assign( 12,  6, 0.5*(_x64+4*_x14*qx*_x39-4*_x18*qw*_x38-4*_x9*qz*_x37)+_x70);
    dyd_RdY_assign( 12, 10, 0.5*(-4*_x18*qw*_x51+4*_x14*qx*_x49-4*_x9*qz*_x47)-2*qdw*qy+2*qdy*qw);
    dyd_RdY_assign( 12, 11, 0.5*(4*_x14*qx*_x56-4*_x18*qw*_x55-4*_x9*qz*_x54)-2*qdx*qy+2*qdy*qx);
    dyd_RdY_assign( 12, 12, 0.5*(-4*_x18*qw*_x60+4*_x14*qx*_x58-4*_x9*qz*_x57)+_x45+_x44+_x43+_x42);
    dyd_RdY_assign( 12, 13, 0.5*(4*_x14*qx*_x63-4*_x18*qw*_x62-4*_x9*qz*_x61)+2*qdy*qz-2*qdz*qy);
    dyd_RdY_assign( 13,  3, 0.5*(_x67-4*_x18*qx*_x20-4*_x14*qw*_x17+4*_x9*qy*_x13)+_x36);
    dyd_RdY_assign( 13,  4, 0.5*(_x69-4*_x14*qw*_x26-4*_x18*qx*_x25+4*_x9*qy*_x24)+_x68);
    dyd_RdY_assign( 13,  5, 0.5*(_x28-4*_x18*qx*_x33-4*_x14*qw*_x31+4*_x9*qy*_x30)+_x70);
    dyd_RdY_assign( 13,  6, 0.5*(-4*_x14*qw*_x39-4*_x18*qx*_x38+4*_x9*qy*_x37)+_x7+_x6+_x4+_x65);
    dyd_RdY_assign( 13, 10, 0.5*(-4*_x18*qx*_x51-4*_x14*qw*_x49+4*_x9*qy*_x47)-2*qdw*qz+2*qdz*qw);
    dyd_RdY_assign( 13, 11, 0.5*(-4*_x14*qw*_x56-4*_x18*qx*_x55+4*_x9*qy*_x54)-2*qdx*qz+2*qdz*qx);
    dyd_RdY_assign( 13, 12, 0.5*(-4*_x18*qx*_x60-4*_x14*qw*_x58+4*_x9*qy*_x57)-2*qdy*qz+2*qdz*qy);
    dyd_RdY_assign( 13, 13, 0.5*(-4*_x14*qw*_x63-4*_x18*qx*_x62+4*_x9*qy*_x61)+_x45+_x44+_x43+_x42);
    return dyd_RdY_count;
}
