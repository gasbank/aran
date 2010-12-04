#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A spacetime particle example
(Spacetime constraints)
"""

from numpy import *
from math import *
from scipy import sparse
from scipy.sparse import linalg as LA
#from scipy.sparse.linalg import isolve as IS
import random
import lsqr
import cPickle

def dRdSVector(S):
	dRdS = zeros((6*nf))
	for i in range(3*nf,6*nf):
		f_c = S[i]
		dRdS[i] = 2*f_c
	return dRdS

def CVector(S):
	global nf, mass, h, fg
	C = zeros((3*(nf-2)+6 + 1))
	for i in range(3*(nf-2)):
		curFrame = i/3 + 1
		axis = i % 3
		x_n = S[3*(curFrame+1) + axis]
		x_c = S[3*(curFrame+0) + axis]
		x_p = S[3*(curFrame-1) + axis]
		f_c = S[3*(curFrame+0) + axis + 3*nf]
		if axis == 2: # z-axis
			C[i] = mass*(x_n - 2*x_c + x_p)/h**2 - f_c - mass*fg
		else:
			C[i] = mass*(x_n - 2*x_c + x_p)/h**2 - f_c
	C[3*(nf-2):3*(nf-2)+6] = array([ S[0]-x1[0],
	                                 S[1]-x1[1],
	                                 S[2]-x1[2],
	                                 S[3*(nf-1) + 0] - xn[0],
	                                 S[3*(nf-1) + 1] - xn[1],
	                                 S[3*(nf-1) + 2] - xn[2] ])
	C[3*(nf-2)+6] = S[5]-S[2]
	return C


nf = 10      # Number of animation frames
mass = 1     # mass of the particle
h = 0.1      # time-step
fg = -9.81    # gravitational acceleration
x1 = array([0,0,0])
xn = array([0.0,0.0,3.7])

assert(nf>=2)
S = ones((nf*6))  # Solution vector (initial guess is set to zero)


dCdS = sparse.lil_matrix((3*(nf-2)+6 + 1, 6*nf))
for i in range(3*(nf-2)):
	curConstraint = i/3 + 1
	curConstraintAxis = i%3
	for j in range(0,3*nf):
		curFrame = j/3
		curFrameAxis = j%3
		if curFrame == curConstraint and curFrameAxis == curConstraintAxis:
			dCdS[i,j] = -2*mass/h**2
		elif ((curFrame-1 == curConstraint) or (curFrame+1 == curConstraint)) and (curFrameAxis == curConstraintAxis):
			dCdS[i,j] = mass/h**2
	for j in range(3*nf, 6*nf):
		curFrame = j/3 - nf
		curFrameAxis = j%3
		if curFrame == curConstraint and curFrameAxis == curConstraintAxis:
			dCdS[i,j] = -1
dCdS[3*(nf-2):3*(nf-2)+3 , 0:3] = identity(3)
dCdS[3*(nf-2)+3:3*(nf-2)+6 , 3*(nf-1):3*nf] = identity(3)
dCdS[3*(nf-2)+6 , 2] = -1.
dCdS[3*(nf-2)+6 , 5] = +1.

assert(dCdS.shape == (3*(nf-2)+6 + 1, 6*nf))

d2RdSdS = sparse.lil_matrix((6*nf, 6*nf))
for i in range(3*nf, 6*nf):
	d2RdSdS[i, i] = 2.
assert(d2RdSdS.shape == (6*nf, 6*nf))

"""
try:
	Sfileread = open('S.dat','r')
	S0 = cPickle.load(Sfileread)
	if S0.shape == (nf*6,):
		print 'Continued from the last solution found.'
		S = S0
	Sfileread.close()
except:
	print 'Newly starting the solver.'
	pass
"""

iteration = 0
while True:
	#print S
	#print linalg.norm(CVector(S))
	
	drds = dRdSVector(S)
	drdsNorm = linalg.norm(drds)
	SS = lsqr.lsqr(d2RdSdS,-drds)
	S1p = SS[0] + S
	drds_2 = linalg.norm(dRdSVector(S1p))
	
	CS1p = CVector(S1p)
	CS1pNorm = linalg.norm(CS1p)
	SSS = lsqr.lsqr( dCdS, -CS1p)
	
	S = SSS[0] + S1p
	
	CNorm = linalg.norm(CVector(S))
	
	R = sum([v**2 for v in S[3*nf:6*nf]])
	print '|C| =', CNorm, ',', 'R =', R, ',', '|dRdS| =', drdsNorm
	#print iteration
	iteration = iteration + 1
	Sfile = open('S.dat','w')
	cPickle.dump(S, Sfile)
	Sfile.close()

	
	if CNorm < 1e-15 and drdsNorm < 1e-15:
		break

print 'finished'
