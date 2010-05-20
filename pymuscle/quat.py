from numpy import array, linalg
from math import sqrt

def quat_to_mat(q):
	'''
	Convert the rotation quaternion q to a matrix qx
	which makes q x r = qx . r where r is another rotation quaternion and
	'x' and '.' are quaternion multiplication operator and matrix-vector multiplication,
	respectively.
	
	IMPORTANT: scalar component first!
	'''
	a, b, c, d = q
	return array([ [ a, -b, -c, -d ],
	               [ b,  a, -d,  c ],
	               [ c,  d,  a, -b ],
	               [ d, -c,  b,  a ] ])

def quat_mult(q1, q2):
	"""
	Multiplication between two quaternions

	IMPORTANT: scalar component first!
	"""
	a1, b1, c1, d1 = q1
	a2, b2, c2, d2 = q2
	return array([ a1*a2 - b1*b2 - c1*c2 - d1*d2,
	               a1*b2 + b1*a2 + c1*d2 - d1*c2,
	               a1*c2 - b1*d2 + c1*a2 + d1*b2,
	               a1*d2 + b1*c2 - c1*b2 + d1*a2 ])

def quat_normalize(q):
	assert len(q) == 4
	return q / linalg.norm(q)

def quat_rot(q, v):
	'''
	Rotate a vector by a quaternion rotation
	'''
	assert len(q) == 4 and len(v) == 3
	return quat_mult(q, quat_mult([0, v[0], v[1], v[2]], quat_conj(q)))[1:4]

def quat_conj(q):
	a, b, c, d = q
	return ( a, -b, -c, -d )

def QddFromAngAcc_BC(q, qd, angacc_body):
	omega = QuatToAngularVel_BC(q, qd)
	ret = 0.5 * array(quat_mult(qd, [0]+list(omega)) + quat_mult(q, [0]+list(angacc_body)))
	assert len(ret) == 4
	return ret

def QddFromAngAcc_WC(q, qd, angacc_world):
	qdd = 0.5*quat_mult([0, angacc_world[0], angacc_world[1], angacc_world[2]], q) - q
	return qdd

def QuatToAngularVel_BC(q, qd):
	return (2*quat_mult(quat_conj(q), qd))[1:4]

def QuatToAngularVel_WC(q, qd):
	return (2*quat_mult(qd, quat_conj(q)))[1:4]

def test():
	a = 1.5
	b = -2.5
	c = -3.
	d = 4.
	qlen = sqrt(a*a+b*b+c*c+d*d)
	a /= qlen
	b /= qlen
	c /= qlen
	d /= qlen
	v1 = 1.5
	v2 = -2.2
	v3 = -3.5
	print ( a*(-b*v1 - c*v2 - d*v3) + b*(a*v1 + c*v3 - d*v2) + c*(a*v2 + d*v1 - b*v3) + d*(a*v3 + b*v2 - c*v1),
		    a*(a*v1 + c*v3 - d*v2) + c*(a*v3 + b*v2 - c*v1) - b*(-b*v1 - c*v2 - d*v3) - d*(a*v2 + d*v1 - b*v3),
		    a*(a*v2 + d*v1 - b*v3) + d*(a*v1 + c*v3 - d*v2) - b*(a*v3 + b*v2 - c*v1) - c*(-b*v1 - c*v2 - d*v3),
		    a*(a*v3 + b*v2 - c*v1) + b*(a*v2 + d*v1 - b*v3) - c*(a*v1 + c*v3 - d*v2) - d*(-b*v1 - c*v2 - d*v3) )

'''
q = ( Symbol('a'), Symbol('b'), Symbol('c'), Symbol('d') )
v = ( 0, Symbol('v1'), Symbol('v2'), Symbol('v3') )
#q2 = ( Symbol('a2'), Symbol('b2'), Symbol('c2'), Symbol('d2') )

#print quat_mult(quat_mult(q, v), quat_conj(q))

if __name__ == "__main__":
	test()

'''