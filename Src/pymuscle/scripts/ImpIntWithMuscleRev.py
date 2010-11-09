#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A generalized approach of simulating
rigid bodies and muscle fibers
with an implicit integration technique
"""
from OneRbImp import *
from FiberEffectImp import *
from scipy import sparse
from numpy import *
ORIGIN = 0
INSERTION = 1

def BuildRigidBodyEquations(rbIdx, # Origin body's index
                            nBody, # Total number of rigid bodies
                            nMuscle, # Total number of muscle fibers
                            body):
	
	yd_Ri, dyd_RidY = OneRbImp(body)

	matSize = nBody*14 + nMuscle
	
	Yd_Ri = zeros(matSize)
	Yd_Ri[rbIdx*14:(rbIdx+1)*14] = yd_Ri
	
	dYd_RidY = sparse.lil_matrix((matSize, matSize))
	dYd_RidY[rbIdx*14:(rbIdx+1)*14, rbIdx*14:(rbIdx+1)*14] = dyd_RidY
	
	return Yd_Ri, dYd_RidY

def BuildMuscleEquations(orgIdx, # Origin body's index
                         insIdx, # Insertion body's index
                         mIdx, # Muscle fiber's index
                         nBody, # Total number of rigid bodies
                         nMuscle, # Total number of muscle fibers
                         body_org, body_ins,
                         muscle):

	Td, dTddy_orgins, dTddT = FiberEffectImp_1(body_org, body_ins, muscle)

	yd_Q_org, dyd_Q_orgdy_orgins, dyd_Q_orgdT = FiberEffectImp_2(
	    ORIGIN, body_org, body_ins, muscle)

	yd_Q_ins, dyd_Q_insdy_orgins, dyd_Q_insdT = FiberEffectImp_2(
		INSERTION, body_org, body_ins, muscle)
	
	matSize = nBody*14 + nMuscle
	
	Yd_Qi = zeros(matSize)
	Yd_Qi[orgIdx*14:(orgIdx+1)*14] = yd_Q_org
	Yd_Qi[insIdx*14:(insIdx+1)*14] = yd_Q_ins
	Yd_Qi[nBody*14 + mIdx] = Td

	dYd_QidY = sparse.lil_matrix((matSize, matSize))

	# We have 9 blocks in dYd_Qi_dY

	# (1) Starting from the easiest...
	#
	#   .
	# d T
	# ---
	# d T
	#
	dYd_QidY[nBody*14 + mIdx, nBody*14 + mIdx] = dTddT

	# (2)(3)
	#
	#   .
	# d T
	# ------
	# d y
	#    {org,ins}
	#
	dYd_QidY[nBody*14 + mIdx, orgIdx*14:(orgIdx+1)*14] = dTddy_orgins[ORIGIN,:]
	dYd_QidY[nBody*14 + mIdx, insIdx*14:(insIdx+1)*14] = dTddy_orgins[INSERTION,:]

	# (4)(5)
	#
	#   . {org,ins}
	# d y
	#     Q
	# ------
	# d T
	#
	#
	# Seem to be this kind of assignment has a problem:
	#
	# dYd_QidY[orgIdx*14:(orgIdx+1)*14, nBody*14 + mIdx ] = dyd_Q_orgdT
	#
	for j in range(14):
		dYd_QidY[orgIdx*14 + j, nBody*14 + mIdx ] = dyd_Q_orgdT[j]
	for j in range(14):
		dYd_QidY[insIdx*14 + j, nBody*14 + mIdx ] = dyd_Q_insdT[j]
	
	# (6)(7)(8)(9)
	#
	#    . {org/ins}
	#  d y
	#      Q
	# ---------
	#  d y
	#     org
	# 
	# 
	#    . {org/ins}
	#  d y
	#      Q
	# ---------
	#  d y
	#     ins
	#
	dYd_QidY[orgIdx*14:(orgIdx+1)*14, orgIdx*14:(orgIdx+1)*14] = dyd_Q_orgdy_orgins[0:14,:]
	dYd_QidY[orgIdx*14:(orgIdx+1)*14, insIdx*14:(insIdx+1)*14] = dyd_Q_orgdy_orgins[14:28,:]
	dYd_QidY[insIdx*14:(insIdx+1)*14, insIdx*14:(insIdx+1)*14] = dyd_Q_insdy_orgins[14:28,:]
	dYd_QidY[insIdx*14:(insIdx+1)*14, orgIdx*14:(orgIdx+1)*14] = dyd_Q_insdy_orgins[0:14,:]
	
	return Yd_Qi, dYd_QidY


if __name__ == '__main__':
	m1 = sparse.lil_matrix((3, 2))
	m1[0,0] = 1985
	m1[0,1] = 100
	m1[2,1] = 8888
	
	m2 = sparse.lil_matrix((1, 3))
	m2[0,1] = 1111111
	
	M = sparse.lil_matrix((10, 10))
	
	M[1:4,1:3] = m1
	M[9,7:10] = m2
	
	print M
		
		
	p = [1,2,3];
	q = [1,0,0,0];
	pd = [0,0,1];
	qd = [0,0,0,1];
	m = 10;
	Idiag = 10,20,30
	Fr = 10,10,5
	Tr = -10,-5,7
	
	KSE = 1000.
	KPE = 900.
	b = 0.1
	xrest = 0.2
	T = 10.
	A = 0.
	
	Yd_Qi, dYd_QidY = BuildMuscleEquations(
	    0, # Origin body's index
	    1, # Insertion body's index
	    0, # Muscle fiber's index
	    5, # Total number of rigid bodies
	    20, # Total number of muscle fibers
	    p, q, pd, qd, m, Idiag, [0,0,0.5],
	    [1,2,3.1], q, pd, qd, m, Idiag, [1,1,2],
	    KSE, KPE, b, xrest, T, A)
	
	print Yd_Qi
	print dYd_QidY
	
	Yd_Ri, dYd_RidY = BuildRigidBodyEquations(7, # Origin body's index
	                                          25, # Total number of rigid bodies
	                                          20, # Total number of muscle fibers
	                                          p, q, pd, qd, m, Idiag, Fr, Tr)
	
	print Yd_Ri
	print dYd_RidY
	