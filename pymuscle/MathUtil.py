#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Common math routines
"""
from math import cos, sin, atan2, asin, sqrt
from numpy import array, dot, linalg

def RotationMatrixFromQuaternion(a,b,c,d):
	# 'a' is a scalar part
	return array([[a**2+b**2-c**2-d**2,  2*b*c-2*a*d,  2*b*d+2*a*c],
	              [2*b*c+2*a*d,  a**2-b**2+c**2-d**2,  2*c*d-2*a*b],
	              [2*b*d-2*a*c,  2*c*d+2*a*b,  a**2-b**2-c**2+d**2]])

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


def EulerAnglesFromQuaternion(q):
	#
	# Quaternion to euler angles (?-?-? convention)
	# IMPORTANT: scalar component first
	#
	q0,q1,q2,q3 = q
	phi_   = atan2(2*(q0*q1+q2*q3), 1.-2*(q1**2+q2**2))
	# asin(x) need x in range of [-1,1]
	clamped = min(1., 2*(q0*q2-q3*q1))
	clamped = max(-1., clamped)
	theta_ = asin(clamped)
	psi_   = atan2(2*(q0*q3+q1*q2), 1.-2*(q2**2+q3**2))
	return (phi_, theta_, psi_)


if __name__ == '__main__':
	q = array([sqrt(2.)/2, 0, sqrt(2.)/2, 0])
	q /= linalg.norm(q)
	print EulerAnglesFromQuaternion(q)
	