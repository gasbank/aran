#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid box LCP-based simulator
(without MOSEK optimizer)
"""
from numpy import *
from math import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from lemke import *
from SymbolicMC import *
from SymbolicForce import *
from SymbolicPenetration import *
from SymbolicTensor import *
from glprim import *
from MathUtil import *
import sys
import PIL.Image as pil
import pygame
from pygame.font import *
from PmBody import *
from PmMuscle import *
from GlobalContext import *
import ctypes as ct

ESCAPE = '\033'

class BipedParameter:
	def __init__(self):
		self.p = {}
		# NOTE: set this in the unit of centimeters.
		# 1. Body dimension

		self.p['soleLen']           = 50.
		self.p['soleHeight']        = 8.
		self.p['soleWidth']         = 15.

		self.p['toeLen']            = 15.
		self.p['toeHeight']         = 5.
		self.p['toeWidth']          = 12.

		self.p['calfLen']           = 50.
		self.p['calfWidth']         = 8.
		self.p['calfLatWidth']      = 8.

		self.p['thighLen']          = 55.
		self.p['thighWidth']        = 12.
		self.p['thighLatWidth']     = 12.

		self.p['trunkLen']          = 70.
		self.p['trunkWidth']        = 45.
		self.p['trunkLatWidth']     = 25.


		# 2. Body gap (positive) or overlap (negative)
		#    Generally, overlapped one gives stable equilibrium condition
		self.p['trunkThighGap']     = -3.
		self.p['thighCalfGap']      = -3.
		self.p['calfSoleGap']       = -3.
		self.p['soleToeGap']        = -3.

		# 3. Other
		self.p['legsDist']          = self.p['trunkWidth']/2.


		# 4. Muscle Attachment Parameters
		self.p['hipMuscleDist']       = 20.
		self.p['hipMuscleLatDist']    = 14.
		self.p['hipMuscleDist2']      = self.p['thighWidth']
		self.p['hipMuscleLatDist2']   = self.p['thighLatWidth']
		self.p['hipMuscleInsLen']     = 4.

		self.p['ankleMuscleDist']     = self.p['calfWidth']
		self.p['ankleMuscleLatDist']  = self.p['calfLatWidth']
		self.p['ankleMuscleDist2']    = self.p['soleWidth']
		self.p['ankleMuscleLatDist2'] = self.p['soleWidth']
		self.p['ankleMuscleInsLen']   = 0

		# Convert the unit from centimeters to meters
		for (k, v) in self.p.iteritems():
			self.p[k] = v/100.

	def __getitem__(self, i):
		return self.p[i]

	def getBipedHeight(self):
		return (self.p['trunkLen']
				+ self.p['trunkThighGap']
				+ self.p['thighLen']
				+ self.p['thighCalfGap']
				+ self.p['calfLen']
				+ self.p['calfSoleGap']
				+ self.p['soleHeight'])

	def getTrunkPos(self):
		return array([0,
				      0,
				      self.getBipedHeight() - self.p['trunkLen']/2])
	def getThighPos(self):
		return array([self.p['legsDist']/2,
				      0,
				      self.p['thighLen']/2
				      + self.p['thighCalfGap']
				      + self.p['calfLen']
				      + self.p['calfSoleGap']
				      + self.p['soleHeight']])
	def getCalfPos(self):
		return array([self.p['legsDist']/2,
				      0,
				      self.p['calfLen']/2
				      + self.p['calfSoleGap']
				      + self.p['soleHeight']])
	def getSolePos(self):
		return array([self.p['legsDist']/2,
				      0,
				      self.p['soleHeight']/2])
	def getToePos(self):
		return array([self.p['legsDist']/2,
				      -self.p['soleLen']/2-self.p['soleToeGap']-self.p['toeLen']/2,
				      self.p['toeHeight']/2])
	def getBipedCenter(self):
		return array([0,0,self.getBipedHeight()/2])
	def getHipJointCenter(self):
		return self.getThighPos() + array([0,0,self['thighLen']/2+self['trunkThighGap']/2])
	def getKneeJointCenter(self):
		return self.getCalfPos() + array([0,0,self['calfLen']/2+self['thighCalfGap']/2])
	def getAnkleJointCenter(self):
		return self.getSolePos() + array([0,0,self['soleHeight']/2+self['calfSoleGap']/2])
	def getToeJointCenter(self):
		return self.getToePos() + array([0,self['toeLen']/2+self['soleToeGap']/2,0])

	def getKneeLigaments(self):
		c = self.getKneeJointCenter()
		p1 = c + array([0, -self['thighLatWidth']/2, self['thighCalfGap']/2])
		p2 = c + array([0, +self['thighLatWidth']/2, self['thighCalfGap']/2])
		p3 = c + array([0, -self['calfLatWidth']/2, -self['thighCalfGap']/2])
		p4 = c + array([0, +self['calfLatWidth']/2, -self['thighCalfGap']/2])

		p5 = c + array([-self['thighWidth']/2, 0, self['thighCalfGap']/2])
		p6 = c + array([+self['thighWidth']/2, 0, self['thighCalfGap']/2])
		p7 = c + array([-self['calfWidth']/2, 0, -self['thighCalfGap']/2])
		p8 = c + array([+self['calfWidth']/2, 0, -self['thighCalfGap']/2])

		return [ ('kneeLiga1L', p1,p4, 'thighL', 'calfL'),
				 ('kneeLiga2L', p2,p3, 'thighL', 'calfL'),
				 ('kneeLiga3L', p5,p7, 'thighL', 'calfL'),
				 ('kneeLiga4L', p6,p8, 'thighL', 'calfL') ]

	def getAnkleLigaments_1(self):
		# Original one
		c = self.getAnkleJointCenter()
		p1 = c + array([0, -self['calfLatWidth']/2, self['calfSoleGap']/2])
		p2 = c + array([0, +self['calfLatWidth']/2, self['calfSoleGap']/2])
		p3 = c + array([0, -self['soleLen']/2, -self['calfSoleGap']/2])
		p4 = c + array([0, +self['soleLen']/2, -self['calfSoleGap']/2])

		p5 = c + array([-self['calfWidth']/2, 0, self['calfSoleGap']/2])
		p6 = c + array([+self['calfWidth']/2, 0, self['calfSoleGap']/2])
		p7 = c + array([-self['soleWidth']/2, 0, -self['calfSoleGap']/2])
		p8 = c + array([+self['soleWidth']/2, 0, -self['calfSoleGap']/2])

		return [ ('ankleLiga1L', p1,p4, 'calfL', 'soleL'),
				 ('ankleLiga2L', p2,p3, 'calfL', 'soleL'),
				 ('ankleLiga3L', p5,p7, 'calfL', 'soleL'),
				 ('ankleLiga4L', p6,p8, 'calfL', 'soleL') ]
	def getAnkleLigaments_2(self):
		# Crossed (X)
		c = self.getAnkleJointCenter()
		p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2, self['calfSoleGap']/2])
		p2 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2, -self['calfSoleGap']/2])
		p3 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2, self['calfSoleGap']/2])
		p4 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2, -self['calfSoleGap']/2])

		p5 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2, self['calfSoleGap']/2])
		p6 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2, -self['calfSoleGap']/2])
		p7 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2, self['calfSoleGap']/2])
		p8 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2, -self['calfSoleGap']/2])

		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL'),
				 ('ankleLiga2L', p3,p4, 'calfL', 'soleL'),
				 ('ankleLiga2L', p5,p6, 'calfL', 'soleL'),
				 ('ankleLiga2L', p7,p8, 'calfL', 'soleL') ]
	def getAnkleLigaments_3(self):
		# Vertical only (| |)
		c = self.getAnkleJointCenter()
		p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p2 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2, -self['calfSoleGap']/2])
		p3 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p4 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2, -self['calfSoleGap']/2])

		p5 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p6 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2, -self['calfSoleGap']/2])
		p7 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p8 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2, -self['calfSoleGap']/2])

		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL'),
				 ('ankleLiga2L', p3,p4, 'calfL', 'soleL'),
				 ('ankleLiga2L', p5,p6, 'calfL', 'soleL'),
				 ('ankleLiga2L', p7,p8, 'calfL', 'soleL') ]
	
	def getAnkleLigaments_4(self):
		# Inclined (/ \)
		c = self.getAnkleJointCenter()
		p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p2 = c + array([ self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
		p3 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p4 = c + array([ self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])

		p5 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p6 = c + array([-self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
		p7 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
		p8 = c + array([-self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])

		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL'),
				 ('ankleLiga2L', p3,p4, 'calfL', 'soleL'),
				 ('ankleLiga2L', p5,p6, 'calfL', 'soleL'),
				 ('ankleLiga2L', p7,p8, 'calfL', 'soleL') ]
	
	def getAnkleLigaments_5(self):
		# Inclined (/ \)
		c = self.getAnkleJointCenter()
		p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
		p2 = c + array([ self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
		p3 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p4 = c + array([ self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])

		p5 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p6 = c + array([-self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
		p7 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p8 = c + array([-self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])

		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL'),
				 ('ankleLiga2L', p3,p4, 'calfL', 'soleL'),
				 ('ankleLiga2L', p5,p6, 'calfL', 'soleL'),
				 ('ankleLiga2L', p7,p8, 'calfL', 'soleL') ]
	
	def getAnkleLigaments(self):
		# Single ligament
		c = self.getAnkleJointCenter()
		p1 = c + array([ 0, 0,  self['calfSoleGap']/2])
		p2 = c + array([ 0, 0, -self['calfSoleGap']/2])
		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL') ]

	def getToeLigaments(self):
		c = self.getToeJointCenter()
		p1 = c + array([0, self['soleToeGap']/2, self['toeHeight']/2])
		p2 = c + array([0, self['soleToeGap']/2, -self['toeHeight']/2])
		p3 = c + array([0, -self['soleToeGap']/2, self['toeHeight']/2])
		p4 = c + array([0, -self['soleToeGap']/2, -self['toeHeight']/2])

		p5 = c + array([ self['toeWidth']/2, self['soleToeGap']/2, 0])
		p6 = c + array([-self['toeWidth']/2, self['soleToeGap']/2, 0])
		p7 = c + array([ self['toeWidth']/2, -self['soleToeGap']/2, 0])
		p8 = c + array([-self['toeWidth']/2, -self['soleToeGap']/2, 0])

		return [ ('toeLiga1L', p1,p4, 'soleL', 'toeL'),
				 ('toeLiga2L', p2,p3, 'soleL', 'toeL'),
				 ('toeLiga3L', p5,p7, 'soleL', 'toeL'),
				 ('toeLiga4L', p6,p8, 'soleL', 'toeL') ]

	def getHipLigaments(self):
		c = self.getHipJointCenter()
		p1 = c + array([0, 0, self['trunkThighGap']/2])
		p2 = c + array([0, 0, -self['trunkThighGap']/2])
		p3 = c + array([0, -self['trunkLatWidth']/2, self['trunkThighGap']/2])
		p4 = c + array([0, -self['thighLatWidth']/2, -self['trunkThighGap']/2])
		p5 = c + array([0, self['trunkLatWidth']/2, self['trunkThighGap']/2])
		p6 = c + array([0, self['thighLatWidth']/2, -self['trunkThighGap']/2])
		p7 = c + array([self['hipMuscleDist']/2, 0, self['trunkThighGap']/2])
		p8 = c + array([self['hipMuscleDist2']/2, 0, -self['trunkThighGap']/2])
		p9 = c + array([-self['hipMuscleDist']/2, 0, self['trunkThighGap']/2])
		p10 = c + array([-self['hipMuscleDist2']/2, 0, -self['trunkThighGap']/2])
		return [ ('hipLiga1L', p1,p2, 'trunk', 'thighL'),
				 ('hipLiga2L', p3,p4, 'trunk', 'thighL'),
				 ('hipLiga3L', p5,p6, 'trunk', 'thighL'),
				 ('hipLiga4L', p7,p8, 'trunk', 'thighL'),
				 ('hipLiga5L', p9,p10, 'trunk', 'thighL') ]
	def getHamstring(self):
		c1 = self.getSolePos()
		c2 = self.getThighPos()
		p1 = c1 + array([ 0,  self['soleLen']/2, 0])
		p2 = c2 + array([ 0,  self['thighLatWidth']/2, -self['thighLen']/2])
		p3 = c1 + array([ 0,  -self['soleLen']/2, 0])
		p4 = c2 + array([ 0,  -self['thighLatWidth']/2, -self['thighLen']/2])
		
		p5 = c1 + array([ self['soleWidth']/2,  0, 0])
		p6 = c2 + array([ self['thighWidth']/2,  0, -self['thighLen']/2])
		p7 = c1 + array([ -self['soleWidth']/2,  0, 0])
		p8 = c2 + array([ -self['thighWidth']/2,  0, -self['thighLen']/2])
		
		return [ ('hamstring1L', p1,p2, 'soleL', 'thighL'),
		         ('hamstring2L', p3,p4, 'soleL', 'thighL'),
		         ('hamstring3L', p5,p6, 'soleL', 'thighL'),
		         ('hamstring4L', p7,p8, 'soleL', 'thighL') ]
	
	def getGastro(self):
		'''
		c = self.getAnkleJointCenter()
		p1 = c + array([ 0, self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/4])
		p2 = c + array([ 0, self['soleLen']/2, -self['calfSoleGap']/2])
		return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL') ]
		'''
		
		c = self.getAnkleJointCenter()
		p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
		p2 = c + array([ self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
		p3 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p4 = c + array([ self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])

		p5 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p6 = c + array([-self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
		p7 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
		p8 = c + array([-self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])

		return [ ('gastro1L', p1,p2, 'calfL', 'soleL'),
				 ('gastro2L', p3,p4, 'calfL', 'soleL'),
				 ('gastro3L', p5,p6, 'calfL', 'soleL'),
				 ('gastro4L', p7,p8, 'calfL', 'soleL') ]
		
	def getTibialis(self):
		c1 = self.getToePos()
		c2 = self.getCalfPos()
		p1 = c1 + array([0, -self['toeLen']/2, self['toeHeight']/2])
		p2 = c2 + array([0, -self['calfLatWidth']/2, -self['calfLen']/4])
		return [ ('TibialisL', p1,p2, 'toeL', 'calfL') ]
	def getBicepsFemoris(self):
		c1 = self.getHipJointCenter()
		c2 = self.getThighPos()
		p1 = c1 + array([0, -self['trunkLatWidth']/2, 0])
		p2 = c2 + array([0, -self['thighLatWidth']/2, 0])
		p3 = c1 + array([0, +self['trunkLatWidth']/2, 0])
		p4 = c2 + array([0, +self['thighLatWidth']/2, 0])
		p5 = c1 + array([-self['legsDist']/2, 0, 0])
		p6 = c2 + array([-self['thighWidth']/2, 0, 0])
		p7 = c1 + array([+self['legsDist']/2, 0, 0])
		p8 = c2 + array([+self['thighWidth']/2, 0, 0])
		return [ ('BicepsFemoris1L', p1,p2, 'trunk', 'thighL'),
		         ('BicepsFemoris2L', p3,p4, 'trunk', 'thighL'),
		         ('BicepsFemoris3L', p5,p6, 'trunk', 'thighL'),
		         ('BicepsFemoris4L', p7,p8, 'trunk', 'thighL')  ]
	
	def getAllLigaments(self):
		return self.getKneeLigaments() + self.getAnkleLigaments() + self.getToeLigaments() + self.getHipLigaments()
		#return []
	
	def getAllMuscles(self):
		return self.getHamstring() + self.getTibialis() + self.getBicepsFemoris() + self.getGastro()

	def buildBody(self):
		body = []
		l = [ ('trunk',  [self['trunkWidth'], self['trunkLatWidth'], self['trunkLen']], self.getTrunkPos(), 1.),
			  ('thighL', [self['thighWidth'], self['thighLatWidth'], self['thighLen']], self.getThighPos(), 3.),
			  ('calfL',  [self['calfWidth'], self['calfLatWidth'], self['calfLen']],    self.getCalfPos(),  3.),
			  ('soleL',  [self['soleWidth'], self['soleLen'], self['soleHeight']],      self.getSolePos(),  3.),
			  ('toeL',   [self['toeWidth'], self['toeLen'], self['toeHeight']],         self.getToePos(),   3.) ]


		# DEBUGGING START
		l = l[2:4]
		# DEBUGGING NED


		for i, ll in zip(range(len(l)), l):
			identityQuaternion = [1.,0,0,0]
			pos0 = hstack([ ll[2], identityQuaternion ])
			vel0 = zeros(7)
			drawingColor = (0.2,0.1,0.2)

			'''
			#DEBUGGING START
			identityQuaternion = [1,1,2,3]
			identityQuaternion = quat_normalize(identityQuaternion)
			pos0[3:7] = identityQuaternion
			pos0[2] += 0.2 # Start from the sky
			vel0[3:7] = [0.1,0.2,0.3,0.4]
			#DEBUGGING END
			'''

			#pos0[2] += 1.5 # Start from the sky
			
			name = ll[0]
			#name, pName, mass, boxsize, q, qd, dc, rotParam
			b = PmBody(name, None, ll[3], ll[1], pos0, vel0, drawingColor, 'QUAT_WFIRST')
			body.append(b)
			'''
			if name[-1] == 'L':
				# If there are left body segments, we also need right counterparts
				newName = name[:-1] + 'R'
				newPos0 = copy(pos0) # Deep copy
				newPos0[0] *= -1
				newVel0 = copy(vel0) # Deep copy
				b = PmBody(newName, None, ll[3], ll[1], newPos0, newVel0, drawingColor, 'QUAT_WFIRST')
				body.append(b)
				'''

		return body
	
	def buildFiber(self, availableBodyNames):
		# Fiber constants for a ligament
		KSE = 1e50
		KPE = 0.
		b   = 1e50
		T   = 0.
		A   = 0.
		fiber_liga   = self._buildFiber(availableBodyNames, self.getAllLigaments(), KSE, KPE, b, T, A)
		# Fiber constants for a muscle fiber
		KSE = 1e4
		KPE = 1e4
		b   = 1e2
		T   = 0.
		A   = 0.
		fiber_muscle = self._buildFiber(availableBodyNames, self.getAllMuscles(), KSE, KPE, b, T, A)
		return fiber_liga + fiber_muscle
	def _buildFiber(self, availableBodyNames, fiberList, KSE, KPE, b, T, A):
		fiber = []

		for (name, orgPosGlobal, insPosGlobal, orgBody, insBody) in fiberList:
			#orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
			orgPosLocal = self.changeToLocal(orgBody, orgPosGlobal)
			insPosLocal = self.changeToLocal(insBody, insPosGlobal)
			xrest = linalg.norm(orgPosGlobal - insPosGlobal)
			cfg = (orgBody, orgPosLocal, insBody, insPosLocal, KSE, KPE, b, xrest, T, A)
			m = PmMuscle(name, cfg, 'LIGAMENT', False)
			if orgBody in availableBodyNames and insBody in availableBodyNames:
				fiber.append(m)

			if name[-1] == 'L':
				# If there are left-side fibers, we also need right counterparts
				newName = name[:-1] + 'R'
				#orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
				if orgBody[-1] == 'L':
					newOrgBody = orgBody[:-1] + 'R'
				else:
					newOrgBody = orgBody
				if insBody[-1] == 'L':
					newInsBody = insBody[:-1] + 'R'
				else:
					newInsBody = insBody

				newOrgPosGlobal = copy(orgPosGlobal)
				newOrgPosGlobal[0] *= -1
				newInsPosGlobal = copy(insPosGlobal)
				newInsPosGlobal[0] *= -1
				orgPosLocal = self.changeToLocal(newOrgBody, newOrgPosGlobal)
				insPosLocal = self.changeToLocal(newInsBody, newInsPosGlobal)
				xrest = linalg.norm(orgPosGlobal - insPosGlobal)
				cfg = (newOrgBody, orgPosLocal, newInsBody, insPosLocal, KSE, KPE, b, xrest, T, A)
				m = PmMuscle(newName, cfg, 'LIGAMENT', False)
				if newOrgBody in availableBodyNames and newInsBody in availableBodyNames:
					fiber.append(m)
		return fiber

	def changeToLocal(self, bodyName, globalPos):
		flipX = (bodyName[-1] == 'R')
		bodyOrigin = 0
		if bodyName == 'trunk': bodyOrigin = self.getTrunkPos()
		elif bodyName[:-1] == 'thigh': bodyOrigin = self.getThighPos()
		elif bodyName[:-1] == 'calf': bodyOrigin = self.getCalfPos()
		elif bodyName[:-1] == 'sole': bodyOrigin = self.getSolePos()
		elif bodyName[:-1] == 'toe' : bodyOrigin = self.getToePos()
		else: raise Exception('What the...')
		if flipX:
			bodyOrigin[0] *= -1
		return globalPos - bodyOrigin

	def getBipedBB(self):
		"""
		A bounding box for the whole biped (for adjusting the camera)
		"""
		maxX = max(self['trunkWidth']/2,
				   self['legsDist']/2 + self['thighWidth']/2,
				   self['legsDist']/2 + self['calfWidth']/2,
				   self['legsDist']/2 + self['soleWidth']/2,
				   self['legsDist']/2 + self['toeWidth']/2)
		minX = -maxX

		maxY = max(self['trunkLatWidth']/2,
				   self['thighLatWidth']/2,
				   self['calfLatWidth']/2,
				   self['soleLen']/2)
		minY = min(-maxY, -self['soleLen']/2-self['soleToeGap']-self['toeLen'])

		maxZ = self.getBipedHeight()
		minZ = 0.

		return (minX, minY, minZ, maxX, maxY, maxZ)


def InitializeGl():				# We call this right after our OpenGL window is created.
	glClearColor(0,0,0,1.0)
	glClearDepth(1.0)									# Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL)								# The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST)								# Enables Depth Testing
	glShadeModel (GL_FLAT);								# Select Flat Shading (Nice Definition Of Objects)
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) 	# Really Nice Perspective Calculations
	#glEnable (GL_LIGHT0)
	#glEnable (GL_LIGHTING)
	glEnable (GL_COLOR_MATERIAL)
	#glEnable(GL_TEXTURE_2D)
	#glLineWidth(2)
	#gQuadric = gluNewQuadric(1)
	#glDisable(GL_CULL_FACE);

def ResizeGlScene(winWidth, winHeight):
	if winHeight == 0:
		winHeight = 1
	global gWinWidth, gWinHeight
	gWinWidth  = winWidth
	gWinHeight = winHeight

def KeyPressed(*args):
	global gWireframe, gPoi
	key = args [0]
	if key == ESCAPE: sys.exit ()
	elif key == 'z': gWireframe = not gWireframe
	elif key >= '1' and key <= '5': gPoi = int(key)-1

def Draw():

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)


	# Point Of Interest
	global gPoi
	poiName, zoomLevel = gPoiList[gPoi]
	if poiName == 'Biped': poi = gBiped.getBipedCenter()
	elif poiName == 'Hip': poi = gBiped.getHipJointCenter()
	elif poiName == 'Knee': poi = gBiped.getKneeJointCenter()
	elif poiName == 'Ankle': poi = gBiped.getAnkleJointCenter()
	elif poiName == 'Toe': poi = gBiped.getToeJointCenter()
	else: raise Exception('What the...')

	SetViewport(0, 0, gWinWidth/2, gWinHeight/2, zoomLevel)
	gluLookAt(poi[0], -100, poi[2],
		      poi[0], 0, poi[2],
		      0, 0, 1);
	DrawBiped()

	SetViewport(gWinWidth/2, 0, gWinWidth/2, gWinHeight/2, zoomLevel)
	gluLookAt(100, poi[1], poi[2],
		      0, poi[1], poi[2],
		      0, 0, 1);
	DrawBiped()

	SetViewport(0, gWinHeight/2, gWinWidth/2, gWinHeight/2, zoomLevel)
	gluLookAt(poi[0], poi[1], 100,
		      poi[0], poi[1], 0,
		      0, 1, 0);
	DrawBiped()

	SetViewport(gWinWidth/2, gWinHeight/2, gWinWidth/2, gWinHeight/2, zoomLevel)
	gluLookAt(poi[0]+1, poi[1]-1.5, poi[2]+1,
		      poi[0], poi[1], poi[2],
		      0, 0, 1);
	DrawBiped()



	SetViewport(0,0, gWinWidth,gWinHeight, 0)
	glDisable(GL_DEPTH_TEST)
	glColor(0.8,0.8,0.8)
	glBegin(GL_LINES)
	glVertex(-1,0,0)
	glVertex(1,0,0)
	glVertex(0,-1,0)
	glVertex(0,1,0)
	glEnd()
	glEnable(GL_DEPTH_TEST)

	glutSwapBuffers()


def SetViewport(x, y, w, h, zoomLevel):
	glViewport(x, y, w, h)
	aspectRatio = float(gWinWidth) / gWinHeight
	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	if zoomLevel != 0:
		glOrtho(-aspectRatio/zoomLevel, aspectRatio/zoomLevel,
				-1./zoomLevel, 1./zoomLevel,
				1, 1000)
	glMatrixMode(GL_MODELVIEW)		# // Select The Modelview Matrix
	glLoadIdentity()					# // Reset The Modelview Matrix

def DrawBiped():
	trunkPos = gBiped.getTrunkPos()
	glPushMatrix()
	glTranslate(trunkPos[0],trunkPos[1],trunkPos[2])
	glScale(gBiped['trunkWidth'], gBiped['trunkLatWidth'], gBiped['trunkLen'])
	glColor(1,0,1)
	if gWireframe: glutWireCube(1)
	else: glutSolidCube(1)
	glPopMatrix()

	glLineWidth(1)
	# Left leg drawn
	DrawLeg()

	# Right leg drawn
	glPushMatrix()
	glScale(-1,1,1)
	DrawLeg()
	glPopMatrix()

	
	glLineWidth(2)
	glBegin(GL_LINES)
	glColor(1,1,1) # Ligament is white
	for (muscleName, orgPos, insPos, orgName, insName) in gBiped.getAllLigaments():
		glVertex(orgPos[0], orgPos[1], orgPos[2])
		glVertex(insPos[0], insPos[1], insPos[2])
	glColor(1,0,0) # Muscle is red
	for (muscleName, orgPos, insPos, orgName, insName) in gBiped.getAllMuscles():
		glVertex(orgPos[0], orgPos[1], orgPos[2])
		glVertex(insPos[0], insPos[1], insPos[2])
	glEnd()

def DrawLeg():
	thighPos = gBiped.getThighPos()
	glPushMatrix()
	glTranslate(thighPos[0],thighPos[1],thighPos[2])
	glScale(gBiped['thighWidth'], gBiped['thighLatWidth'], gBiped['thighLen'])
	glColor(1,1,0)
	if gWireframe: glutWireCube(1)
	else: glutSolidCube(1)
	glPopMatrix()

	calfPos = gBiped.getCalfPos()
	glPushMatrix()
	glTranslate(calfPos[0],calfPos[1],calfPos[2])
	glScale(gBiped['calfWidth'], gBiped['calfLatWidth'], gBiped['calfLen'])
	glColor(1,0,0)
	if gWireframe: glutWireCube(1)
	else: glutSolidCube(1)
	glPopMatrix()

	solePos = gBiped.getSolePos()
	glPushMatrix()
	glTranslate(solePos[0],solePos[1],solePos[2])
	glScale(gBiped['soleWidth'], gBiped['soleLen'], gBiped['soleHeight'])
	glColor(1,1,1)
	if gWireframe: glutWireCube(1)
	else: glutSolidCube(1)
	glPopMatrix()

	toePos = gBiped.getToePos()
	glPushMatrix()
	glTranslate(toePos[0],toePos[1],toePos[2])
	glScale(gBiped['toeWidth'], gBiped['toeLen'], gBiped['toeHeight'])
	glColor(0,1,0)
	if gWireframe: glutWireCube(1)
	else: glutSolidCube(1)
	glPopMatrix()

def Main():
	glutInit(sys.argv)
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
	glutInitWindowSize(gWinWidth, gWinHeight)
	glutInitWindowPosition(0, 0)
	window = glutCreateWindow("Muscle")
	glutDisplayFunc(Draw)
	glutIdleFunc(Draw)
	glutReshapeFunc(ResizeGlScene)
	glutKeyboardFunc(KeyPressed)
	InitializeGl()
	glutMainLoop()


gQuadric = None
gWinWidth = 700
gWinHeight = 700
gWireframe = False
gPoiList = ( ('Biped', 1.),
             ('Hip', 5.),
             ('Knee', 5.),
             ('Ankle', 5.),
             ('Toe', 5.) )
gPoi = 0 # Point of interest
if __name__ == '__main__':
	gBiped = BipedParameter()
	print 'Biped Height =', gBiped.getBipedHeight()
	Main()
