#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Symbolic form of penetration constraints for a rigid body (FLAT ground)
"""
from numpy import *
from math import pow, sin, cos

def SymbolicPenetration(q):
	gf = zeros((6))

	cx, cy, cz, phi, theta, psix = q

	cc[0] = 0.0
	cc[1] = 0.0
	cc[2] = 1.0
	cc[3] = px*cos(phi)*sin(theta)-py*sin(phi)*sin(theta)
	cc[4] = -pz*sin(theta)+py*cos(phi)*cos(theta)+px*cos(theta)*sin(phi)
	cc[5] = 0.0
	return cc
	
