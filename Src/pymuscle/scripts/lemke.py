#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Lemke's algorithm for solving LCP
"""
import lsqr


from numpy import *
seterr(divide='raise')
def lemke(M, q, z0):
	n = len(q)
	assert M.shape == (n,n)
	
	#z0 = zeros((n)) # z0 is zero-vector for now...

	zer_tol = 1e-5
	piv_tol = 1e-8
	maxiter = min(1000,50*n)
	err = 0

	# Trivial solution exists
	if all([qi>=0 for qi in q]):
		z = zeros((n))
		return z, err

	# Initializations
	# (note: all variables are initialized to their appropriate sizes)
	z = zeros((2*n))
	j = zeros((n))
	iterr = 0
	theta = 0
	ratio = 0
	leaving = 0
	Be = ones((n))
	x = q
	bas = zeros((n))
	nonbas = zeros((n))

	t = 2*n         # Artificial variable
	entering = t    # is the first entering variable

	# Determine initial basis
	if len(z0) == 0:
		bas = array([],dtype=int64)
		nonbas = array(range(n))
	else:
		bas = nonzero(z0>0)[0]
		nonbas = nonzero(z0<=0)[0]

	# allocate memory for B
	B = -identity(n)

	# Determine initial values
	if len(bas) != 0:
		B = concatenate((M[:,bas], B[:,nonbas]), axis=1)
		'''
		condest = sum(abs(linalg.inv(B)))
		print condest
		if condest > 1e16:
			z=[]
			err=3
			return z, err
		'''

		x = -linalg.solve(B, q)
		#x = -linalg.lstsq(B, q)
		#x = x[0]

	# Check if initial basis provides solution
	if all([xi>=0 for xi in x]):
		z[ bas ] = x[ range(len(bas)) ]
		z = z[ range(n) ]
		return z, err

	# Determine initial leaving variable
	tval = max(-x)
	lvindex = list(x).index(-max(-x))
	bas = concatenate((bas,array([n+nonbasi for nonbasi in nonbas])))
	leaving = bas[lvindex]

	bas[lvindex] = t # pivot in the artificial variable

	U=array([(xi<0)*1.0 for xi in x])
	Be=-dot(B, U)
	x = x + tval*U
	x[lvindex] = tval
	B[:, lvindex] = Be

	# Main iterations begin here
	for iterr in range(maxiter):
		# Check if done; if not, get new entering variable
		assert type(leaving) in [int64, int]
		assert type(t) in [int64, int]
		assert type(entering) in [int64, int]
		'''
		print 'ITERATION', iterr
		print 'Python leaving', leaving
		print 'bas vector', bas
		'''
		if leaving == t:
			break
		elif leaving < n:
			entering = n + leaving
			Be = zeros((n))
			Be[leaving] = -1
		else:
			entering = leaving - n
			Be = M[:, entering]
		d = linalg.solve(B, Be)
		#d = linalg.lstsq(B, Be)
		#d = d[0] # we only need the solution

		# Find new leaving variable
		j = nonzero(d>piv_tol)[0] # indices of d>0
		if len(j) == 0: # no new pivots - ray termination    err=2;
			err=2
			break
		theta = min([ (xj + zer_tol)/dj for xj, dj in zip(x[j], d[j]) ])
		j = j[ list(nonzero( [ xj/dj for xj, dj in zip(x[j], d[j]) ] <= theta )[0]) ]
		assert len(j) > 0
		lvindex = nonzero(bas[j] == t)[0]
		assert len(lvindex) in [0,1]
		if len(lvindex) != 0:
			lvindex = j[lvindex]
		else:
			assert len(j) > 0
			theta = max(d[j])
			lvindex = list(d).index(max(d[j]))
			lvindex = list(nonzero(d[j] == theta)[0])

			lvindex = int(ceil((len(lvindex)-1)*0.5))
			#lvindex = int(ceil((len(lvindex)-1)*random.random()))
			#lvindex = 0

			lvindex = j[lvindex]
			aaaaaaaaaaaaaaaaaa=99
		leaving = bas[lvindex]
		if type(leaving) not in [int64, int]:
			assert len(leaving) == 1
			leaving = leaving[0]

		if type(lvindex) is int64:
			pass
		elif len(lvindex) == 1:
			lvindex = lvindex[0]
		else:
			raise Exception, 'type error!'
		# Perform pivot
		ratio = x[lvindex] / d[lvindex]
		x = x - ratio*d
		x[lvindex] = ratio
		B[:,lvindex] = Be
		bas[lvindex] = entering
		aaaaaaaaaaaaa=100
		'''
		print 'Python ratio', ratio
		'''
		# end of iterations

	if iterr >= maxiter and leaving[0] != t:
		err = 1

	if max(bas) >= len(z):
		z = concatenate((z,zeros((max(bas)-len(z)+1))))
	z[bas] = x
	z = z[range(n)]

	# Display warning messages if no error code is returned
	if err != 0:
		s = 'Warning: solution not found - '
		if err == 1:
			print s, 'Iterations exceeded limit'
		elif err == 2:
			print s, 'Unbounded ray'
		elif err == 3:
			print s, 'Initial basis infeasible'

	return z, err