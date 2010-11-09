#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid body class using the exponential map rotation
"""
from MathUtil import *
from quat import *
from dynamics import BoxInertia
from dRdv_real import *
from numpy import hstack, identity, hstack, vstack, array
from SymbolicTensor import *

class ExpBody:
	def __init__(self, name, pName, mass, boxsize, q, linvel, angvel, dc):
		"""
		mass: mass of the body
		size: size tuple (sx, sy, sz) of rigid body
		q   : linear & angular position (6 dimension)
		qd  : linear & angular velocity (6 dimension)
		dc  : drawing color
		"""
		sx, sy, sz = boxsize
		
		self.name       = name
		self.pName      = pName
		self.mass       = mass
		self.boxsize    = boxsize
		self.q          = q
		#self.omega_wc   = omega_wc
		#self.setQd(linvel, omega_wc)  # setQd() sets self.qd
		self.qd         = hstack([linvel, angvel])
		
		self.qdCo       = 'EXP'
		self.dc         = dc
		self.rho        = mass / (sx*sy*sz) # density of the body
		self.I          = SymbolicTensor(sx, sy, sz, self.rho)
		self.corners    = [ ( sx/2,  sy/2,  sz/2),
		                    ( sx/2,  sy/2, -sz/2),
		                    ( sx/2, -sy/2,  sz/2),
		                    ( sx/2, -sy/2, -sz/2),
		                    (-sx/2,  sy/2,  sz/2),
		                    (-sx/2,  sy/2, -sz/2),
		                    (-sx/2, -sy/2,  sz/2),
		                    (-sx/2, -sy/2, -sz/2) ]
		self.rotParam   = 'EXP'
		self.ME         = identity(3) * self.mass
		self.IN         = BoxInertia(self.boxsize, self.mass)
		self.M_BC       = self.getMassMatrix_BC()
		self.cf         = []

	def setQd(self, linvel, omega_wc):
		self.qd         = hstack([linvel, VdotFromOmega(self.q[3:6], omega_wc)])
		
	def globalPos(self, localPos):
		'''
		Transform local coordinate position(point) to global coordinate position
		'''
		return self.q[0:3] + dot(self.getRotMat(), localPos)
	def localVec(self, globalVec):
		'''
		Transform global coordinate vector to local coordinate vector
		'''
		return dot(linalg.inv(self.getRotMat()), globalVec)
	
	def getRotMat(self):
		return RotationMatrixFromV(self.q[3:6])
	
	def __str__(self):
		ret = 'ExpBody %s, parent %s\n' % (self.name, self.pName)
		ret = ret + '   q: %s\n' % self.q
		ret = ret + '   qd: %s (%s)\n' % (self.qd, self.rotParam)
		if hasattr(self, 'extForce'):
			ret = ret + '   extForce: %s' % self.extForce
		return ret
	
	def getCorners_WC(self):
		cornersWc = []
		for c in self.corners:
			cw = self.q[0:3] + dot(self.getRotMat(), c)
			cornersWc.append(cw)
		return cornersWc
	
	def getMassMatrix_BC(self):
		ret = vstack([  hstack([ self.ME, zeros((3,3)) ]),
						hstack([ zeros((3,3)), self.IN ]) ])
		return ret
	def getMassMatrix_WC(self, q=None):
		if q==None:
			q = self.q
		ret = vstack([  hstack([ self.ME     , zeros((3,3))    ]),
						hstack([ zeros((3,3)), self.getIN_WC(q) ])   ])
		return ret
	def getCoriolisVector_WC(self, q=None, omega_wc =None):
		if q==None:
			q = self.q
		if omega_wc==None:
			omega_wc = self.omega_wc
		return hstack([zeros(3), array(cross(self.omega_wc, dot(self.getIN_WC(q), self.omega_wc)))])
	def getIN_WC(self, q=None):
		if q==None:
			A = self.getRotMat()
		else:
			A = RotationMatrixFromV(q[3:6])
		return dot(dot(A, self.IN), A.T)
	