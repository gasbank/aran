#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid body class
"""
from SymbolicTensor import *
from MathUtil import *

class PmBody:
	def __init__(self, name, pName, mass, boxsize, q, qd, dc, rotParam):
		"""
		mass: mass of the body
		size: size tuple (sx, sy, sz) of rigid body
		q   : linear & angular position (6 dimension)
		qd  : linear & angular velocity (6 dimension) -- which parameterization?
		dc  : drawing color
		"""
		sx, sy, sz = boxsize
		
		self.name    = name
		self.pName   = pName
		self.mass    = mass
		self.boxsize = boxsize
		self.q       = q
		self.qd      = qd
		self.dc      = dc
		self.rho     = mass / (sx*sy*sz) # density of the body
		self.I       = SymbolicTensor(sx, sy, sz, self.rho)
		self.corners = [ ( sx/2,  sy/2,  sz/2),
		                 ( sx/2,  sy/2, -sz/2),
		                 ( sx/2, -sy/2,  sz/2),
		                 ( sx/2, -sy/2, -sz/2),
		                 (-sx/2,  sy/2,  sz/2),
		                 (-sx/2,  sy/2, -sz/2),
		                 (-sx/2, -sy/2,  sz/2),
		                 (-sx/2, -sy/2, -sz/2) ]
		assert(rotParam in ['EULER_XYZ', 'EULER_ZXZ', 'QUAT_WFIRST'])
		self.rotParam  = rotParam
	
	def globalPos(self, localPos):
		if self.rotParm == 'EULER_XYZ':
			A = RotationMatrixFromEulerAngles_xyz(self.q[3], self.q[4], self.q[5])
		elif self.rotParm == 'EULER_ZXZ':
			A = RotationMatrixFromEulerAngles_zxz(self.q[3], self.q[4], self.q[5])
		elif self.rotParm == 'QUAT_WFIRST':
			A = RotationMatrixFromQuaternion(self.q[3], self.q[4], self.q[5], self.q[6])
		return self.q[0:3] + dot(A, localPos)
	
	def __str__(self):
		ret = 'PmBody %s, parent %s\n' % (self.name, self.pName)
		ret = ret + 'q: %s\n' % self.q
		ret = ret + 'qd: %s (%s)' % (self.qd, self.rotParam)
		return ret