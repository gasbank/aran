#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Muscle/ligament fiber class
"""
from SymbolicTensor import *
from MathUtil import *

class PmMuscle:
	def __init__(self, name, orgBody, orgPos, insBody, insPos, mType, bAttachedPosNormalized):
		"""
		orgBody/Pos: the muscle fiber's origin body name
					and position in local coordinates
		insBody/Pos: the muscle fiber's insertion body name
					and position in local coordinates
		"""
		self.name    = name
		self.orgBody = orgBody
		self.orgPos  = orgPos
		self.insBody = insBody
		self.insPos  = insPos
		self.mType   = mType
		assert(mType in ['MUSCLE', 'LIGAMENT'])
		self.bAttachedPosNormalized = bAttachedPosNormalized
		assert(bAttachedPosNormalized in [True, False])
		#
		# Dynamics properties
		#
		self.KSE     = 5000.
		self.KPE     = 4500.
		self.b       = 200.
		self.xrest   = 0.45
		self.T       = 0
		self.A       = 0