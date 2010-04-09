#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Symbolic form of generalized forces for a rigid body
"""
from numpy import *
from math import pow, sin, cos

def SymbolicForce(q, f, p):
	gf = zeros((6))

	cx, cy, cz, phi, theta, psix = q
	fx, fy, fz = f
	px, py, pz = p

	gf[0] = fx
	gf[1] = fy
	gf[2] = fz
	gf[3] = fz*(px*cos(phi)*sin(theta)-py*sin(phi)*sin(theta))-fx*(px*(cos(psix)*sin(phi)+cos(phi)*cos(theta)*sin(psix))+py*(cos(phi)*cos(psix)-cos(theta)*sin(phi)*sin(psix)))-fy*(px*(sin(phi)*sin(psix)-cos(phi)*cos(psix)*cos(theta))+py*(cos(phi)*sin(psix)+cos(psix)*cos(theta)*sin(phi)))
	gf[4] = -fy*(pz*cos(psix)*cos(theta)+py*cos(phi)*cos(psix)*sin(theta)+px*cos(psix)*sin(phi)*sin(theta))+fx*(pz*cos(theta)*sin(psix)+py*cos(phi)*sin(psix)*sin(theta)+px*sin(phi)*sin(psix)*sin(theta))+fz*(-pz*sin(theta)+py*cos(phi)*cos(theta)+px*cos(theta)*sin(phi))
	gf[5] = fx*(-px*(cos(phi)*sin(psix)+cos(psix)*cos(theta)*sin(phi))+py*(sin(phi)*sin(psix)-cos(phi)*cos(psix)*cos(theta))+pz*cos(psix)*sin(theta))+fy*(px*(cos(phi)*cos(psix)-cos(theta)*sin(phi)*sin(psix))-py*(cos(psix)*sin(phi)+cos(phi)*cos(theta)*sin(psix))+pz*sin(psix)*sin(theta))

	return gf
	
