#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Utility functions related to dynamics system
"""
from numpy import *
from math import *
def BoxInertia(box_size, mass):
	return diag([
		mass*(box_size[1]**2+box_size[2]**2)/12.0,
		mass*(box_size[0]**2+box_size[2]**2)/12.0,
		mass*(box_size[0]**2+box_size[1]**2)/12.0  ])


def cross_op_mat(v):
	assert len(v) is 3
	return array( [ [0,-v[2],v[1]], [v[2],0,-v[0]], [-v[1],v[0],0] ] )

def OrthonormalizedOfOrientation(o):
	assert o.shape == (3,3)
	X = o[:,0] / sqrt(dot(o[:,0],o[:,0]))
	Z = cross(X, o[:,1])
	Z = Z / sqrt(dot(Z,Z))
	Y = cross(Z, X)
	Y = Y / sqrt(dot(Y,Y))
	return array([X,Y,Z]).T

def ibits(i,pos,len):
	return (i >> pos) & ~(-1 << len)

def sign(x):
	if x is 0: return -1
	else: return 1

def BoxCorners2(r_CM, v_CM, A, omega, box_size):
	r_corners_local = zeros((3,8))
	for i in range(8):
		for j in range(3):
			r_corners_local[j,i] = box_size[j]/2.0*sign(ibits(i,j,1))
	r_corners = dot(A, r_corners_local)
	cc = [] # Contact Candidates
	for i in range(8):
		r_corners[:,i] += r_CM
		if r_corners[2,i] < 0.005:
			cc.append(i)
	
	v_corners = zeros((3,8))
	for i in range(8):
		v_corners[:,i] = v_CM + cross(omega, r_corners_local[:,i])
	
	return r_corners, v_corners, cc
