#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Inertia tensor matrix defined in [Jain 2009].
For box shaped body.
"""
from numpy import *
from math import pow, sin, cos

def SymbolicTensor(sx, sy, sz, rho):
	Ixx = rho*(sx*sx*sx)*sy*sz*(1.0/1.2E1);
	Iyy = rho*sx*(sy*sy*sy)*sz*(1.0/1.2E1);
	Izz = rho*sx*sy*(sz*sz*sz)*(1.0/1.2E1);
	Iww = rho*sx*sy*sz;
	return Ixx, Iyy, Izz, Iww
	
