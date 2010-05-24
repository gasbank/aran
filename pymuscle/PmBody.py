#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid body class
"""
from SymbolicTensor import *
from MathUtil import *
from quat import *
from dynamics import BoxInertia

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
		self.q       = q               # Stored in World Coordinates
		self.qd      = qd              # Stored in World or body coordinates
		self.qdCo    = 'WC'            # 'WC' or 'BC'
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
		
		self.ME = identity(3) * self.mass
		self.IN = BoxInertia(self.boxsize, self.mass)
		self.M_BC = self.getMassMatrix_BC()
	
	def globalPos(self, localPos):
		'''
		Transform local coordinate position(point) to global coordinate position
		'''
		if self.rotParam == 'EULER_XYZ':
			A = RotationMatrixFromEulerAngles_xyz(self.q[3], self.q[4], self.q[5])
		elif self.rotParam == 'EULER_ZXZ':
			A = RotationMatrixFromEulerAngles_zxz(self.q[3], self.q[4], self.q[5])
		elif self.rotParam == 'QUAT_WFIRST':
			A = RotationMatrixFromQuaternion(self.q[3], self.q[4], self.q[5], self.q[6])
		else:
			raise Exception, 'Unknown rotation parameterization'
		return self.q[0:3] + dot(A, localPos)
	def localVec(self, globalVec):
		'''
		Transform global coordinate vector to local coordinate vector
		'''
		if self.rotParam == 'QUAT_WFIRST':
			A = RotationMatrixFromQuaternion(self.q[3], -self.q[4], -self.q[5], -self.q[6])
		else:
			raise Exception, 'Unknown rotation parameterization'
		return dot(A, globalVec)
		
	def __str__(self):
		ret = 'PmBody %s, parent %s\n' % (self.name, self.pName)
		ret = ret + 'q: %s\n' % self.q
		ret = ret + 'qd: %s (%s)' % (self.qd, self.rotParam)
		return ret
	
	def getCorners_WC(self):
		assert self.rotParam == 'QUAT_WFIRST'
		cornersWc = []
		for c in self.corners:
			cw = self.q[0:3] + quat_rot(self.q[3:7], c)
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
	def getCoriolisVector_BC(self):
		assert self.rotParam == 'QUAT_WFIRST'
		
		omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
		return hstack([zeros(3), array(cross(omega, dot(self.IN, omega)))])
	def getExtVector_BC(self):
		omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
		V1_world = array([0,0,-9.81]) * self.mass
		
		V1_body = quat_rot(quat_conj(self.q[3:7]), V1_world)
		V2_body = -array(cross(omega, dot(self.IN, omega)))
		return hstack([V1_body, V2_body])

	def getIN_WC(self, q=None):
		if q==None:
			q = self.q
		A = RotationMatrixFromQuaternion(q[3],q[4],q[5],q[6])
		return dot(dot(A, self.IN), A.T)
	
	def getExtVector_WC(self, q=None, qd=None):
		if q==None:
			q = self.q
		if qd==None:
			qd = self.qd
		omega = QuatToAngularVel_WC(q[3:7], qd[3:7])
		#omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
		
		V1_world = array([0,0,-9.81]) * self.mass
		#V1_world = array([0,0,-9.81]) * 0
		
		#V2_body = -array(cross(omega, dot(self.IN, omega)))
		V2_world = -array(cross(omega, dot(self.getIN_WC(q), omega)))
		return hstack([V1_world, V2_world])
		
	def explicitStep(self, h, acc):
		assert self.rotParam == 'QUAT_WFIRST'
		self.q  += self.qd  * h
		self.qd += array(acc) * h
		
	def getLinearAngularVelocity_BC(self):
		assert self.rotParam == 'QUAT_WFIRST'
		if self.qdCo == 'BC':
			qdLin = self.qd[0:3]
		else:
			qdLin = quat_rot(quat_conj(self.q[3:7]), self.qd[0:3])
			
		return hstack([ qdLin, QuatToAngularVel_BC( self.q[3:7], self.qd[3:7] ) ])
	def getLinearAngularVelocity_WC(self):
		assert self.rotParam == 'QUAT_WFIRST'
		if self.qdCo == 'BC':
			qdLin = quat_rot(self.q[3:7], self.qd[0:3])
		else:
			qdLin = self.qd[0:3]
			
		return hstack([ qdLin, QuatToAngularVel_WC( self.q[3:7], self.qd[3:7] ) ])