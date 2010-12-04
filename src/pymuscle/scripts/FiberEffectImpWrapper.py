#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A generalized approach of simulating
rigid bodies and muscle fibers
with an implicit integration technique
"""
import ctypes as ct
import random
from numpy import array, int32
from scipy import sparse

libFiber = ct.CDLL('/home/johnu/pymuscle/libFiberEffectImp.so.1.0.1')
C_FiberEffectImp = libFiber.FiberEffectImp

#
# Two-dimensional array argument type definition
# Rule:
# (C_TYPE * ELEMENT LENGTH) * ELEMENT COUNT
#
DBL_2x14  = (ct.c_double * 14) * 2
UINT_784x2 = (ct.c_uint * 2) * 784
DBL_784 = ct.c_double * 784
DBL_48 = ct.c_double * 48

# Input parameters
C_input = DBL_48()
C_bClearVariable = ct.c_int(1)

# Output parameters
C_dTddy_orgins = DBL_2x14()
C_yd_Q_orgins = DBL_2x14()
C_dyd_Q_orginsdT = DBL_2x14()
C_Td = ct.c_double()
C_dTddT = ct.c_double()
# Sparse matrix structure of 'dyd_Q_orginsdy_orgins'
C_dyd_Q_orginsdy_orgins_keys = UINT_784x2()
C_dyd_Q_orginsdy_orgins_values = DBL_784()


def FiberEffectImpWrapper(body_org, body_ins, muscle):
    inp = body_org.flatten() + body_ins.flatten() + muscle.flatten()
    assert len(inp) == 18 + 18 + 12
    for i in range(48):
        C_input[i] = ct.c_double(inp[i])
    
    nnz = C_FiberEffectImp(C_input,
                           C_bClearVariable,
                           C_dTddy_orgins,
                           C_yd_Q_orgins,
                           C_dyd_Q_orginsdT,
                           ct.byref(C_Td),
                           ct.byref(C_dTddT),
                           C_dyd_Q_orginsdy_orgins_keys,
                           C_dyd_Q_orginsdy_orgins_values)
    
    ij = array(C_dyd_Q_orginsdy_orgins_keys[0:nnz], dtype=int32).T
    data = list(C_dyd_Q_orginsdy_orgins_values[0:nnz])
    
    dyd_Q_orginsdy_orgins = sparse.coo_matrix((data, ij), shape=(56, 14)).todok()
    
    return C_Td.value, array(C_dTddy_orgins), C_dTddT.value, array(C_yd_Q_orgins), dyd_Q_orginsdy_orgins, array(C_dyd_Q_orginsdT)






if __name__ == '__main__':
    """
    C_dTddy_orgins[0][0] = 1000.0
    C_dTddy_orgins[0][13] = 1000.13
    C_dTddy_orgins[1][0] = 1001.0
    C_dTddy_orgins[1][13] = 1001.13
    """
    
    for j in range(2000):
        
        for i in range(48):
            C_input[i] = random.random()
        """
        print 'C_input[0] =', C_input[0]
        print 'C_dTddy_orgins[0][0] =', C_dTddy_orgins[0][0]
        print 'C_dTddy_orgins[0][13] =', C_dTddy_orgins[0][13]
        print 'C_dTddy_orgins[1][0] =', C_dTddy_orgins[1][0]
        print 'C_dTddy_orgins[1][13] =', C_dTddy_orgins[1][13]
        C_input[0] = 1234.5678
        C_input[47] = 4747.4747
        """
        
        
        nnz = C_FiberEffectImp(C_input,
                               C_dTddy_orgins,
                               C_yd_Q_orgins,
                               C_dyd_Q_orginsdT,
                               C_dyd_Q_orginsdy_orgins_keys,
                               C_dyd_Q_orginsdy_orgins_values,
                               ct.byref(C_Td),
                               ct.byref(C_dTddT),
                               C_bClearVariable)
        """
        print 'C_input[0] =', C_input[0]
        print 'C_dTddy_orgins[0][0] =', C_dTddy_orgins[0][0]
        
        print 'Hello world'
        print 'Td =', C_Td
        print 'dTddT =', C_dTddT
        print 'nnz =', nnz
        """