#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Test file
"""
from numpy import *

def box_inertia(s, m):
	return diag([m*(s[1]**2+s[2]**2)/12.0,
		         m*(s[0]**2+s[2]**2)/12.0,
		         m*(s[0]**2+s[1]**2)/12.0])

def main():
	"""
	Program entry point.
	Define mass, box size, inertia, initial state.
	"""
	m = 1
	box_size = array([0.2,0.2,0.1])
	H_com_bar = box_inertia(box_size, m)
	H_com_inv_bar = linalg.inv(H_com_bar)


main()
