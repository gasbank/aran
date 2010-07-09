# Autogenerated file
# 2010 Geoyeob Kim
# As a part of the thesis implementation
from numpy import array, linalg, zeros
from math import sin, cos
from scipy import sparse
from MathUtil import cot
import PymConstants

def __dRdv(v, th):
    v1, v2, v3 = v
    dRdv1_s = array(zeros((3,3)))
    dRdv2_s = array(zeros((3,3)))
    dRdv3_s = array(zeros((3,3)))
    ################################################
    #      Variable: dRdv1_s
    ################################################
    _x1 = pow(th,-1)
    _x2 = 0.5*th
    _x3 = cos(_x2)
    _x4 = sin(_x2)
    _x5 = -1.0*_x1*_x3*_x4*v1
    _x6 = pow(th,-2)
    _x7 = pow(_x4,2)
    _x8 = pow(th,-3)
    _x9 = pow(v1,3)
    _x10 = pow(th,-4)
    _x11 = pow(v2,2)
    _x12 = -1.0*_x8*_x3*_x4*v1*_x11
    _x13 = 2*_x10*_x7*v1*_x11
    _x14 = pow(v3,2)
    _x15 = -1.0*_x8*_x3*_x4*v1*_x14
    _x16 = 2*_x10*_x7*v1*_x14
    _x17 = 2*_x6*_x7*v2
    _x18 = pow(v1,2)
    _x19 = 2.0*_x8*_x3*_x4*_x18*v2
    _x20 = -4*_x10*_x7*_x18*v2
    _x21 = pow(_x3,2)
    _x22 = 2*_x6*_x7*v3
    _x23 = 2.0*_x8*_x3*_x4*_x18*v3
    _x24 = -4*_x10*_x7*_x18*v3
    _x25 = -2*_x6*_x7*v1
    _x26 = -1.0*_x8*_x3*_x4*_x9
    _x27 = 2*_x10*_x7*_x9
    _x28 = 2.0*_x8*_x3*_x4*v1*v2*v3
    _x29 = -4*_x10*_x7*v1*v2*v3
    dRdv1_s[  0,  0] = _x16+_x15+_x13+_x12-2*_x10*_x7*_x9+1.0*_x8*_x3*_x4*_x9+2*_x6*_x7*v1+_x5
    dRdv1_s[  0,  1] = 1.0*_x6*_x7*v1*v3+2*_x8*_x3*_x4*v1*v3-1.0*_x6*_x21*v1*v3+_x20+_x19+_x17
    dRdv1_s[  0,  2] = _x24+_x23+_x22-1.0*_x6*_x7*v1*v2-2*_x8*_x3*_x4*v1*v2+1.0*_x6*_x21*v1*v2
    dRdv1_s[  1,  0] = -1.0*_x6*_x7*v1*v3-2*_x8*_x3*_x4*v1*v3+1.0*_x6*_x21*v1*v3+_x20+_x19+_x17
    dRdv1_s[  1,  1] = _x16+_x15-2*_x10*_x7*v1*_x11+1.0*_x8*_x3*_x4*v1*_x11+_x27+_x26+_x25+_x5
    dRdv1_s[  1,  2] = _x29+_x28+1.0*_x6*_x7*_x18+2*_x8*_x3*_x4*_x18-1.0*_x6*_x21*_x18-2*_x1*_x3*_x4
    dRdv1_s[  2,  0] = _x24+_x23+_x22+1.0*_x6*_x7*v1*v2+2*_x8*_x3*_x4*v1*v2-1.0*_x6*_x21*v1*v2
    dRdv1_s[  2,  1] = _x29+_x28-1.0*_x6*_x7*_x18-2*_x8*_x3*_x4*_x18+1.0*_x6*_x21*_x18+2*_x1*_x3*_x4
    dRdv1_s[  2,  2] = -2*_x10*_x7*v1*_x14+1.0*_x8*_x3*_x4*v1*_x14+_x13+_x12+_x27+_x26+_x25+_x5
    ################################################
    #      Variable: dRdv2_s
    ################################################
    _x1 = pow(th,-1)
    _x2 = 0.5*th
    _x3 = cos(_x2)
    _x4 = sin(_x2)
    _x5 = -1.0*_x1*_x3*_x4*v2
    _x6 = pow(th,-2)
    _x7 = pow(_x4,2)
    _x8 = -2*_x6*_x7*v2
    _x9 = pow(th,-3)
    _x10 = pow(v1,2)
    _x11 = pow(th,-4)
    _x12 = pow(v2,3)
    _x13 = -1.0*_x9*_x3*_x4*_x12
    _x14 = 2*_x11*_x7*_x12
    _x15 = pow(v3,2)
    _x16 = -1.0*_x9*_x3*_x4*v2*_x15
    _x17 = 2*_x11*_x7*v2*_x15
    _x18 = 2*_x6*_x7*v1
    _x19 = pow(v2,2)
    _x20 = 2.0*_x9*_x3*_x4*v1*_x19
    _x21 = -4*_x11*_x7*v1*_x19
    _x22 = pow(_x3,2)
    _x23 = 2.0*_x9*_x3*_x4*v1*v2*v3
    _x24 = -4*_x11*_x7*v1*v2*v3
    _x25 = -1.0*_x9*_x3*_x4*_x10*v2
    _x26 = 2*_x11*_x7*_x10*v2
    _x27 = 2*_x6*_x7*v3
    _x28 = 2.0*_x9*_x3*_x4*_x19*v3
    _x29 = -4*_x11*_x7*_x19*v3
    dRdv2_s[  0,  0] = _x17+_x16+_x14+_x13-2*_x11*_x7*_x10*v2+1.0*_x9*_x3*_x4*_x10*v2+_x8+_x5
    dRdv2_s[  0,  1] = 1.0*_x6*_x7*v2*v3+2*_x9*_x3*_x4*v2*v3-1.0*_x6*_x22*v2*v3+_x21+_x20+_x18
    dRdv2_s[  0,  2] = _x24+_x23-1.0*_x6*_x7*_x19-2*_x9*_x3*_x4*_x19+1.0*_x6*_x22*_x19+2*_x1*_x3*_x4
    dRdv2_s[  1,  0] = -1.0*_x6*_x7*v2*v3-2*_x9*_x3*_x4*v2*v3+1.0*_x6*_x22*v2*v3+_x21+_x20+_x18
    dRdv2_s[  1,  1] = _x17+_x16-2*_x11*_x7*_x12+1.0*_x9*_x3*_x4*_x12+_x26+_x25+2*_x6*_x7*v2+_x5
    dRdv2_s[  1,  2] = _x29+_x28+_x27+1.0*_x6*_x7*v1*v2+2*_x9*_x3*_x4*v1*v2-1.0*_x6*_x22*v1*v2
    dRdv2_s[  2,  0] = _x24+_x23+1.0*_x6*_x7*_x19+2*_x9*_x3*_x4*_x19-1.0*_x6*_x22*_x19-2*_x1*_x3*_x4
    dRdv2_s[  2,  1] = _x29+_x28+_x27-1.0*_x6*_x7*v1*v2-2*_x9*_x3*_x4*v1*v2+1.0*_x6*_x22*v1*v2
    dRdv2_s[  2,  2] = -2*_x11*_x7*v2*_x15+1.0*_x9*_x3*_x4*v2*_x15+_x14+_x13+_x26+_x25+_x8+_x5
    ################################################
    #      Variable: dRdv3_s
    ################################################
    _x1 = pow(th,-1)
    _x2 = 0.5*th
    _x3 = cos(_x2)
    _x4 = sin(_x2)
    _x5 = -1.0*_x1*_x3*_x4*v3
    _x6 = pow(th,-2)
    _x7 = pow(_x4,2)
    _x8 = -2*_x6*_x7*v3
    _x9 = pow(th,-3)
    _x10 = pow(v1,2)
    _x11 = pow(th,-4)
    _x12 = pow(v2,2)
    _x13 = -1.0*_x9*_x3*_x4*_x12*v3
    _x14 = 2*_x11*_x7*_x12*v3
    _x15 = pow(v3,3)
    _x16 = -1.0*_x9*_x3*_x4*_x15
    _x17 = 2*_x11*_x7*_x15
    _x18 = 2.0*_x9*_x3*_x4*v1*v2*v3
    _x19 = -4*_x11*_x7*v1*v2*v3
    _x20 = pow(_x3,2)
    _x21 = pow(v3,2)
    _x22 = 2*_x6*_x7*v1
    _x23 = 2.0*_x9*_x3*_x4*v1*_x21
    _x24 = -4*_x11*_x7*v1*_x21
    _x25 = -1.0*_x9*_x3*_x4*_x10*v3
    _x26 = 2*_x11*_x7*_x10*v3
    _x27 = 2*_x6*_x7*v2
    _x28 = 2.0*_x9*_x3*_x4*v2*_x21
    _x29 = -4*_x11*_x7*v2*_x21
    dRdv3_s[  0,  0] = _x17+_x16+_x14+_x13-2*_x11*_x7*_x10*v3+1.0*_x9*_x3*_x4*_x10*v3+_x8+_x5
    dRdv3_s[  0,  1] = 1.0*_x6*_x7*_x21+2*_x9*_x3*_x4*_x21-1.0*_x6*_x20*_x21+_x19+_x18-2*_x1*_x3*_x4
    dRdv3_s[  0,  2] = _x24+_x23-1.0*_x6*_x7*v2*v3-2*_x9*_x3*_x4*v2*v3+1.0*_x6*_x20*v2*v3+_x22
    dRdv3_s[  1,  0] = -1.0*_x6*_x7*_x21-2*_x9*_x3*_x4*_x21+1.0*_x6*_x20*_x21+_x19+_x18+2*_x1*_x3*_x4
    dRdv3_s[  1,  1] = _x17+_x16-2*_x11*_x7*_x12*v3+1.0*_x9*_x3*_x4*_x12*v3+_x26+_x25+_x8+_x5
    dRdv3_s[  1,  2] = _x29+_x28+1.0*_x6*_x7*v1*v3+2*_x9*_x3*_x4*v1*v3-1.0*_x6*_x20*v1*v3+_x27
    dRdv3_s[  2,  0] = _x24+_x23+1.0*_x6*_x7*v2*v3+2*_x9*_x3*_x4*v2*v3-1.0*_x6*_x20*v2*v3+_x22
    dRdv3_s[  2,  1] = _x29+_x28-1.0*_x6*_x7*v1*v3-2*_x9*_x3*_x4*v1*v3+1.0*_x6*_x20*v1*v3+_x27
    dRdv3_s[  2,  2] = -2*_x11*_x7*_x15+1.0*_x9*_x3*_x4*_x15+_x14+_x13+_x26+_x25+2*_x6*_x7*v3+_x5
    return dRdv1_s, dRdv2_s, dRdv3_s

def __dRdv0(v, th):
    v1, v2, v3 = v
    dRdv1_s0 = array(zeros((3,3)))
    dRdv2_s0 = array(zeros((3,3)))
    dRdv3_s0 = array(zeros((3,3)))
    ################################################
    #      Variable: dRdv1_s0
    ################################################
    _x1 = pow(v1,3)
    _x2 = pow(v2,2)
    _x3 = -_x2
    _x4 = pow(v3,2)
    _x5 = -_x4
    _x6 = pow(th,2)
    _x7 = -15*v2
    _x8 = pow(v1,2)
    _x9 = 2*_x8*v2
    _x10 = -6*v2
    _x11 = _x8*v2
    _x12 = -6*v3
    _x13 = _x8*v3
    _x14 = -15*v3
    _x15 = 2*_x8*v3
    _x16 = 4*_x8
    _x17 = -v1*v2*v3
    _x18 = 6*_x8
    _x19 = v1*v2*v3
    dRdv1_s0[  0,  0] = _x6*(v1*(_x5+_x3+15)+_x1)/360-(v1*(_x5+_x3)+_x1)/24
    dRdv1_s0[  0,  1] = _x6*(-12*v1*v3+_x9+_x7)/360-(-4*v1*v3+_x11+_x10)/12
    dRdv1_s0[  0,  2] = _x6*(_x15+_x14+12*v1*v2)/360-(_x13+_x12+4*v1*v2)/12
    dRdv1_s0[  1,  0] = _x6*(12*v1*v3+_x9+_x7)/360-(4*v1*v3+_x11+_x10)/12
    dRdv1_s0[  1,  1] = (v1*(_x4+_x3-24)+_x1)/24-_x6*(v1*(_x4+_x3-45)+_x1)/360
    dRdv1_s0[  1,  2] = (_x17+_x16-12)/12-_x6*(_x17+_x18-30)/180
    dRdv1_s0[  2,  0] = _x6*(_x15+_x14-12*v1*v2)/360-(_x13+_x12-4*v1*v2)/12
    dRdv1_s0[  2,  1] = _x6*(_x19+_x18-30)/180-(_x19+_x16-12)/12
    dRdv1_s0[  2,  2] = (v1*(_x5+_x2-24)+_x1)/24-_x6*(v1*(_x5+_x2-45)+_x1)/360
    ################################################
    #      Variable: dRdv2_s0
    ################################################
    _x1 = pow(th,2)
    _x2 = pow(v2,3)
    _x3 = pow(v1,2)
    _x4 = -_x3
    _x5 = pow(v3,2)
    _x6 = -15*v1
    _x7 = pow(v2,2)
    _x8 = 2*v1*_x7
    _x9 = -6*v1
    _x10 = v1*_x7
    _x11 = 4*_x7
    _x12 = v1*v2*v3
    _x13 = 6*_x7
    _x14 = -_x5
    _x15 = -6*v3
    _x16 = _x7*v3
    _x17 = -15*v3
    _x18 = 2*_x7*v3
    _x19 = -v1*v2*v3
    dRdv2_s0[  0,  0] = (v2*(_x5+_x4-24)+_x2)/24-_x1*(v2*(_x5+_x4-45)+_x2)/360
    dRdv2_s0[  0,  1] = _x1*(-12*v2*v3+_x8+_x6)/360-(-4*v2*v3+_x10+_x9)/12
    dRdv2_s0[  0,  2] = _x1*(_x12+_x13-30)/180-(_x12+_x11-12)/12
    dRdv2_s0[  1,  0] = _x1*(12*v2*v3+_x8+_x6)/360-(4*v2*v3+_x10+_x9)/12
    dRdv2_s0[  1,  1] = _x1*(v2*(_x14+_x4+15)+_x2)/360-(v2*(_x14+_x4)+_x2)/24
    dRdv2_s0[  1,  2] = _x1*(_x18+_x17-12*v1*v2)/360-(_x16+_x15-4*v1*v2)/12
    dRdv2_s0[  2,  0] = (_x19+_x11-12)/12-_x1*(_x19+_x13-30)/180
    dRdv2_s0[  2,  1] = _x1*(_x18+_x17+12*v1*v2)/360-(_x16+_x15+4*v1*v2)/12
    dRdv2_s0[  2,  2] = (v2*(_x14+_x3-24)+_x2)/24-_x1*(v2*(_x14+_x3-45)+_x2)/360
    ################################################
    #      Variable: dRdv3_s0
    ################################################
    _x1 = pow(th,2)
    _x2 = pow(v1,2)
    _x3 = -_x2
    _x4 = pow(v2,2)
    _x5 = pow(v3,3)
    _x6 = -v1*v2*v3
    _x7 = pow(v3,2)
    _x8 = 4*_x7
    _x9 = 6*_x7
    _x10 = -6*v1
    _x11 = v1*_x7
    _x12 = -15*v1
    _x13 = 2*v1*_x7
    _x14 = v1*v2*v3
    _x15 = -_x4
    _x16 = -6*v2
    _x17 = v2*_x7
    _x18 = -15*v2
    _x19 = 2*v2*_x7
    dRdv3_s0[  0,  0] = (_x5+(_x4+_x3-24)*v3)/24-_x1*(_x5+(_x4+_x3-45)*v3)/360
    dRdv3_s0[  0,  1] = (_x8+_x6-12)/12-_x1*(_x9+_x6-30)/180
    dRdv3_s0[  0,  2] = _x1*(_x13+12*v2*v3+_x12)/360-(_x11+4*v2*v3+_x10)/12
    dRdv3_s0[  1,  0] = _x1*(_x9+_x14-30)/180-(_x8+_x14-12)/12
    dRdv3_s0[  1,  1] = (_x5+(_x15+_x2-24)*v3)/24-_x1*(_x5+(_x15+_x2-45)*v3)/360
    dRdv3_s0[  1,  2] = _x1*(_x19-12*v1*v3+_x18)/360-(_x17-4*v1*v3+_x16)/12
    dRdv3_s0[  2,  0] = _x1*(_x13-12*v2*v3+_x12)/360-(_x11-4*v2*v3+_x10)/12
    dRdv3_s0[  2,  1] = _x1*(_x19+12*v1*v3+_x18)/360-(_x17+4*v1*v3+_x16)/12
    dRdv3_s0[  2,  2] = _x1*(_x5+(_x15+_x3+15)*v3)/360-(_x5+(_x15+_x3)*v3)/24
    return dRdv1_s0, dRdv2_s0, dRdv3_s0

def dRdv(v, omega):
    assert len(v) == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA(): return __dRdv0(v, th)
    else         : return __dRdv(v, th)

def __dfxdX(p, v, th, pc):
    p1, p2, p3 = p
    v1, v2, v3 = v
    pc1, pc2, pc3 = pc
    dfxdX = array(zeros(6))
    ################################################
    #      Variable: dfxdX
    ################################################
    _x1 = pow(th,-2)
    _x2 = 0.5*th
    _x3 = cos(_x2)
    _x4 = pow(_x3,2)
    _x5 = pow(th,-3)
    _x6 = sin(_x2)
    _x7 = pow(_x6,2)
    _x8 = 2*_x1*_x7*v3
    _x9 = pow(v1,2)
    _x10 = pow(th,-4)
    _x11 = pow(th,-1)
    _x12 = 2.0*_x5*_x3*_x6*v1*v2*v3
    _x13 = -4*_x10*_x7*v1*v2*v3
    _x14 = pow(v1,3)
    _x15 = pow(v2,2)
    _x16 = pow(v3,2)
    _x17 = pow(v2,3)
    _x18 = pow(v3,3)
    dfxdX[  2] = 1
    dfxdX[  3] = pc3*(-2*_x10*_x7*v1*_x16+1.0*_x5*_x3*_x6*v1*_x16+2*_x10*_x7*v1*_x15-1.0*_x5*_x3*_x6*v1*_x15+2*_x10*_x7*_x14-1.0*_x5*_x3*_x6*_x14-2*_x1*_x7*v1-1.0*_x11*_x3*_x6*v1)+pc2*(_x13+_x12-1.0*_x1*_x7*_x9-2*_x5*_x3*_x6*_x9+1.0*_x1*_x4*_x9+2*_x11*_x3*_x6)+pc1*(-4*_x10*_x7*_x9*v3+2.0*_x5*_x3*_x6*_x9*v3+_x8+1.0*_x1*_x7*v1*v2+2*_x5*_x3*_x6*v1*v2-1.0*_x1*_x4*v1*v2)
    dfxdX[  4] = pc3*(-2*_x10*_x7*v2*_x16+1.0*_x5*_x3*_x6*v2*_x16+2*_x10*_x7*_x17-1.0*_x5*_x3*_x6*_x17+2*_x10*_x7*_x9*v2-1.0*_x5*_x3*_x6*_x9*v2-2*_x1*_x7*v2-1.0*_x11*_x3*_x6*v2)+pc2*(-4*_x10*_x7*_x15*v3+2.0*_x5*_x3*_x6*_x15*v3+_x8-1.0*_x1*_x7*v1*v2-2*_x5*_x3*_x6*v1*v2+1.0*_x1*_x4*v1*v2)+pc1*(_x13+_x12+1.0*_x1*_x7*_x15+2*_x5*_x3*_x6*_x15-1.0*_x1*_x4*_x15-2*_x11*_x3*_x6)
    dfxdX[  5] = pc3*(-2*_x10*_x7*_x18+1.0*_x5*_x3*_x6*_x18+2*_x10*_x7*_x15*v3-1.0*_x5*_x3*_x6*_x15*v3+2*_x10*_x7*_x9*v3-1.0*_x5*_x3*_x6*_x9*v3+_x8-1.0*_x11*_x3*_x6*v3)+pc2*(-4*_x10*_x7*v2*_x16+2.0*_x5*_x3*_x6*v2*_x16-1.0*_x1*_x7*v1*v3-2*_x5*_x3*_x6*v1*v3+1.0*_x1*_x4*v1*v3+2*_x1*_x7*v2)+pc1*(-4*_x10*_x7*v1*_x16+2.0*_x5*_x3*_x6*v1*_x16+1.0*_x1*_x7*v2*v3+2*_x5*_x3*_x6*v2*v3-1.0*_x1*_x4*v2*v3+2*_x1*_x7*v1)
    return dfxdX

def __dfxdX0(p, v, th, pc):
    p1, p2, p3 = p
    v1, v2, v3 = v
    pc1, pc2, pc3 = pc
    dfxdX0 = array(zeros(6))
    ################################################
    #      Variable: dfxdX0
    ################################################
    _x1 = pow(th,2)
    _x2 = pow(v1,2)
    _x3 = -pc3*pow(v1,3)
    _x4 = pow(v2,2)
    _x5 = -pc3*v1*_x4
    _x6 = 2*pc1*_x2
    _x7 = 2*pc2*v1*v2
    _x8 = pow(v3,2)
    _x9 = pc3*v1*_x8
    _x10 = 12*pc2*v1
    _x11 = -pc3*_x2
    _x12 = -pc3*pow(v2,3)
    _x13 = 2*pc1*v1*v2
    _x14 = 2*pc2*_x4
    _x15 = pc3*v2*_x8
    _x16 = 8*pc2*v1
    _x17 = -pc3*_x4
    _x18 = (2*pc2*v2+2*pc1*v1)*_x8
    _x19 = pc3*pow(v3,3)
    dfxdX0[  2] = 1
    dfxdX0[  3] = _x1*(_x9+(_x7+_x6-15*pc1)*v3+_x5-12*pc1*v1*v2+_x3+12*pc2*_x2+45*pc3*v1-60*pc2)/360-(_x9+(_x7+_x6-12*pc1)*v3+_x5-8*pc1*v1*v2+_x3+8*pc2*_x2+24*pc3*v1-24*pc2)/24
    dfxdX0[  4] = _x1*(_x15+(_x14+_x13-15*pc2)*v3+_x12-12*pc1*_x4+(_x11+_x10+45*pc3)*v2+60*pc1)/360-(_x15+(_x14+_x13-12*pc2)*v3+_x12-8*pc1*_x4+(_x11+_x16+24*pc3)*v2+24*pc1)/24
    dfxdX0[  5] = _x1*(_x19+_x18+(_x17-12*pc1*v2+_x11+_x10+15*pc3)*v3-15*pc2*v2-15*pc1*v1)/360-(_x19+_x18+(_x17-8*pc1*v2+_x11+_x16)*v3-12*pc2*v2-12*pc1*v1)/24
    return dfxdX0

def dfxdX(p, v, pc):
    assert len(v) == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA: return __dfxdX0(p, v, th, pc)
    else         : return __dfxdX(p, v, th, pc)

def __RotationMatrixFromV(v, th):
    v1, v2, v3 = v
    rotMat = array(zeros((3,3)))
    ################################################
    #      Variable: rotMat
    ################################################
    _x1 = pow(v1,2)
    _x2 = pow(v2,2)
    _x3 = pow(v3,2)
    _x4 = _x3+_x2+_x1
    _x5 = pow(_x4,1/2)
    _x6 = 0.5*_x5
    _x7 = cos(_x6)
    _x8 = pow(_x7,2)
    _x9 = pow(_x4,-1)
    _x10 = sin(_x6)
    _x11 = pow(_x10,2)
    _x12 = -_x2*_x9*_x11
    _x13 = -_x3*_x9*_x11
    _x14 = pow(_x5,-1)
    _x15 = 2*v1*v2*_x9*_x11
    _x16 = 2*v1*v3*_x9*_x11
    _x17 = -_x1*_x9*_x11
    _x18 = 2*v2*v3*_x9*_x11
    rotMat[  0,  0] = _x13+_x12+_x1*_x9*_x11+_x8
    rotMat[  0,  1] = _x15-2*v3*_x14*_x7*_x10
    rotMat[  0,  2] = _x16+2*v2*_x14*_x7*_x10
    rotMat[  1,  0] = _x15+2*v3*_x14*_x7*_x10
    rotMat[  1,  1] = _x13+_x2*_x9*_x11+_x17+_x8
    rotMat[  1,  2] = _x18-2*v1*_x14*_x7*_x10
    rotMat[  2,  0] = _x16-2*v2*_x14*_x7*_x10
    rotMat[  2,  1] = _x18+2*v1*_x14*_x7*_x10
    rotMat[  2,  2] = _x3*_x9*_x11+_x12+_x17+_x8
    return rotMat

def __RotationMatrixFromV0(v, th):
    v1, v2, v3 = v
    rotMat0 = array(zeros((3,3)))
    ################################################
    #      Variable: rotMat0
    ################################################
    _x1 = pow(v1,2)
    _x2 = pow(v2,2)
    _x3 = -_x2
    _x4 = pow(v3,2)
    _x5 = -_x4
    _x6 = pow(th,2)
    _x7 = v1*v2
    _x8 = v1*v3
    _x9 = 2*v1
    _x10 = -v2*v3
    _x11 = 4*v1
    _x12 = v2*v3
    rotMat0[  0,  0] = (_x5+_x3+_x1+4)/4-_x6*(_x5+_x3+_x1+12)/48
    rotMat0[  0,  1] = (_x7-2*v3)/2-_x6*(_x7-4*v3)/24
    rotMat0[  0,  2] = (_x8+2*v2)/2-_x6*(_x8+4*v2)/24
    rotMat0[  1,  0] = (2*v3+_x7)/2-_x6*(4*v3+_x7)/24
    rotMat0[  1,  1] = _x6*(_x4+_x3+_x1-12)/48-(_x4+_x3+_x1-4)/4
    rotMat0[  1,  2] = _x6*(_x10+_x11)/24-(_x10+_x9)/2
    rotMat0[  2,  0] = (_x8-2*v2)/2-_x6*(_x8-4*v2)/24
    rotMat0[  2,  1] = (_x12+_x9)/2-_x6*(_x12+_x11)/24
    rotMat0[  2,  2] = _x6*(_x5+_x2+_x1-12)/48-(_x5+_x2+_x1-4)/4
    return rotMat0

def RotationMatrixFromV(v):
    assert len(v) == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA(): return __RotationMatrixFromV0(v, th)
    else         : return __RotationMatrixFromV(v, th)

def __VdotFromOmega(v, th, omega):
    v1, v2, v3 = v
    omega1, omega2, omega3 = omega
    vd = array(zeros(3))
    ################################################
    #      Variable: vd
    ################################################
    _x1 = cot(th/2)
    _x2 = pow(th,-1)
    _x3 = _x1-2*_x2
    _x4 = omega3*v3+omega2*v2+omega1*v1
    vd[  0] = (-_x2*_x3*v1*_x4+omega2*v3-omega3*v2+omega1*th*_x1)/2
    vd[  1] = (-_x2*_x3*v2*_x4-omega1*v3+omega3*v1+omega2*th*_x1)/2
    vd[  2] = (-_x2*_x3*v3*_x4+omega1*v2-omega2*v1+omega3*th*_x1)/2
    return vd

def __VdotFromOmega0(v, th, omega):
    v1, v2, v3 = v
    omega1, omega2, omega3 = omega
    vd0 = array(zeros(3))
    ################################################
    #      Variable: vd0
    ################################################
    _x1 = pow(th,2)
    _x2 = pow(v1,2)
    _x3 = omega2*v1*v2
    _x4 = v1*v3
    _x5 = omega2*pow(v2,2)
    _x6 = omega2*v2*v3
    _x7 = pow(v3,2)
    vd0[  0] = (omega3*(_x4-6*v2)+6*omega2*v3+_x3+omega1*(_x2+12))/12+_x1*(omega3*v1*v3+_x3+omega1*(_x2-60))/720
    vd0[  1] = (omega3*(v2*v3+6*v1)+omega1*(v1*v2-6*v3)+_x5+12*omega2)/12+_x1*(omega3*v2*v3+_x5+omega1*v1*v2-60*omega2)/720
    vd0[  2] = (omega3*(_x7+12)+omega1*(_x4+6*v2)+_x6-6*omega2*v1)/12+_x1*(omega3*(_x7-60)+_x6+omega1*v1*v3)/720
    return vd0

def VdotFromOmega(v, omega):
    assert len(v) == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA(): return __VdotFromOmega0(v, th, omega)
    else         : return __VdotFromOmega(v, th, omega)

def __GeneralizedForce(v, th, Fr, r):
    v1, v2, v3 = v
    Fr1, Fr2, Fr3 = Fr
    r1, r2, r3 = r
    Q = array(zeros(6))
    ################################################
    #      Variable: Q
    ################################################
    _x1 = pow(th,-2)
    _x2 = 0.5*th
    _x3 = cos(_x2)
    _x4 = pow(_x3,2)
    _x5 = -1.0*_x1*_x4*v1*v2
    _x6 = pow(th,-3)
    _x7 = sin(_x2)
    _x8 = 2*_x6*_x3*_x7*v1*v2
    _x9 = pow(_x7,2)
    _x10 = 1.0*_x1*_x9*v1*v2
    _x11 = 2*_x1*_x9*v3
    _x12 = pow(v1,2)
    _x13 = 2.0*_x6*_x3*_x7*_x12*v3
    _x14 = pow(th,-4)
    _x15 = -4*_x14*_x9*_x12*v3
    _x16 = pow(th,-1)
    _x17 = 2*_x16*_x3*_x7
    _x18 = 2.0*_x6*_x3*_x7*v1*v2*v3
    _x19 = -4*_x14*_x9*v1*v2*v3
    _x20 = -1.0*_x16*_x3*_x7*v1
    _x21 = -2*_x1*_x9*v1
    _x22 = pow(v1,3)
    _x23 = -1.0*_x6*_x3*_x7*_x22
    _x24 = 2*_x14*_x9*_x22
    _x25 = pow(v2,2)
    _x26 = -1.0*_x6*_x3*_x7*v1*_x25
    _x27 = 2*_x14*_x9*v1*_x25
    _x28 = pow(v3,2)
    _x29 = 2*_x1*_x9*v2
    _x30 = 2.0*_x6*_x3*_x7*_x12*v2
    _x31 = -4*_x14*_x9*_x12*v2
    _x32 = 1.0*_x1*_x4*v1*v3
    _x33 = -2*_x6*_x3*_x7*v1*v3
    _x34 = -1.0*_x1*_x9*v1*v3
    _x35 = -2*_x16*_x3*_x7
    _x36 = -1.0*_x6*_x3*_x7*v1*_x28
    _x37 = 2*_x14*_x9*v1*_x28
    _x38 = -1.0*_x1*_x4*v1*v3
    _x39 = 2*_x6*_x3*_x7*v1*v3
    _x40 = 1.0*_x1*_x9*v1*v3
    _x41 = 1.0*_x1*_x4*v1*v2
    _x42 = -2*_x6*_x3*_x7*v1*v2
    _x43 = -1.0*_x1*_x9*v1*v2
    _x44 = 2*_x1*_x9*v1
    _x45 = 2.0*_x6*_x3*_x7*_x25*v3
    _x46 = -4*_x14*_x9*_x25*v3
    _x47 = -1.0*_x16*_x3*_x7*v2
    _x48 = -2*_x1*_x9*v2
    _x49 = -1.0*_x6*_x3*_x7*_x12*v2
    _x50 = 2*_x14*_x9*_x12*v2
    _x51 = pow(v2,3)
    _x52 = -1.0*_x6*_x3*_x7*_x51
    _x53 = 2*_x14*_x9*_x51
    _x54 = 2.0*_x6*_x3*_x7*v1*_x25
    _x55 = -4*_x14*_x9*v1*_x25
    _x56 = 1.0*_x1*_x4*v2*v3
    _x57 = -2*_x6*_x3*_x7*v2*v3
    _x58 = -1.0*_x1*_x9*v2*v3
    _x59 = -1.0*_x6*_x3*_x7*v2*_x28
    _x60 = 2*_x14*_x9*v2*_x28
    _x61 = -1.0*_x1*_x4*v2*v3
    _x62 = 2*_x6*_x3*_x7*v2*v3
    _x63 = 1.0*_x1*_x9*v2*v3
    _x64 = 2.0*_x6*_x3*_x7*v1*_x28
    _x65 = -4*_x14*_x9*v1*_x28
    _x66 = 2.0*_x6*_x3*_x7*v2*_x28
    _x67 = -4*_x14*_x9*v2*_x28
    _x68 = -1.0*_x16*_x3*_x7*v3
    _x69 = -1.0*_x6*_x3*_x7*_x12*v3
    _x70 = 2*_x14*_x9*_x12*v3
    _x71 = -1.0*_x6*_x3*_x7*_x25*v3
    _x72 = 2*_x14*_x9*_x25*v3
    _x73 = pow(v3,3)
    _x74 = -2*_x1*_x9*v3
    _x75 = -1.0*_x6*_x3*_x7*_x73
    _x76 = 2*_x14*_x9*_x73
    Q[  0] = Fr1
    Q[  1] = Fr2
    Q[  2] = Fr3
    Q[  3] = Fr1*(r1*(_x37+_x36+_x27+_x26-2*_x14*_x9*_x22+1.0*_x6*_x3*_x7*_x22+_x44+_x20)+r3*(_x15+_x13+_x11+_x43+_x42+_x41)+r2*(_x40+_x39+_x38+_x31+_x30+_x29))+Fr2*(r2*(_x37+_x36-2*_x14*_x9*v1*_x25+1.0*_x6*_x3*_x7*v1*_x25+_x24+_x23+_x21+_x20)+r3*(_x19+_x18+1.0*_x1*_x9*_x12+2*_x6*_x3*_x7*_x12-1.0*_x1*_x4*_x12+_x35)+r1*(_x34+_x33+_x32+_x31+_x30+_x29))+Fr3*(r3*(-2*_x14*_x9*v1*_x28+1.0*_x6*_x3*_x7*v1*_x28+_x27+_x26+_x24+_x23+_x21+_x20)+r2*(_x19+_x18-1.0*_x1*_x9*_x12-2*_x6*_x3*_x7*_x12+1.0*_x1*_x4*_x12+_x17)+r1*(_x15+_x13+_x11+_x10+_x8+_x5))
    Q[  4] = Fr1*(r1*(_x60+_x59+_x53+_x52-2*_x14*_x9*_x12*v2+1.0*_x6*_x3*_x7*_x12*v2+_x48+_x47)+r3*(_x19+_x18-1.0*_x1*_x9*_x25-2*_x6*_x3*_x7*_x25+1.0*_x1*_x4*_x25+_x17)+r2*(_x63+_x62+_x61+_x55+_x54+_x44))+Fr2*(r2*(_x60+_x59-2*_x14*_x9*_x51+1.0*_x6*_x3*_x7*_x51+_x50+_x49+_x29+_x47)+r3*(_x46+_x45+_x11+_x10+_x8+_x5)+r1*(_x58+_x57+_x56+_x55+_x54+_x44))+Fr3*(r3*(-2*_x14*_x9*v2*_x28+1.0*_x6*_x3*_x7*v2*_x28+_x53+_x52+_x50+_x49+_x48+_x47)+r2*(_x46+_x45+_x11+_x43+_x42+_x41)+r1*(_x19+_x18+1.0*_x1*_x9*_x25+2*_x6*_x3*_x7*_x25-1.0*_x1*_x4*_x25+_x35))
    Q[  5] = Fr1*(r1*(_x76+_x75+_x72+_x71-2*_x14*_x9*_x12*v3+1.0*_x6*_x3*_x7*_x12*v3+_x74+_x68)+r3*(_x65+_x64+_x58+_x57+_x56+_x44)+r2*(1.0*_x1*_x9*_x28+2*_x6*_x3*_x7*_x28-1.0*_x1*_x4*_x28+_x19+_x18+_x35))+Fr2*(r2*(_x76+_x75-2*_x14*_x9*_x25*v3+1.0*_x6*_x3*_x7*_x25*v3+_x70+_x69+_x74+_x68)+r3*(_x67+_x66+_x40+_x39+_x38+_x29)+r1*(-1.0*_x1*_x9*_x28-2*_x6*_x3*_x7*_x28+1.0*_x1*_x4*_x28+_x19+_x18+_x17))+Fr3*(r3*(-2*_x14*_x9*_x73+1.0*_x6*_x3*_x7*_x73+_x72+_x71+_x70+_x69+_x11+_x68)+r2*(_x67+_x66+_x34+_x33+_x32+_x29)+r1*(_x65+_x64+_x63+_x62+_x61+_x44))
    return Q

def __GeneralizedForce0(v, th, Fr, r):
    v1, v2, v3 = v
    Fr1, Fr2, Fr3 = Fr
    r1, r2, r3 = r
    Q0 = array(zeros(6))
    ################################################
    #      Variable: Q0
    ################################################
    _x1 = pow(th,2)
    _x2 = pow(v1,3)
    _x3 = -r1*_x2
    _x4 = 15*r2
    _x5 = pow(v1,2)
    _x6 = -2*r2*_x5
    _x7 = pow(v2,2)
    _x8 = r1*v1*_x7
    _x9 = 15*r3*v3
    _x10 = -2*r3*_x5*v3
    _x11 = 12*r2*v3
    _x12 = pow(v3,2)
    _x13 = r1*_x12
    _x14 = r2*_x2
    _x15 = -r2*v1*_x7
    _x16 = 15*r1
    _x17 = -2*r1*_x5
    _x18 = -2*r3*v1*v3
    _x19 = -12*r1*v3
    _x20 = r2*_x12
    _x21 = r3*_x2
    _x22 = r3*v1*_x7
    _x23 = -2*r1*v3
    _x24 = _x23-12*r2
    _x25 = 12*r1
    _x26 = -2*r2*v3
    _x27 = _x26+_x25
    _x28 = -45*r3
    _x29 = -r3*_x12
    _x30 = 12*r2
    _x31 = 12*r3*v3
    _x32 = 8*r2*v3
    _x33 = -24*r3
    _x34 = -24*r2
    _x35 = -8*r1*v3
    _x36 = 12*r1*v3
    _x37 = _x23-8*r2
    _x38 = _x26+8*r1
    _x39 = -2*r2*v1
    _x40 = pow(v2,3)
    _x41 = r1*_x40
    _x42 = -r1*_x5
    _x43 = -r2*_x40
    _x44 = _x7*(-2*r3*v3-2*r1*v1)
    _x45 = r2*_x5
    _x46 = r3*_x40
    _x47 = r3*_x5
    _x48 = -24*r1
    _x49 = -r1*_x5*v3
    _x50 = r1*_x7*v3
    _x51 = -2*r2*v1*v3
    _x52 = 12*r3
    _x53 = -2*r3*_x12
    _x54 = pow(v3,3)
    _x55 = r1*_x54
    _x56 = r2*_x5*v3
    _x57 = -r2*_x7*v3
    _x58 = -2*r1*v1*v3
    _x59 = r2*_x54
    _x60 = r3*_x5*v3
    _x61 = r3*_x7*v3
    _x62 = -2*r1*_x12
    _x63 = -2*r2*_x12
    _x64 = -r3*_x54
    _x65 = 15*r3
    Q0[  0] = Fr1
    Q0[  1] = Fr2
    Q0[  2] = Fr3
    Q0[  3] = (Fr3*(v1*(_x29+_x33)+v1*v2*_x38+_x5*_x37+_x36+_x22+_x21+24*r2)+Fr2*(v1*(_x20+_x35+_x34)+v2*(_x18+_x17+_x25)+_x15+_x14+8*r3*_x5+_x33)+Fr1*(v1*(_x13+_x32)+_x10+_x31+_x8+(_x6-8*r3*v1+_x30)*v2+_x3))/24-_x1*(Fr3*(v1*(_x29+_x28)+v1*v2*_x27+_x5*_x24+15*r1*v3+_x22+_x21+60*r2)+Fr2*(v1*(_x20+_x19-45*r2)+v2*(_x18+_x17+_x16)+_x15+_x14+12*r3*_x5-60*r3)+Fr1*(v1*(_x13+_x11-15*r1)+_x10+_x9+_x8+(_x6-12*r3*v1+_x4)*v2+_x3))/360
    Q0[  4] = (Fr3*(v2*(_x29+v1*_x37+_x47+_x33)+_x7*_x38+_x11+_x46+_x48)+Fr2*(v2*(_x20+_x35+_x45+8*r3*v1)+_x44+_x31+_x43+12*r1*v1)+Fr1*(v2*(_x13+_x18+_x32+_x42+_x48)+_x41+(_x39-8*r3)*_x7+12*r2*v1+24*r3))/24-_x1*(Fr3*(v2*(_x29+v1*_x24+_x47+_x28)+_x7*_x27+15*r2*v3+_x46-60*r1)+Fr2*(v2*(_x20+_x19+_x45+12*r3*v1-15*r2)+_x44+_x9+_x43+15*r1*v1)+Fr1*(v2*(_x13+_x18+_x11+_x42-45*r1)+_x41+(_x39-12*r3)*_x7+15*r2*v1+60*r3))/360
    Q0[  5] = (Fr3*(_x64+v2*(_x63+8*r1*v3+_x30)+v1*(_x62-8*r2*v3+_x25)+_x61+_x60)+Fr2*(_x59+v2*(_x53+_x58+_x52)-8*r1*_x12+_x57+_x56+8*r3*v1*v3-24*r2*v3+24*r1)+Fr1*(_x55+v1*(_x53+_x52)+8*r2*_x12+v2*(_x51-8*r3*v3)+_x50+_x49-24*r1*v3+_x34))/24-_x1*(Fr3*(_x64+v2*(_x63+_x36+_x4)+v1*(_x62-12*r2*v3+_x16)+_x61+_x60-15*r3*v3)+Fr2*(_x59+v2*(_x53+_x58+_x65)-12*r1*_x12+_x57+_x56+12*r3*v1*v3-45*r2*v3+60*r1)+Fr1*(_x55+v1*(_x53+_x65)+12*r2*_x12+v2*(_x51-12*r3*v3)+_x50+_x49-45*r1*v3-60*r2))/360
    return Q0

def GeneralizedForce(v, Fr, r):
    assert len(v)  == 3
    assert len(Fr) == 3
    assert len(r)  == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA(): return __GeneralizedForce0(v, th, Fr, r)
    else         : return __GeneralizedForce(v, th, Fr, r)

def __QuatdFromV(v, th, vd):
    v1, v2, v3 = v
    vd1, vd2, vd3 = vd
    dqdt = array(zeros(4))
    ################################################
    #      Variable: dqdt
    ################################################
    _x1 = pow(th,-1)
    _x2 = th/2
    _x3 = sin(_x2)
    _x4 = v3*vd3+v2*vd2+v1*vd1
    _x5 = pow(th,-2)
    _x6 = cos(_x2)*_x4/2-_x1*_x3*_x4
    dqdt[  0] = -_x1*_x3*_x4/2
    dqdt[  1] = _x5*v1*_x6+_x1*_x3*vd1
    dqdt[  2] = _x5*v2*_x6+_x1*_x3*vd2
    dqdt[  3] = _x5*v3*_x6+_x1*_x3*vd3
    return dqdt

def __QuatdFromV0(v, th, vd):
    v1, v2, v3 = v
    vd1, vd2, vd3 = vd
    dqdt0 = array(zeros(4))
    ################################################
    #      Variable: dqdt0
    ################################################
    _x1 = v2*vd2
    _x2 = v3*vd3
    _x3 = _x2+_x1+v1*vd1
    _x4 = pow(th,2)
    _x5 = pow(v1,2)*vd1
    _x6 = v1*(_x2+_x1)
    _x7 = v1*v2*vd1
    _x8 = pow(v2,2)*vd2
    _x9 = v2*v3*vd3
    _x10 = v1*v3*vd1
    _x11 = v2*v3*vd2
    _x12 = pow(v3,2)*vd3
    dqdt0[  0] = _x4*_x3/96-_x3/4
    dqdt0[  1] = _x4*(_x6+_x5-20*vd1)/960-(_x6+_x5-12*vd1)/24
    dqdt0[  2] = _x4*(_x9+_x8-20*vd2+_x7)/960-(_x9+_x8-12*vd2+_x7)/24
    dqdt0[  3] = _x4*(_x12-20*vd3+_x11+_x10)/960-(_x12-12*vd3+_x11+_x10)/24
    return dqdt0

def QuatdFromV(v, vd):
    assert len(v)  == 3
    assert len(vd) == 3
    th = linalg.norm(v)
    if th < PymConstants.THETA(): return __QuatdFromV0(v, th, vd)
    else         : return __QuatdFromV(v, th, vd)
