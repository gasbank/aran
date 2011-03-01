# Autogenerated file
# 2010 Geoyeob Kim
# As a part of the thesis implementation
from numpy import *
from math import pow, sin, cos
from scipy import sparse
def OneRbImp(p, q, pd, qd, m, Idiag, Fr, Tr):
    px, py, pz = p
    qw, qx, qy, qz = q
    pdx, pdy, pdz = pd
    qdw, qdx, qdy, qdz = qd
    Ixx, Iyy, Izz = Idiag
    Frx, Fry, Frz = Fr
    Trx, Try, Trz = Tr

    yd_R = zeros((14))
    dyd_RdY = sparse.lil_matrix((14, 14))
    ################################################
    #      Variable: yd_R
    ################################################
    _x1 = pow(m,-1)
    _x2 = -qdw*qz+qdx*qy-qdy*qx+qdz*qw
    _x3 = -qdx*qz-qdw*qy+qdz*qx+qdy*qw
    _x4 = qdy*qz-qdz*qy-qdw*qx+qdx*qw
    _x5 = qdz*qz+qdy*qy+qdx*qx+qdw*qw
    _x6 = pow(Ixx,-1)
    _x7 = Trx-4*(Izz*_x2*_x3-Iyy*_x2*_x3)
    _x8 = pow(Iyy,-1)
    _x9 = Try-4*(Ixx*_x2*_x4-Izz*_x2*_x4)
    _x10 = pow(Izz,-1)
    _x11 = Trz-4*(Iyy*_x3*_x4-Ixx*_x3*_x4)
    yd_R[  0] = pdx
    yd_R[  1] = pdy
    yd_R[  2] = pdz
    yd_R[  3] = qdw
    yd_R[  4] = qdx
    yd_R[  5] = qdy
    yd_R[  6] = qdz
    yd_R[  7] = Frx*_x1
    yd_R[  8] = Fry*_x1
    yd_R[  9] = Frz*_x1
    yd_R[ 10] = 0.5*(-_x10*qz*_x11-_x8*qy*_x9-_x6*qx*_x7)+qdw*_x5-qdx*_x4-qdy*_x3-qdz*_x2
    yd_R[ 11] = 0.5*(_x10*qy*_x11-_x8*qz*_x9+_x6*qw*_x7)+qdx*_x5+qdw*_x4-qdz*_x3+qdy*_x2
    yd_R[ 12] = 0.5*(-_x10*qx*_x11+_x8*qw*_x9+_x6*qz*_x7)+qdy*_x5+qdz*_x4+qdw*_x3-qdx*_x2
    yd_R[ 13] = 0.5*(_x10*qw*_x11+_x8*qx*_x9-_x6*qy*_x7)+qdz*_x5-qdy*_x4+qdx*_x3+qdw*_x2
    ################################################
    #      Variable: dyd_RdY
    ################################################
    _x1 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    _x2 = pow(qdw,2)
    _x3 = pow(qdx,2)
    _x4 = -_x3
    _x5 = pow(qdy,2)
    _x6 = -_x5
    _x7 = pow(qdz,2)
    _x8 = -_x7
    _x9 = pow(Ixx,-1)
    _x10 = -qdw*qz+qdx*qy-qdy*qx+qdz*qw
    _x11 = -qdx*qz-qdw*qy+qdz*qx+qdy*qw
    _x12 = -Iyy*qdz*_x11
    _x13 = Izz*qdz*_x11+_x12+Izz*qdy*_x10-Iyy*qdy*_x10
    _x14 = pow(Izz,-1)
    _x15 = qdy*qz-qdz*qy-qdw*qx+qdx*qw
    _x16 = -Ixx*qdy*_x15
    _x17 = Iyy*qdy*_x15+_x16+Iyy*qdx*_x11-Ixx*qdx*_x11
    _x18 = pow(Iyy,-1)
    _x19 = -Izz*qdx*_x10
    _x20 = -Izz*qdz*_x15+Ixx*qdz*_x15+_x19+Ixx*qdx*_x10
    _x21 = 2*qdw*qdx
    _x22 = Izz*qdz*_x10
    _x23 = Iyy*qdy*_x11
    _x24 = -Izz*qdy*_x11+_x23+_x22-Iyy*qdz*_x10
    _x25 = Izz*qdy*_x15+_x16+Izz*qdw*_x10-Ixx*qdw*_x10
    _x26 = Iyy*qdz*_x15-Ixx*qdz*_x15-Iyy*qdw*_x11+Ixx*qdw*_x11
    _x27 = Trx-4*(Izz*_x10*_x11-Iyy*_x10*_x11)
    _x28 = -_x9*_x27
    _x29 = 2*qdw*qdy
    _x30 = Izz*qdx*_x11-Iyy*qdx*_x11-Izz*qdw*_x10+Iyy*qdw*_x10
    _x31 = -Iyy*qdw*_x15+Ixx*qdw*_x15+_x12+Ixx*qdz*_x11
    _x32 = Ixx*qdx*_x15
    _x33 = -Izz*qdx*_x15+_x32+_x22-Ixx*qdz*_x10
    _x34 = Try-4*(Ixx*_x10*_x15-Izz*_x10*_x15)
    _x35 = -_x18*_x34
    _x36 = 2*qdw*qdz
    _x37 = -Izz*qdw*_x11+Iyy*qdw*_x11+_x19+Iyy*qdx*_x10
    _x38 = Izz*qdw*_x15-Ixx*qdw*_x15-Izz*qdy*_x10+Ixx*qdy*_x10
    _x39 = -Iyy*qdx*_x15+_x32+_x23-Ixx*qdy*_x11
    _x40 = Trz-4*(Iyy*_x11*_x15-Ixx*_x11*_x15)
    _x41 = -_x14*_x40
    _x42 = 2*qdw*qw
    _x43 = 2*qdx*qx
    _x44 = 2*qdy*qy
    _x45 = 2*qdz*qz
    _x46 = Iyy*qz*_x11
    _x47 = -Izz*qz*_x11+_x46-Izz*qy*_x10+Iyy*qy*_x10
    _x48 = Ixx*qy*_x15
    _x49 = -Iyy*qy*_x15+_x48-Iyy*qx*_x11+Ixx*qx*_x11
    _x50 = Izz*qx*_x10
    _x51 = Izz*qz*_x15-Ixx*qz*_x15+_x50-Ixx*qx*_x10
    _x52 = -Izz*qz*_x10
    _x53 = -Iyy*qy*_x11
    _x54 = Izz*qy*_x11+_x53+_x52+Iyy*qz*_x10
    _x55 = -Izz*qy*_x15+_x48-Izz*qw*_x10+Ixx*qw*_x10
    _x56 = -Iyy*qz*_x15+Ixx*qz*_x15+Iyy*qw*_x11-Ixx*qw*_x11
    _x57 = -Izz*qx*_x11+Iyy*qx*_x11+Izz*qw*_x10-Iyy*qw*_x10
    _x58 = Iyy*qw*_x15-Ixx*qw*_x15+_x46-Ixx*qz*_x11
    _x59 = -Ixx*qx*_x15
    _x60 = Izz*qx*_x15+_x59+_x52+Ixx*qz*_x10
    _x61 = Izz*qw*_x11-Iyy*qw*_x11+_x50-Iyy*qx*_x10
    _x62 = -Izz*qw*_x15+Ixx*qw*_x15+Izz*qy*_x10-Ixx*qy*_x10
    _x63 = Iyy*qx*_x15+_x59+_x53+Ixx*qy*_x11
    _x64 = _x9*_x27
    _x65 = -_x2
    _x66 = 2*qdx*qdy
    _x67 = _x14*_x40
    _x68 = 2*qdx*qdz
    _x69 = _x18*_x34
    _x70 = 2*qdy*qdz
    dyd_RdY[  0,  7] = 1
    dyd_RdY[  1,  8] = 1
    dyd_RdY[  2,  9] = 1
    dyd_RdY[  3, 10] = 1
    dyd_RdY[  4, 11] = 1
    dyd_RdY[  5, 12] = 1
    dyd_RdY[  6, 13] = 1
    dyd_RdY[  7,  0] = _x1[0]
    dyd_RdY[  7,  1] = _x1[1]
    dyd_RdY[  7,  2] = _x1[2]
    dyd_RdY[  7,  3] = _x1[3]
    dyd_RdY[  7,  4] = _x1[4]
    dyd_RdY[  7,  5] = _x1[5]
    dyd_RdY[  7,  6] = _x1[6]
    dyd_RdY[  7,  7] = _x1[7]
    dyd_RdY[  7,  8] = _x1[8]
    dyd_RdY[  7,  9] = _x1[9]
    dyd_RdY[  7, 10] = _x1[10]
    dyd_RdY[  7, 11] = _x1[11]
    dyd_RdY[  7, 12] = _x1[12]
    dyd_RdY[  7, 13] = _x1[13]
    dyd_RdY[  8,  0] = _x1[0]
    dyd_RdY[  8,  1] = _x1[1]
    dyd_RdY[  8,  2] = _x1[2]
    dyd_RdY[  8,  3] = _x1[3]
    dyd_RdY[  8,  4] = _x1[4]
    dyd_RdY[  8,  5] = _x1[5]
    dyd_RdY[  8,  6] = _x1[6]
    dyd_RdY[  8,  7] = _x1[7]
    dyd_RdY[  8,  8] = _x1[8]
    dyd_RdY[  8,  9] = _x1[9]
    dyd_RdY[  8, 10] = _x1[10]
    dyd_RdY[  8, 11] = _x1[11]
    dyd_RdY[  8, 12] = _x1[12]
    dyd_RdY[  8, 13] = _x1[13]
    dyd_RdY[  9,  0] = _x1[0]
    dyd_RdY[  9,  1] = _x1[1]
    dyd_RdY[  9,  2] = _x1[2]
    dyd_RdY[  9,  3] = _x1[3]
    dyd_RdY[  9,  4] = _x1[4]
    dyd_RdY[  9,  5] = _x1[5]
    dyd_RdY[  9,  6] = _x1[6]
    dyd_RdY[  9,  7] = _x1[7]
    dyd_RdY[  9,  8] = _x1[8]
    dyd_RdY[  9,  9] = _x1[9]
    dyd_RdY[  9, 10] = _x1[10]
    dyd_RdY[  9, 11] = _x1[11]
    dyd_RdY[  9, 12] = _x1[12]
    dyd_RdY[  9, 13] = _x1[13]
    dyd_RdY[ 10,  3] = 0.5*(4*_x18*qy*_x20+4*_x14*qz*_x17+4*_x9*qx*_x13)+_x8+_x6+_x4+_x2
    dyd_RdY[ 10,  4] = 0.5*(_x28+4*_x14*qz*_x26+4*_x18*qy*_x25+4*_x9*qx*_x24)+_x21
    dyd_RdY[ 10,  5] = 0.5*(_x35+4*_x18*qy*_x33+4*_x14*qz*_x31+4*_x9*qx*_x30)+_x29
    dyd_RdY[ 10,  6] = 0.5*(_x41+4*_x14*qz*_x39+4*_x18*qy*_x38+4*_x9*qx*_x37)+_x36
    dyd_RdY[ 10, 10] = 0.5*(4*_x18*qy*_x51+4*_x14*qz*_x49+4*_x9*qx*_x47)+_x45+_x44+_x43+_x42
    dyd_RdY[ 10, 11] = 0.5*(4*_x14*qz*_x56+4*_x18*qy*_x55+4*_x9*qx*_x54)+2*qdw*qx-2*qdx*qw
    dyd_RdY[ 10, 12] = 0.5*(4*_x18*qy*_x60+4*_x14*qz*_x58+4*_x9*qx*_x57)+2*qdw*qy-2*qdy*qw
    dyd_RdY[ 10, 13] = 0.5*(4*_x14*qz*_x63+4*_x18*qy*_x62+4*_x9*qx*_x61)+2*qdw*qz-2*qdz*qw
    dyd_RdY[ 11,  3] = 0.5*(_x64+4*_x18*qz*_x20-4*_x14*qy*_x17-4*_x9*qw*_x13)+_x21
    dyd_RdY[ 11,  4] = 0.5*(-4*_x14*qy*_x26+4*_x18*qz*_x25-4*_x9*qw*_x24)+_x8+_x6+_x3+_x65
    dyd_RdY[ 11,  5] = 0.5*(_x67+4*_x18*qz*_x33-4*_x14*qy*_x31-4*_x9*qw*_x30)+_x66
    dyd_RdY[ 11,  6] = 0.5*(_x35-4*_x14*qy*_x39+4*_x18*qz*_x38-4*_x9*qw*_x37)+_x68
    dyd_RdY[ 11, 10] = 0.5*(4*_x18*qz*_x51-4*_x14*qy*_x49-4*_x9*qw*_x47)-2*qdw*qx+2*qdx*qw
    dyd_RdY[ 11, 11] = 0.5*(-4*_x14*qy*_x56+4*_x18*qz*_x55-4*_x9*qw*_x54)+_x45+_x44+_x43+_x42
    dyd_RdY[ 11, 12] = 0.5*(4*_x18*qz*_x60-4*_x14*qy*_x58-4*_x9*qw*_x57)+2*qdx*qy-2*qdy*qx
    dyd_RdY[ 11, 13] = 0.5*(-4*_x14*qy*_x63+4*_x18*qz*_x62-4*_x9*qw*_x61)+2*qdx*qz-2*qdz*qx
    dyd_RdY[ 12,  3] = 0.5*(_x69-4*_x18*qw*_x20+4*_x14*qx*_x17-4*_x9*qz*_x13)+_x29
    dyd_RdY[ 12,  4] = 0.5*(_x41+4*_x14*qx*_x26-4*_x18*qw*_x25-4*_x9*qz*_x24)+_x66
    dyd_RdY[ 12,  5] = 0.5*(-4*_x18*qw*_x33+4*_x14*qx*_x31-4*_x9*qz*_x30)+_x8+_x5+_x4+_x65
    dyd_RdY[ 12,  6] = 0.5*(_x64+4*_x14*qx*_x39-4*_x18*qw*_x38-4*_x9*qz*_x37)+_x70
    dyd_RdY[ 12, 10] = 0.5*(-4*_x18*qw*_x51+4*_x14*qx*_x49-4*_x9*qz*_x47)-2*qdw*qy+2*qdy*qw
    dyd_RdY[ 12, 11] = 0.5*(4*_x14*qx*_x56-4*_x18*qw*_x55-4*_x9*qz*_x54)-2*qdx*qy+2*qdy*qx
    dyd_RdY[ 12, 12] = 0.5*(-4*_x18*qw*_x60+4*_x14*qx*_x58-4*_x9*qz*_x57)+_x45+_x44+_x43+_x42
    dyd_RdY[ 12, 13] = 0.5*(4*_x14*qx*_x63-4*_x18*qw*_x62-4*_x9*qz*_x61)+2*qdy*qz-2*qdz*qy
    dyd_RdY[ 13,  3] = 0.5*(_x67-4*_x18*qx*_x20-4*_x14*qw*_x17+4*_x9*qy*_x13)+_x36
    dyd_RdY[ 13,  4] = 0.5*(_x69-4*_x14*qw*_x26-4*_x18*qx*_x25+4*_x9*qy*_x24)+_x68
    dyd_RdY[ 13,  5] = 0.5*(_x28-4*_x18*qx*_x33-4*_x14*qw*_x31+4*_x9*qy*_x30)+_x70
    dyd_RdY[ 13,  6] = 0.5*(-4*_x14*qw*_x39-4*_x18*qx*_x38+4*_x9*qy*_x37)+_x7+_x6+_x4+_x65
    dyd_RdY[ 13, 10] = 0.5*(-4*_x18*qx*_x51-4*_x14*qw*_x49+4*_x9*qy*_x47)-2*qdw*qz+2*qdz*qw
    dyd_RdY[ 13, 11] = 0.5*(-4*_x14*qw*_x56-4*_x18*qx*_x55+4*_x9*qy*_x54)-2*qdx*qz+2*qdz*qx
    dyd_RdY[ 13, 12] = 0.5*(-4*_x18*qx*_x60-4*_x14*qw*_x58+4*_x9*qy*_x57)-2*qdy*qz+2*qdz*qy
    dyd_RdY[ 13, 13] = 0.5*(-4*_x14*qw*_x63-4*_x18*qx*_x62+4*_x9*qy*_x61)+_x45+_x44+_x43+_x42
    return yd_R, dyd_RdY