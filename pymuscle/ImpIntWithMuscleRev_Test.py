#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A generalized approach of simulating
rigid bodies and muscle fibers
with an implicit integration technique
"""

from quat import quat_mult
from numpy import array, hstack, diag, dot, linalg, identity, zeros
import pylab
from MathUtil import EulerAnglesFromQuaternion
from scipy import sparse
from scipy.sparse import linalg as spla
from math import sin, pi
import cPickle
from ImpIntWithMuscleRev import *


class ImBody:
	def __init__(self):
		# Initial state setting
		self.w = array([0., 0., 0.])
		self.p = array([0, 0, 0.])
		self.q = array([1., 0, 0, 0])
		self.pd = array([0, 0, 0])
		self.qd = 0.5*array(quat_mult(self.q, hstack([0, self.w])))

		# Rigid body inertia tensor properties
		self.setMassAndSize(10., 0.1, 0.2, 0.3)
		
		# External resultant forces and torques
		self.Fr = array([0,0,-9.81*self.m])
		self.Tr = array([0,0,0])
	def renormalizeQuat(self):
		self.q = self.q / linalg.norm(self.q)
		
	def setMassAndSize(self, m, sx, sy, sz):
		self.m = m
		self.sx, self.sy, self.sz = sx, sy, sz
		self.Idiag = [ self.m*(self.sy*self.sy + self.sz*self.sz)/12.,
		               self.m*(self.sx*self.sx + self.sz*self.sz)/12.,
		               self.m*(self.sx*self.sx + self.sy*self.sy)/12. ];

class ImMuscle:
	def __init__(self, orgIdx, insIdx, fibb_org, fibb_ins):
		self.KSE = 50000.
		self.KPE = 45000.
		self.b = 5000.1
		self.xrest = 0.05
		self.T = 10.
		self.A = 0.
		
		self.orgIdx = orgIdx
		self.insIdx = insIdx
		self.fibb_org = fibb_org
		self.fibb_ins = fibb_ins


def GoTest():
	# Simulation time step in seconds
	h = 0.001
	# Simulation time step count
	simLen = 100
			
	nBody = 2
	body = [ ImBody(), ImBody() ]
	
	body[0].p[2] = 0
	body[0].setMassAndSize(1.,0.1, 0.2, 0.3)
	body[1].p[2] = 1
	body[1].setMassAndSize(100.,0.1, 0.2, 0.3)
	
	nMuscle = 1
	muscle = [ ImMuscle(0, 1, [.1,0,0], [0,0,0]) ]
	
	
	# Linear position history for plotting
	pHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]
	eHistory = [zeros(simLen), zeros(simLen), zeros(simLen)]
	
	matSize = nBody*14 + nMuscle
	# A useful constant diagonal matrix used in integration
	Ioverh = sparse.lil_matrix((matSize, matSize))
	Ioverh.setdiag([1./h,]*matSize)
	
	for i in range(simLen):
		if i > 40 and i < 60:
			body[1].Fr[2] += 200.
		else:
			body[1].Fr[2] = -9.81*body[1].m
		
		print 'Simulating', i, 'step... (', float(i)/simLen*100,'%)'
	
		# Renormalize the quaternion
		for j in range(nBody):
			body[j].renormalizeQuat()
		
		# Record history for plotting
		for axis in range(3):
			pHistory[axis][i] = body[0].p[axis]
			eHistory[axis][i] = body[1].p[axis]
		
		dfdY = sparse.lil_matrix((matSize, matSize))
		f = zeros(matSize)
		for j in range(nBody):
			Yd_Rj, dYd_RjdY = BuildRigidBodyEquations(j,       # Body's index
				                                      nBody,   # Total number of rigid bodies
				                                      nMuscle, # Total number of muscle fibers
				                                      body[j])
			f += Yd_Rj
			dfdY += dYd_RjdY
		for j in range(nMuscle):
			mj = muscle[j]
			body_org = body[mj.orgIdx]
			body_ins = body[mj.insIdx]
			
			Yd_Qj, dYd_QjdY = BuildMuscleEquations(mj.orgIdx,  # Origin body's index
				                                   mj.insIdx,  # Insertion body's index
				                                   j,          # Muscle fiber's index
				                                   nBody,      # Total number of rigid bodies
				                                   nMuscle,    # Total number of muscle fibers
				                                   body_org, body_ins,
			                                       mj)
			
			f += Yd_Qj
			dfdY += dYd_QjdY
				
		deltaY = spla.spsolve(Ioverh - dfdY, f)
		for j in range(nBody):
			dpx, dpy, dpz, dqw, dqx, dqy, dqz, dpdx, dpdy, dpdz, dqdw, dqdx, dqdy, dqdz = deltaY[14*j:14*(j+1)]
			
			#print body[0].p
			
			body[j].p += (dpx, dpy, dpz)
			body[j].q += (dqw, dqx, dqy, dqz)
			body[j].pd += (dpdx, dpdy, dpdz)
			body[j].qd += (dqdw, dqdx, dqdy, dqdz)
		for j in range(nMuscle):
			dT = deltaY[14*nBody + j]
			muscle[j].T += dT
	
	return pHistory, eHistory
	
if __name__ == '__main__':
	pHistory, eHistory = GoTest()
	
	pylab.figure(1)
	pylab.subplot(311)
	pylab.ylabel('x (m)')
	pylab.plot(pHistory[0], 'r', eHistory[0], 'b')
	pylab.subplot(312)
	pylab.ylabel('y (m)')
	pylab.plot(pHistory[1], 'r', eHistory[1], 'b')
	pylab.subplot(313)
	pylab.ylabel('z (m)')
	pylab.plot(pHistory[2], 'r', eHistory[2], 'b')
	
	pylab.show()
	

