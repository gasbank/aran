#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Test file
"""
from numpy import *
from dynamics import *

def main():
	"""
	Program entry point.
	Define mass, box size, inertia, initial state.
	"""
	m = 1
	box_size = array([0.2,0.2,0.1])
	H_com_bar = BoxInertia(box_size, m)
	H_com_inv_bar = linalg.inv(H_com_bar)
	
	r = array([0.,0,box_size[2]/2.0])
	v = array([0.,0,0])
	A = identity(3)
	omega = array([0.,0,0])


main()
