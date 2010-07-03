#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Common math routines
"""
from math import cos, sin, atan2, asin, sqrt, tan, acos
from numpy import array, dot, linalg, cross
from quat import QuatToAngularVel_WC

def cot(x): return 1./tan(x)

def RotationMatrixFromQuaternion(a,b,c,d):
	# 'a' is a scalar part
	return array([ [a**2+b**2-c**2-d**2,  2*b*c-2*a*d,          2*b*d+2*a*c        ],
	               [2*b*c+2*a*d,          a**2-b**2+c**2-d**2,  2*c*d-2*a*b        ],
	               [2*b*d-2*a*c,          2*c*d+2*a*b,          a**2-b**2-c**2+d**2] ])

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



# NOT USED
def ang_vel(q, v):
	x, y, z, phi, theta, psi = q
	xd, yd, zd, phid, thetad, psid = v
	return array([phid*sin(theta)*sin(psi)+thetad*cos(psi),
		          phid*sin(theta)*cos(psi)-thetad*sin(psi),
		          phid*cos(theta)+psid])

def QuatToV(q):
	'''
	Convert orientation quaternion to Exp rotation
	'''
	assert len(q) == 4
	qw, qx, qy, qz = q
	qv = array([qx,qy,qz])
	qv_mag = linalg.norm(qv)
	if qv_mag < THETA:
		return 1/(0.5-qv_mag**2/48)*qv
	else:
		#print 'qw=',qw
		return 2*acos(qw)/qv_mag*qv
def VtoQuat(v):
	th = linalg.norm(v)
	if th < 0.001:
		a = 0.5-th**2/48
	else:
		a = sin(th/2)/th
	return array([cos(th/2), a*v[0], a*v[1], a*v[2]])
def QuatdToVd(q, qd, v):
	'''
	Convert quaternion time derivative to Exp rotation time derivative
	'''
	omega_wc = QuatToAngularVel_WC(q, qd)
	th = linalg.norm(v)
	p = cross(omega_wc, v)
	if th < 0.01:
		gamma = (12-th**2)/6
		eta = dot(omega_wc, v)*(60+th**2)/360
	else:
		gamma = th/tan(0.5*th)
		eta = dot(omega_wc, v)/th*(1/tan(0.5*th)-2/th)
	return 0.5*(gamma*omega_wc + p - eta*v)
	
if __name__ == '__main__':
	q = array([sqrt(2.)/2, 0, sqrt(2.)/2, 0])
	q /= linalg.norm(q)
	print EulerAnglesFromQuaternion(q)
	
