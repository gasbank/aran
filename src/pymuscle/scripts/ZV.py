# ZV.py
# 2010 Geoyeob Kim
# As a part of the thesis implementation
#
# Estimate next step's position
# Linear function of chi_2
#
#   - chi_0 : previous step l-1  (known)
#   - chi_1 : current step l     (known)
#   - chi_2 : next step l+1      (unknown)
#
# Returns the matrix Z and vector V which calculates pW_2
# 
#  pW_2 = Z*p_1 + V
#
#  p_1   - local
#  pW_2  - global
#
# p     : (contact) point in body coordinates (4x1)
# n     : contact normal in world coordinates (4x1)
#

from dRdv_real import dRdv, RotationMatrixFromV
from numpy import array, linalg, zeros, dot, vstack, hstack, identity
from math import sin, cos
from scipy import sparse
from MathUtil import cot

def GetWFrom6Dof(chi_1):
    nd = 6
    assert len(chi_1) == nd
    W = zeros((4,4))    
    W[0:3,3  ] = chi_1[0:3]
    W[0:3,0:3] = RotationMatrixFromV(chi_1[3:6]) # 'V' in the function name has different meaning for V in ZV().
    W[3,3] = 1
    return W

def dRdv_tensor(chi_1):
    dRdv1, dRdv2, dRdv3 = dRdv(chi_1[3:6])
    dRdv1x = zeros((4,4)); dRdv1x[0:3,0:3] = dRdv1
    dRdv2x = zeros((4,4)); dRdv2x[0:3,0:3] = dRdv2
    dRdv3x = zeros((4,4)); dRdv3x[0:3,0:3] = dRdv3
    dRdv_tensor = [ dRdv1x, dRdv2x, dRdv3x ]
    return dRdv_tensor

def ZVQ(chi_1, p, n, W, dRdv_tensor):
    nd = 6
    assert len(chi_1) == nd
    assert len(p) == 4 # Homogeneous coordinates
    assert len(n) == 4
    Z = sparse.lil_matrix((4,nd))
    V = zeros((4,1))
    
    Z[0,0] = 1
    Z[1,1] = 1
    Z[2,2] = 1
    Z[:,3] = dot(dRdv_tensor[0], p).reshape(4,1)
    Z[:,4] = dot(dRdv_tensor[1], p).reshape(4,1)
    Z[:,5] = dot(dRdv_tensor[2], p).reshape(4,1)
    
    V = dot(W, p) - Z*chi_1
    Q = sparse.lil_matrix((6,5))
    Q[0:6,0:4] = Z.T
    for i in xrange(6):
        Q[i, 4] = Z[:,i].T * n
    
    '''
    print 'Z'
    print Z.todense()
    print 'V'
    print V
    print 'Q'
    print Q.todense()
    '''
    
    return Z, V, Q

if __name__ == '__main__':
    Z, V, Q = ZVQ(array([1.,2,3,0,1,2]), array([10.,20,30,1]), array([1.,2,3,0]))
    print Z
    print V
    print Q
