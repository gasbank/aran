#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A box with a muscle fiber
(simulated with implicit integration technique)
"""

from SymbolicImplicitWithMuscleSimplified import Symbolic
from quat import quat_mult
from numpy import array, hstack, diag, dot, linalg, identity, zeros
import matplotlib.pyplot as plt
from MathUtil import EulerAnglesFromQuaternion
from scipy import sparse
from scipy.sparse import linalg as spla
from math import sin, pi
#from scipy import sparse, linsolve

p0 = array([0, 0, 0.5])
q0 = array([1., 0, 0, 0])
v0 = array([0, 0, 0])
w0 = array([0.0, 0, 0])
qd0 = 0.5*array(quat_mult(q0, hstack([0, w0])))
h = 0.005
m = 10.
F = array([0,0,-9.81*m])
T = array([0,0,0])
sx, sy, sz = 0.1, 0.2, 0.3
Icm0 = array([m*(sy*sy + sz*sz)/12.,
			 m*(sx*sx + sz*sz)/12.,
			 m*(sx*sx + sy*sy)/12.])

#KPE, KSE, b, xrest = fibParam
#fibx, fiby, fibz = fibFixPos
fibParam = (185000., 185000., 18500.0001, 0)  # Muscle fiber parameters
fibFixPos = (0, 0, 1.) # Muscle fixed position (oppsite to a box)
#bodyFixPos = (0.1/2, 0.2/2, 0.3/2) # Muscle fixed position on the body in body coordinates
#bodyFixPos = (0,0,0) # Muscle fixed position on the body in body coordinates
bodyFixPos = (sx/2,sy/2,sz/2) # Muscle fixed position on the body in body coordinates
Ten0 = 0 # Initial muscle tension
A0 = 0 # Initial muscle actuating force


simLen = 100

# Linear position history for plotting
pHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]
# Euler angles orientation history for plotting (converted from quaternion)
eHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]
# Muscle fiber tension
tenHistory = zeros(simLen)

Ioverh = sparse.lil_matrix((15, 15))
Ioverh.setdiag([1./h,]*15)
	
for i in range(simLen):
	#A0 = 3*sin(float(i)/simLen*4*pi)
	if i%50 == 0:
		print 'Simulating', i, 'step... (', float(i)/simLen*100,'%)'

	# Record history for plotting
	q0 = q0/linalg.norm(q0)
	eAng = EulerAnglesFromQuaternion(q0)
	for axis in range(3):
		pHistory[axis][i] = p0[axis]
		eHistory[axis][i] = eAng[axis]
	tenHistory[i] = Ten0
	
	f, dfdY = Symbolic(p0, q0, v0, qd0, F, T, Icm0, m,
	                   fibParam, fibFixPos, bodyFixPos, Ten0, A0)
	deltaY = spla.spsolve(Ioverh - dfdY, f)
	dpx, dpy, dpz, dqw, dqx, dqy, dqz, dTen, dvx, dvy, dvz, dqdw, dqdx, dqdy, dqdz = deltaY
		
	p0 += (dpx, dpy, dpz)
	q0 += (dqw, dqx, dqy, dqz)
	v0 += (dvx, dvy, dvz)
	qd0 += (dqdw, dqdx, dqdy, dqdz)
	Ten0 += dTen
	
	
	
plt.figure(1)
plt.subplot(311)
plt.ylabel('x (m)')
plt.plot(pHistory[0], 'r^', pHistory[0], 'r')
plt.subplot(312)
plt.ylabel('y (m)')
plt.plot(pHistory[1], 'g^', pHistory[1], 'g')
plt.subplot(313)
plt.ylabel('z (m)')
plt.plot(pHistory[2], 'b^', pHistory[2], 'b')

plt.figure(2)
plt.subplot(311)
plt.ylabel('phi (rad)')
plt.plot(eHistory[0], 'r^', eHistory[0], 'r')
plt.subplot(312)
plt.ylabel('theta (rad)')
plt.plot(eHistory[1], 'g^', eHistory[1], 'g')
plt.subplot(313)
plt.ylabel('psi (rad)')
plt.plot(eHistory[2], 'b^', eHistory[2], 'b')

plt.figure(3)
plt.ylabel('tension (N)')
plt.plot(tenHistory, 'r^', tenHistory, 'r')

plt.show()
