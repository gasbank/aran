#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Forward simulation of a box without any contact
(NO GUI)
"""
from numpy import *
from math import *
from SymbolicMC import *
from SymbolicForce import *
from MathUtil import *
from dynamics import *
import sys

# Determine body constants
size = (1, 2, 3)
s1, s2, s3 = size
mass = 10
Hcm = diag([mass*(s2*s2+s3*s3)/12, mass*(s1*s1+s3*s3)/12, mass*(s1*s1+s2*s2)/12])
Hcminv = linalg.inv(Hcm)
h = 0.001
# Determine initial conditions
rcm = array([0.,0,0])      # Initial linear position
vcm = array([0.,0,0])      # Initial linear velocity
oRb = identity(3)          # Initial orientation
Lcm = array([0.,0,0])      # Initial angular momentum
# Compute initial auxiliary quantities
HcmX = dot(dot(oRb, Hcminv), oRb.T)
omega = dot(HcmX, Lcm)


noFrame = 100
for i in range(noFrame):
	M = SymbolicM_Global(oRb, size[0], size[1], size[2], mass)
	Minv = linalg.inv(M)
	C = SymbolicC_Global(oRb, omega, size[0], size[1], size[2], mass)
	ext=0
	ext = SymbolicForce_Manual2(oRb, (0, 0, 1), (0, 0.1, 0))
	
	acc = dot(Minv, ext - C)
	linacc = acc[0:3]
	angacc = acc[3:6]
	
	rcm = rcm + h*vcm
	vcm = vcm + h*linacc
	oRb = oRb + h*dot(cross_op_mat(omega),oRb)
	omega = omega + h*angacc
	print rcm
	
	oRb = OrthonormalizedOfOrientation(oRb)
	