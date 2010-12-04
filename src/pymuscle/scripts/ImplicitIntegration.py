#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Implicit integration
"""

from SymbolicImplicit import Symbolicf, SymbolicdfdY
from quat import quat_mult
from numpy import array, hstack, diag, dot, linalg, identity, zeros
import matplotlib.pyplot as plt
from MathUtil import EulerAnglesFromQuaternion

p0 = array([0, 0, 0.1])
q0 = array([1., 0, 0, 0])
v0 = array([0, 0, 0.5])
w0 = array([0.0, 0, 2.0])
qd0 = 0.5*array(quat_mult(q0, hstack([0, w0])))
h = 0.1
m = 1.
F = array([0,0,-9.81*m])
T = array([0,0,0])
sx, sy, sz = 0.1, 0.2, 0.3
Icm0 = array([m*(sy*sy + sz*sz)/12.,
			 m*(sx*sx + sz*sz)/12.,
			 m*(sx*sx + sy*sy)/12.])

simLen = 100

# Linear position history for plotting
pHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]
# Euler angles orientation history for plotting (converted from quaternion)
eHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]

for i in range(simLen):
	print 'Simulating', i, 'step...'
	f = Symbolicf(q0, v0, qd0, F, T, Icm0, m)
	dfdY = SymbolicdfdY(q0, qd0, T, Icm0)
	
	deltaY = dot(linalg.inv(1./h*identity(14) - dfdY), f)
	dpx, dpy, dpz, dqw, dqx, dqy, dqz, dvx, dvy, dvz, dqdw, dqdx, dqdy, dqdz = deltaY
	
	p0 += (dpx, dpy, dpz)
	q0 += (dqw, dqx, dqy, dqz)
	v0 += (dvx, dvy, dvz)
	qd0 += (dqdw, dqdx, dqdy, dqdz)
	
	q0 = q0/linalg.norm(q0)
	eAng = EulerAnglesFromQuaternion(q0)
	for axis in range(3):
		pHistory[axis][i] = p0[axis]
		eHistory[axis][i] = eAng[axis]
		
plt.figure(1)
plt.subplot(311)
plt.ylabel('x')
plt.plot(pHistory[0], 'r^', pHistory[0], 'r')
plt.subplot(312)
plt.ylabel('y')
plt.plot(pHistory[1], 'g^', pHistory[1], 'g')
plt.subplot(313)
plt.ylabel('z')
plt.plot(pHistory[2], 'b^', pHistory[2], 'b')

plt.figure(2)
plt.subplot(311)
plt.ylabel('phi')
plt.plot(eHistory[0], 'r^', eHistory[0], 'r')
plt.subplot(312)
plt.ylabel('theta')
plt.plot(eHistory[1], 'g^', eHistory[1], 'g')
plt.subplot(313)
plt.ylabel('psi')
plt.plot(eHistory[2], 'b^', eHistory[2], 'b')

plt.show()
