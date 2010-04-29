#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Common math routines
"""
from math import cos, sin
from numpy import array, dot, linalg
#
# z-x-z (moving frame set)
#
def RotationMatrixFromEulerAngles_zxz(phi, theta, psix):
	c1,s1 = cos(phi), sin(phi)
	c2,s2 = cos(theta), sin(theta)
	c3,s3 = cos(psix), sin(psix)
	return array([ [ c1*c3 - c2*s1*s3,  -c3*s1 - c1*c2*s3,   s2*s3],
	               [ c2*c3*s1 + c1*s3,   c1*c2*c3 - s1*s3,  -c3*s2],
	               [ s1*s2,              c1*s2,              c2   ] ])
def AngularVelocityFromEulerAngles_zxz(phi, theta, psix, phid, thetad, psixd):
	X = array([ [sin(theta)*sin(phi),  cos(phi),   0],
	            [sin(theta)*cos(phi), -sin(phi),   0],
	            [      cos(theta)   ,      0   ,   1] ])
	"""
	return array([ psixd*sin(theta)*sin(phi)+thetad*cos(phi),
	               psixd*sin(theta)*cos(phi)-thetad*sin(phi),
	               psixd*cos(theta)+phid ])
	"""
	return dot(X, array([phid, thetad, psixd]))
def EulerAngleRateFromAngularVelocity_zxz(phi, theta, psix, omega):
	X = array([ [sin(theta)*sin(phi),  cos(phi),   0],
	            [sin(theta)*cos(phi), -sin(phi),   0],
	            [      cos(theta)   ,      0   ,   1] ])
	return dot(linalg.inv(X), omega)
#
# x-y-z (fixed angle set)
#
def RotationMatrixFromEulerAngles_xyz(phi, theta, psix):
	c1,s1 = cos(phi), sin(phi)
	c2,s2 = cos(theta), sin(theta)
	c3,s3 = cos(psix), sin(psix)
	return array([ [ c2*c3, c3*s1*s2-c1*s3, c1*c3*s2+s1*s3],
	               [ c2*s3, c1*c3+s1*s2*s3, c1*s2*s3-c3*s1],        
	               [ -s2,   c2*s1,          c1*c2         ] ])
