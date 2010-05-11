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
from FiberEffectImpWrapper import *

ORIGIN = 0
INSERTION = 1

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
	def flatten(self):
		return [ self.p[0], self.p[1], self.p[2],
		         self.q[0], self.q[1], self.q[2], self.q[3],
		         self.pd[0], self.pd[1], self.pd[2],
		         self.qd[0], self.qd[1], self.qd[2], self.qd[3],
		         self.m, self.Idiag[0], self.Idiag[1], self.Idiag[2] ]

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
	def flatten(self):
		return [ self.KSE, self.KPE, self.b, self.xrest, self.T, self.A,
		         self.fibb_org[0], self.fibb_org[1], self.fibb_org[2],
		         self.fibb_ins[0], self.fibb_ins[1], self.fibb_ins[2] ]
		        


def GoTest():
	# Simulation time step in seconds
	h = 0.001
	# Simulation time step count
	simLen = 2000
			
	nBody = 2
	body = [ ImBody(), ImBody() ]
	
	body[0].p[2] = 0
	body[0].setMassAndSize(1.,0.1, 0.2, 0.3)
	body[1].p[2] = 1
	body[1].setMassAndSize(3.,0.1, 0.2, 0.3)
	
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
		
		if (i % 10) == 0:
			print 'Simulating %d step (%6.2f %%)' % (i, float(i)/simLen*100)
	
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
			yd_Rj, dyd_RjdY = OneRbImp(body[j])
			f[j*14:(j+1)*14] += yd_Rj
			for (k,v) in dyd_RjdY.iteritems():
				dfdY[j*14 + k[0], j*14 + k[1]] = v

		for j in range(nMuscle):
			mj = muscle[j]
			body_org = body[mj.orgIdx]
			body_ins = body[mj.insIdx]
			
			fiberRet = FiberEffectImp(body_org, body_ins, mj)
			#fiberRet = FiberEffectImpWrapper(body_org, body_ins, mj)
			
			Td, dTddy_orgins, dTddT, yd_Q_orgins, dyd_Q_orginsdy_orgins, dyd_Q_orginsdT = fiberRet
			
			f[mj.orgIdx*14 + 7 : mj.orgIdx*14 + 14] += yd_Q_orgins[ORIGIN, 7:14]
			f[mj.insIdx*14 + 7 : mj.insIdx*14 + 14] += yd_Q_orgins[INSERTION, 7:14]
			f[nBody*14 + j] = Td
			
			# We have 9 blocks in dYd_Qi_dY

			# (1) Starting from the easiest...
			#
			#   .
			# d T
			# ---
			# d T
			#
			dfdY[nBody*14 + j, nBody*14 + j] = dTddT
		
			# (2)(3)
			#
			#   .
			# d T
			# ------
			# d y
			#    {org,ins}
			#
			for k in range(14):
				dfdY[nBody*14 + j, mj.orgIdx*14 + k] = dTddy_orgins[ORIGIN,k]
				dfdY[nBody*14 + j, mj.insIdx*14 + k] = dTddy_orgins[INSERTION,k]
			
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
			for k in range(14):
				dfdY[mj.orgIdx*14 + k, nBody*14 + j ] = dyd_Q_orginsdT[ORIGIN,k]
				dfdY[mj.insIdx*14 + k, nBody*14 + j ] = dyd_Q_orginsdT[INSERTION,k]
			
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
			
			
			for (k, v) in dyd_Q_orginsdy_orgins.iteritems():
				if k[0] < 14:
					r = mj.orgIdx*14 + k[0]
					c = mj.orgIdx*14 + k[1]
				elif k[0] < 28:
					r = mj.orgIdx*14 + k[0] - 14
					c = mj.insIdx*14 + k[1]
				elif k[0] < 42:
					r = mj.insIdx*14 + k[0] - 28
					c = mj.orgIdx*14 + k[1]
				else:
					r = mj.insIdx*14 + k[0] - 42
					c = mj.insIdx*14 + k[1]

				dfdY[r, c] += v
			
			"""
			dfdY[mj.orgIdx*14:(mj.orgIdx+1)*14, mj.orgIdx*14:(mj.orgIdx+1)*14] += dyd_Q_orginsdy_orgins[0:14,:]
			dfdY[mj.orgIdx*14:(mj.orgIdx+1)*14, mj.insIdx*14:(mj.insIdx+1)*14] += dyd_Q_orginsdy_orgins[14:28,:]
			dfdY[mj.insIdx*14:(mj.insIdx+1)*14, mj.orgIdx*14:(mj.orgIdx+1)*14] += dyd_Q_orginsdy_orgins[28:42,:]
			dfdY[mj.insIdx*14:(mj.insIdx+1)*14, mj.insIdx*14:(mj.insIdx+1)*14] += dyd_Q_orginsdy_orgins[42:56,:]
			"""
		
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
	

