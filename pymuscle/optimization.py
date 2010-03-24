#!/usr/bin/python
# -*- coding: utf-8 -*-

from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys
import copy
import mosek
from math import cos, sin, atan
from ArcBall import * 				# ArcBallT and this tutorials set of points/vectors/matrix types
from numpy import *
PI2 = 2.0*3.1415926535			# 2 * PI (not squared!) 		// PI Squared


# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0

# *********************** Globals *********************** 
# Python 2.2 defines these directly
try:
	True
except NameError:
	True = 1==1
	False = 1==0

g_Transform = Matrix4fT ()
g_LastRot = Matrix3fT ()
g_ThisRot = Matrix3fT ()

g_ArcBall = ArcBallT (640, 480)
g_isDragging = False
g_quadratic = None

def BoxInertia(box_size, mass):
	return diag([
		mass*(box_size[1]**2+box_size[2]**2)/12.0,
		mass*(box_size[0]**2+box_size[2]**2)/12.0,
		mass*(box_size[0]**2+box_size[1]**2)/12.0  ])


# friction coefficients and friction cone basis
def BuildFrictionConeBasis(mu):
	theta = 1.0/atan(mu)
	V = array([
		[  1,  1, sqrt(2)/mu ],
		[  1, -1, sqrt(2)/mu ],
		[ -1,  1, sqrt(2)/mu ],
		[ -1, -1, sqrt(2)/mu ] ])
	V = V / sqrt(dot(V[0],V[0]))
	V = V.T
	"""
	print 'Friction cone basis'
	print V
	"""
	return V

def ibits(i,pos,len):
	return (i >> pos) & ~(-1 << len)
def sign(x):
	if x is 0: return -1
	else: return 1

#
# Calcualte box corner position and velocity for given state
#
def BoxCorners(pos, posd, ang, angd, box_size):
	# Box corner position in local coordinates
	corners = zeros((4,8))
	for i in range(8):
		for j in range(3):
			corners[j,i] = box_size[j]/2.0*sign(ibits(i,j,1))
		corners[3,i] = 1
	#print corners

	matRotX = array(
		[[                1,                 0,                 0, 0],
		 [                 0,  math.cos(ang[0]), -math.sin(ang[0]), 0],
		 [                 0,  math.sin(ang[0]),  math.cos(ang[0]), 0],
		 [                 0,                 0,                 0, 1]])
	matRotY = array(
		[[ math.cos(ang[1]),                 0,  math.sin(ang[1]), 0],
		 [                 0,                 1,                 0, 0],
		 [ -math.sin(ang[1]),                 0,  math.cos(ang[1]), 0],
		 [                 0,                 0,                 0, 1]])
	matRotZ = array(
		[[ math.cos(ang[2]), -math.sin(ang[2]), 0, 0],
		 [  math.sin(ang[2]),  math.cos(ang[2]), 0, 0],
		 [                 0,                 0, 1, 0],
		 [                 0,                 0, 0, 1]])
	matTrans = array(
		[[ 1, 0, 0, pos[0] ],
		 [  0, 1, 0, pos[1] ],
		 [  0, 0, 1, pos[2] ],
		 [  0, 0, 0,      1 ]])

	# Box corner position
	cornersX = dot(matTrans,dot(matRotZ,dot(matRotY,dot(matRotX,corners))))
	#print cornersX

	# Box corner velocity
	cornersXd = zeros((4,8))
	for i in range(8):
		#print corners[0:3,i:i+1]
		cornersXd[0:3,i:i+1] = (posd + cross(angd,corners[0:3,i:i+1].T)).T
	#print cornersXd

	return cornersX, cornersXd

def diag_sub(S, n):
	assert n >= 1
	"""
	Builds a matrix whose diagonal elements are the same sub-block matrices S e.g.
	for n=4
	SS = [[ S, 0, 0, 0 ],
	      [ 0, S, 0, 0 ],
	      [ 0, 0, S, 0 ],
	      [ 0, 0, 0, S ]]
	S can be any size of matrix
	"""
	SS = S
	n = n - 1
	while n > 0:
		SS = vstack([hstack([SS,zeros((SS.shape[0],S.shape[1]))]), hstack([zeros((S.shape[0],SS.shape[1])),S])])
		n = n - 1
	return SS

def diag_sub_list(Sl):
	assert len(Sl) >= 1
	SS = Sl
	for S in Sl:
		SS = vstack([hstack([SS,zeros((SS.shape[0],S.shape[1]))]), hstack([zeros((S.shape[0],SS.shape[1])),S])])
	return SS

def cross_op_mat(v):
	assert len(v) is 3
	return array( [ [0,-v[2],v[1]], [v[2],0,-v[0]], [-v[1],v[0],0] ] )

def expand_vertical(m):
	mm = zeros((m.shape[0]*m.shape[1],m.shape[1]))
	for i in range(m.shape[1]):
		mm[i*m.shape[0]:(i+1)*m.shape[0],i] = m[:,i]
	return mm

def DetermineContactPoints(cornersX, cornersXd):
	"""
	접촉점 리스트 중 static, dynamic 접촉점을 구별해서 인덱스를 리턴한다.
	"""
	static_contacts = []
	dynamic_contacts = []
	for i in range(8): # 8 is the maximum number of contact points of a rigid box
		z = cornersX[2,i]
		# criterion for contact point registration
		"""
		if z < 0:
			print 'WARN: contact point already penetrated !!! (z=%f)' % z
		"""
		if z < 0.01:
			# tangential velocity of the contact point
			if sqrt(dot(cornersXd[0:2,i],cornersXd[0:2,i])) < 0.01:
				static_contacts.append(i)
			else:
				dynamic_contacts.append(i)

	return static_contacts, dynamic_contacts

def BuildP1Matrix(V, n_cs):
	"""
	lambda_cs에 곱해져서 static contact force의 합력 f_cs를 구해주는
	행렬 P1을 구합니다.
	
	f_cs = P1 * lambda_cs
	
	f_cs      : 3 x 1
	P1        : 3 x n_cs*4
	lambda_cs : n_cs*4 x 1
	"""
	P1 = 0
	if n_cs is not 0:
		P1 = dot(V, concatenate((identity(4),)*n_cs,axis=1))
		assert P1.shape == (3, n_cs*4)
	return P1

def BuildP2Matrix(mu, n_cd, cornersXd, dynamic_contacts):
	"""
	lambda_cd에 곱해져서 dynamic contact force의 합력 f_cd를 구해주는
	행렬 P2을 구합니다.
	
	f_cd = P2 * lambda_cd
	
	f_cd      : 3 x 1
	P2        : 3 x n_cd
	lambda_cd : n_cd x 1
	"""
	P2 = 0
	if n_cd is not 0:
		P2i = []
		for i in dynamic_contacts:
			# extract tangential velocity from cornersXd
			a=array([0,0,1]) - mu*array([cornersXd[0,i], cornersXd[1,i], 0])
			P2i.append( array( [ [a[0]], [a[1]], [a[2]] ] ) )
		P2 = concatenate(P2i, axis=1)
		assert P2.shape == (3, n_cd)
	return P2

def LinearAccelerationComMatrix(mass, cornersXd, static_contacts, dynamic_contacts, V, mu):
	"""
	Center of mass의 선 가속도를 최적화 변수 x_opt에 대한 1차 식으로 나타내는
	행렬 A_a_com, 벡터 b_a_com을 계산한다.
	
	p_comdd = A_a_com*x_opt + b_a_com
	"""
	n_cs = len(static_contacts)
	n_cd = len(dynamic_contacts)
	n_u = 0

	P1 = BuildP1Matrix(V, n_cs)
	P2 = BuildP2Matrix(mu, n_cd, cornersXd, dynamic_contacts)
	
	S_lambda = BuildSelectionMatrix(n_cs, n_cd, n_u, 'lambda')

	A_a_com = 0
	if P1 is not 0 and P2 is not 0:
		A_a_com = concatenate( (P1,P2), axis=1 )
	elif P1 is not 0:
		A_a_com = P1
	elif P2 is not 0:
		A_a_com = P2

	if A_a_com is not 0:
		A_a_com = dot(A_a_com, S_lambda)
		A_a_com /= mass

	b_a_com = array([ [0], [0], [-9.81/mass] ])

	return A_a_com, b_a_com

def NextPointPositionMatrix(A_a_q, b_a_q, p_q, pd_q, dt):
	"""
	다음 스텝에서의 어떤 점 q의 위치를 x_opt에 대한 1차 식으로
	나타내는 행렬 A_q_next와 b_q_next를 구한다.
	
	p_q_next = A_q_next * x_opt + b_q_next
	"""
	A_q_next = 0
	if A_a_q is not 0:
		A_q_next = (dt/2.0)**2*A_a_q
	b_q_next = ((dt/2.0)**2*b_a_q).flatten() + p_q + [pd_q[i]*dt for i in range(3)]
	return A_q_next, b_q_next

def AngularAccelerationComMatrix(cornersX, cornersXd, static_contacts, dynamic_contacts, V, pos, mu, H_com):
	"""
	Center of mass에서의 각 가속도를 최적화 변수 x_opt에 대한 1차 식으로 나타내는
	행렬 A_alpha_com을 계산한다.
	
	alpha_com = A_a_com*x_opt
	"""
	n_cs = len(static_contacts)
	n_cd = len(dynamic_contacts)
	n_u = 0 # TODO: muscle
	
	VAI_list = []
	V_2 = 0
	if n_cs is not 0:
		V_2 = diag_sub(V, n_cs)
		VAI_list.append(V_2)
	r_csx = 0
	for i in static_contacts:
		r_cs = cornersX[0:3,i] - pos
		r_cs_i = cross_op_mat(r_cs)
		if r_csx is 0:
			r_csx = r_cs_i
		else:
			r_csx = concatenate((r_csx,r_cs_i),axis=1)

	r_cdx = 0
	for i in dynamic_contacts:
		r_cd = cornersX[0:3,i] - pos
		r_cd_i = cross_op_mat(r_cd)
		if r_cdx is 0:
			r_cdx = r_cd_i
		else:
			r_cdx = concatenate((r_cdx,r_cd_i),axis=1)

	r_ux = 0 # TODO: user force (muscle force) vector
	A_fcd = 0
	if n_cd is not 0:
		A_fcd = expand_vertical(BuildP2Matrix(mu, n_cd, cornersXd, dynamic_contacts))
		VAI_list.append(A_fcd)

	R = 0
	if r_csx is not 0 and r_cdx is not 0:
		R = concatenate((r_csx,r_cdx), axis=1)
	elif r_csx is not 0:
		R = r_csx
	elif r_cdx is not 0:
		R = r_cdx

	VAI = 0
	if V_2 is not 0 and A_fcd is not 0:
		VAI = diag_sub_list((V_2,A_fcd))
	elif V_2 is not 0:
		VAI = V_2
	elif A_fcd is not 0:
		VAI = A_fcd

	A_alpha = 0
	if R is not 0 and VAI is not 0:
		A_alpha = dot( dot( linalg.inv(H_com), R ), VAI )
		S_lambda = BuildSelectionMatrix(n_cs, n_cd, n_u, 'lambda')
		A_alpha = dot(A_alpha, S_lambda)

	return A_alpha, VAI

def BuildSelectionMatrix(n_cs, n_cd, n_u, sel):
	"""
	최적화 벡터 변수 (x_opt)는 여러 변수가 모여 있다.
	여기서 필요한 변수만 뽑아서 새로운 벡터를 만들어주는
	선택 행렬(selection matrix)가 필요한 경우가 있는데
	이 함수는 그것을 만들어 준다.
	
	최적화 벡터는 아래와 같이 저장되어 있다고 가정한다.
	
	 - lambda_cs_ab: a번째 static friction force의 b번째 basis coefficient
	 - lambda_cd_a: a번째 dynamic friction force의 coefficient
	 - u_a: a번째 muscle의 actuating force
	
	x_opt = [ epsilon,
	          lambda_cs_11,
			  lambda_cs_12,
			  lambda_cs_13,
			  lambda_cs_14,
			  ...,
			  lambda_cs_n1,
			  lambda_cs_n2,
			  lambda_cs_n3,
			  lambda_cs_n4,
			  lambda_cd_1,
			  ...,
			  lambda_cd_n,
			  u_1, 
			  ...
			  u_n ]^T
			  
	즉, x_opt는 길이가 1+(n_cs*4)+n_cd+n_u인 벡터이다.
	"""
	S = 0
	if sel == 'lambda':
		S = concatenate( (
		    zeros((4*n_cs+n_cd+n_u, 1)),
		    identity(4*n_cs+n_cd),
		    zeros((4*n_cs+n_cd+n_u, n_u))
		    ), axis=1 )
	elif sel == 'lambda_fu':
		S = concatenate( (
		    zeros((4*n_cs+n_cd+n_u, 1)),
		    identity(4*n_cs+n_cd+n_u)
		    ), axis=1 )
	
	return S

def LinearAccelerationMatrix(A_a_com, b_a_com, r_pq, A_alpha, omega):
	"""
	임의의 점 q에 대해 x_opt에 대한 1차 식으로 표현되는 가속도를 구하는
	행렬 A_a_q, b_a_q를 계산한다.

	qdd = A_a_q*x_opt + b_a_q
	
	현재 각 속도 omega,
	LinearAccelerationComMatrix() 함수로 구해지는 A_a_com, b_a_com,
	AngularAccelerationComMatrix() 함수로 구해지는 A_alpha가 인자로 필요하다.
	
	벡터 r_pq의 p는 COM을 가리키고 q는 우리가 다음 위치를 알길 원하는 점이다.
	만일 r_pq를 0으로 설정하면 COM의 다음 위치가 계산되는 1차식의 A_next와 b_next가
	구해진다. r_pq가 0인 경우 A_alpha와 omega의 값은 무시된다.
	"""
	A_a_q = A_a_com - dot(cross_op_mat(r_pq), A_alpha)
	b_a_q = b_a_com.flatten() + dot(dot(cross_op_mat(omega), cross_op_mat(omega)), r_pq).flatten()

	return A_a_q, b_a_q

def BuildMatrices(mu, cornersX, cornersXd, dt, H_com, mass, pos, V):
	"""
	최적화 변수(컨트롤 변수)와 독립적인 값으로 구성할 수 있는
	모든 행렬 및 벡터를 계산한다.

	pdd = A_a*x_opt + b_a
	
	pdd는 강체의 점 p에서의 가속도
	x_opt는 최적화 변수 벡터이다.
	이 함수는 행렬 A_a, 벡터 b_a를 계산한다.
	
	
	
	"""
	static_contacts = []
	dynamic_contacts = []
	for i in range(8): # 8 is the maximum number of contact points of a rigid box
		z = cornersX[2,i]
		# criterion for contact point registration
		if z < 0:
			print 'WARN: contact point already penetrated !!! (z=%d)' % z
			
		if z < 0.01:
			
			
			# tangential velocity of the contact point
			if sqrt(dot(cornersXd[0:2,i],cornersXd[0:2,i])) < 0.01:
				static_contacts.append(i)
			else:
				dynamic_contacts.append(i)
			

	n_cs = len(static_contacts)
	n_cd = len(dynamic_contacts)
	n_u = 0
	
	"""
	print 'Static contacts: %d' % n_cs
	print 'Dynamic contacts: %d' % n_cd
	"""

	P1 = 0
	if n_cs is not 0:
		P1 = dot(V, concatenate((identity(4),)*n_cs,axis=1))

	P2 = 0
	if n_cd is not 0:
		P2i = []
		for i in dynamic_contacts:
			# extract tangential velocity from cornersXd
			a=array([0,0,1]) - mu*array([cornersXd[0,i], cornersXd[1,i], 0])
			P2i.append( array( [ [a[0]], [a[1]], [a[2]] ] ) )
		"""
		print 'P2i'
		print P2i
		"""
		P2 = concatenate(P2i, axis=1)
	"""
	print 'Intermediate test 1'
	print P1
	print 'Intermediate test 2'
	print P2
	"""
	
	S_lambda = BuildSelectionMatrix(n_cs, n_cd, n_u, 'lambda')
	#print Slambda
	A_a = 0
	if P1 is not 0 and P2 is not 0:
		A_a = concatenate( (P1,P2), axis=1 )
	elif P1 is not 0:
		A_a = P1
	elif P2 is not 0:
		A_a = P2

	if A_a is not 0:
		A_a = dot(A_a, S_lambda)
		A_a = A_a / mass

	#print 'A_a'
	#print A_a

	b_a = array([ [0], [0], [-9.81/mass] ])
	#print 'b_a'
	#print b_a

	A_com = 0
	if A_a is not 0:
		A_com = dt*dt/2.0 * dot(identity(3) , A_a)

	b_cs = []
	for i in static_contacts:
		b_cs.append(dt*dt/2.0*dot(identity(3),b_a) + reshape(cornersX[0:3,i]+cornersXd[0:3,i]*dt,(3,1)))
	b_cd = []
	for i in dynamic_contacts:
		b_cd.append(dt*dt/2.0*dot(identity(3),b_a) + reshape(cornersX[0:3,i]+cornersXd[0:3,i]*dt,(3,1)))

	"""
	print 'b_cs'
	print b_cs
	print 'b_cd'
	print b_cd
	"""

	VAI_list = []
	V_2 = 0
	if n_cs is not 0:
		V_2 = diag_sub(V, n_cs)
		VAI_list.append(V_2)
	r_csx = 0
	for i in static_contacts:
		r_cs = cornersX[0:3,i] - pos
		r_cs_i = cross_op_mat(r_cs)
		if r_csx is 0:
			r_csx = r_cs_i
		else:
			r_csx = concatenate((r_csx,r_cs_i),axis=1)

	r_cdx = 0
	for i in dynamic_contacts:
		r_cd = cornersX[0:3,i] - pos
		r_cd_i = cross_op_mat(r_cd)
		if r_cdx is 0:
			r_cdx = r_cd_i
		else:
			r_cdx = concatenate((r_cdx,r_cd_i),axis=1)





	r_ux = 0
	A_fcd = 0
	if P2 is not 0:
		A_fcd = expand_vertical(P2)
		VAI_list.append(A_fcd)

	R = 0
	if r_csx is not 0 and r_cdx is not 0:
		R = concatenate((r_csx,r_cdx), axis=1)
	elif r_csx is not 0:
		R = r_csx
	elif r_cdx is not 0:
		R = r_cdx

	VAI = 0
	if V_2 is not 0 and A_fcd is not 0:
		VAI = diag_sub_list((V_2,A_fcd))
	elif V_2 is not 0:
		VAI = V_2
	elif A_fcd is not 0:
		VAI = A_fcd

	A_alpha_p = 0
	A_alpha = 0
	if R is not 0 and VAI is not 0:
		A_alpha_p = dot( dot( linalg.inv(H_com), R ), VAI )
		A_alpha = concatenate( ( zeros((A_alpha_p.shape[0],1)), A_alpha_p ), axis=1 )

	"""
	print 'r_csx'
	print r_csx
	print 'r_cdx'
	print r_cdx
	print 'A_fcd'
	print A_fcd
	print 'A_alpha'
	print A_alpha
	"""

	return A_a, b_a, A_com, b_cs, b_cd, A_alpha, VAI, static_contacts, dynamic_contacts

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
	sys.stdout.write(text)
	sys.stdout.flush()

# We might write everything directly as a script, but it looks nicer
# to create a function.
def SolveSocp(env, friction_constraints, n_cs, n_cd, n_u):
	# total number of optimization variable is n_x
	n_x = 1 + (4*n_cs) + n_cd + n_u
	assert n_cs+n_cd is not 0
	
	# Attach a printer to the environment
	#env.set_Stream (mosek.streamtype.log, streamprinter)

	# Create a task
	task = env.Task(0,0)

	# Attach a printer to the task
	#task.set_Stream (mosek.streamtype.log, streamprinter)

	for fc in friction_constraints:
		assert fc[0].shape == (3, n_x)
		assert len(fc[1]) is 3
	bkc = [ mosek.boundkey.lo ] * len(friction_constraints)
	blc = [-friction_constraints[i][1][2] for i in range(len(friction_constraints))]
	buc = [ +inf ] * len(friction_constraints)
	
	c   = [1.0] + [0.0]*(n_x-1)
	bkx = [ mosek.boundkey.fr ] + [mosek.boundkey.lo]*(n_x-1)
	blx = [       -inf        ] + [0.0]*(n_x-1)
	bux = [       +inf        ] + [inf]*(n_x-1)


	A = array([ fc[0][2,:] for fc in friction_constraints ])
		
	assert A.shape[1] == n_x
	asub = [ array(range(A.shape[0])) ] * n_x
	aval = [ array(A[:,j]) for j in range(A.shape[1]) ]

	NUMVAR = len(bkx)
	NUMCON = len(bkc)
	#NUMANZ = 4
	# Give MOSEK an estimate of the size of the input data. 
	#  This is done to increase the speed of inputting data. 
	#  However, it is optional. 
	task.putmaxnumvar(NUMVAR)
	task.putmaxnumcon(NUMCON)
	#task.putmaxnumanz(NUMANZ)

	# Append 'NUMCON' empty constraints.
	# The constraints will initially have no bounds. 
	task.append(mosek.accmode.con,NUMCON)

	#Append 'NUMVAR' variables.
	# The variables will initially be fixed at zero (x=0). 
	task.append(mosek.accmode.var,NUMVAR)

	#Optionally add a constant term to the objective. 
	task.putcfix(0.0)

	for j in range(NUMVAR):
		# Set the linear term c_j in the objective.
		task.putcj(j,c[j])
		# Set the bounds on variable j
		# blx[j] <= x_j <= bux[j] 
		task.putbound(mosek.accmode.var,j,bkx[j],blx[j],bux[j])

	for j in range(n_x):
		# Input column j of A 
		task.putavec(mosek.accmode.var,  # Input columns of A.
				     j,                  # Variable (column) index.
				     list(asub[j]),            # Row index of non-zeros in column j.
				     list(aval[j]))            # Non-zero Values of column j. 

	for i in range(NUMCON):
		task.putbound(mosek.accmode.con,i,bkc[i],blc[i],buc[i])

	# Input the cones
	task.appendcone(mosek.conetype.quad, 0.0, [ 0 ] + range(1,n_x))

	# Input the objective sense (minimize/maximize)
	task.putobjsense(mosek.objsense.minimize)

	# Optimize the task
	task.optimize()
	# Print a summary containing information
	# about the solution for debugging purposes
	task.solutionsummary(mosek.streamtype.msg)
	prosta = []
	solsta = []
	[prosta,solsta] = task.getsolutionstatus(mosek.soltype.itr)

	# Output a solution
	xx = zeros(NUMVAR, float)
	task.getsolutionslice(mosek.soltype.itr,
		                  mosek.solitem.xx, 
		                  0,NUMVAR,          
		                  xx)

	if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
		1==1
		#print("Optimal solution: %s" % xx)
	elif solsta == mosek.solsta.dual_infeas_cer: 
		print("Primal or dual infeasibility.\n")
	elif solsta == mosek.solsta.prim_infeas_cer:
		print("Primal or dual infeasibility.\n")
	elif solsta == mosek.solsta.near_dual_infeas_cer:
		print("Primal or dual infeasibility.\n")
	elif  solsta == mosek.solsta.near_prim_infeas_cer:
		print("Primal or dual infeasibility.\n")
	elif mosek.solsta.unknown:
		print("Unknown solution status")
	else:
		print("Other solution status")

	return xx

# A general OpenGL initialization function.  Sets all of the initial parameters. 
def Initialize (Width, Height):				# We call this right after our OpenGL window is created.
	global g_quadratic

	glClearColor(0.0, 0.0, 0.0, 1.0)					# This Will Clear The Background Color To Black
	glClearDepth(1.0)									# Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL)								# The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST)								# Enables Depth Testing
	glShadeModel (GL_FLAT);								# Select Flat Shading (Nice Definition Of Objects)
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) 	# Really Nice Perspective Calculations

	g_quadratic = gluNewQuadric();
	gluQuadricNormals(g_quadratic, GLU_SMOOTH);
	gluQuadricDrawStyle(g_quadratic, GLU_FILL); 
	# Why? this tutorial never maps any textures?! ? 
	# gluQuadricTexture(g_quadratic, GL_TRUE);			# // Create Texture Coords

	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)

	glEnable (GL_COLOR_MATERIAL)

	return True




def Upon_Drag (cursor_x, cursor_y):
	""" Mouse cursor is moving
		Glut calls this function (when mouse button is down)
		and pases the mouse cursor postion in window coords as the mouse moves.
	"""
	global g_isDragging, g_LastRot, g_Transform, g_ThisRot

	if (g_isDragging):
		mouse_pt = Point2fT (cursor_x, cursor_y)
		ThisQuat = g_ArcBall.drag (mouse_pt)						# // Update End Vector And Get Rotation As Quaternion
		g_ThisRot = Matrix3fSetRotationFromQuat4f (ThisQuat)		# // Convert Quaternion Into Matrix3fT
		# Use correct Linear Algebra matrix multiplication C = A * B
		g_ThisRot = Matrix3fMulMatrix3f (g_LastRot, g_ThisRot)		# // Accumulate Last Rotation Into This One
		g_Transform = Matrix4fSetRotationFromMatrix3f (g_Transform, g_ThisRot)	# // Set Our Final Transform's Rotation From This One
	return

def Upon_Click (button, button_state, cursor_x, cursor_y):
	""" Mouse button clicked.
		Glut calls this function when a mouse button is
		clicked or released.
	"""
	global g_isDragging, g_LastRot, g_Transform, g_ThisRot

	g_isDragging = False
	if (button == GLUT_RIGHT_BUTTON and button_state == GLUT_UP):
		# Right button click
		g_LastRot = Matrix3fSetIdentity ();							# // Reset Rotation
		g_ThisRot = Matrix3fSetIdentity ();							# // Reset Rotation
		g_Transform = Matrix4fSetRotationFromMatrix3f (g_Transform, g_ThisRot);	# // Reset Rotation
	elif (button == GLUT_LEFT_BUTTON and button_state == GLUT_UP):
		# Left button released
		g_LastRot = g_ThisRot.copy();							# // Set Last Static Rotation To Last Dynamic One
	elif (button == GLUT_LEFT_BUTTON and button_state == GLUT_DOWN):
		# Left button clicked down
		g_LastRot = g_ThisRot.copy();							# // Set Last Static Rotation To Last Dynamic One
		g_isDragging = True											# // Prepare For Dragging
		mouse_pt = Point2fT (cursor_x, cursor_y)
		g_ArcBall.click (mouse_pt);								# // Update Start Vector And Prepare For Dragging

	return



def Torus(MinorRadius, MajorRadius):		
	# // Draw A Torus With Normals
	glBegin( GL_TRIANGLE_STRIP );									# // Start A Triangle Strip
	for i in xrange (20): 											# // Stacks
		for j in xrange (-1, 20): 										# // Slices
			# NOTE, python's definition of modulus for negative numbers returns
			# results different than C's
			#       (a / d)*d  +  a % d = a
			if (j < 0):
				wrapFrac = (-j%20)/20.0
				wrapFrac *= -1.0
			else:
				wrapFrac = (j%20)/20.0;
			phi = PI2*wrapFrac;
			sinphi = sin(phi);
			cosphi = cos(phi);

			r = MajorRadius + MinorRadius*cosphi;

			glNormal3f (sin(PI2*(i%20+wrapFrac)/20.0)*cosphi, sinphi, cos(PI2*(i%20+wrapFrac)/20.0)*cosphi);
			glVertex3f (sin(PI2*(i%20+wrapFrac)/20.0)*r, MinorRadius*sinphi, cos(PI2*(i%20+wrapFrac)/20.0)*r);

			glNormal3f (sin(PI2*(i+1%20+wrapFrac)/20.0)*cosphi, sinphi, cos(PI2*(i+1%20+wrapFrac)/20.0)*cosphi);
			glVertex3f (sin(PI2*(i+1%20+wrapFrac)/20.0)*r, MinorRadius*sinphi, cos(PI2*(i+1%20+wrapFrac)/20.0)*r);
	glEnd();														# // Done Torus
	return


# // build the display list.
def BuildList():
	cube = glGenLists(1);              # // generate storage for 2 lists, and return a pointer to the first.
	glNewList(cube, GL_COMPILE);       # // store this list at location cube, and compile it once.

	# // cube without the top;
	glBegin(GL_QUADS);			# // Bottom Face

	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad

	# // Front Face
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f( 1.0,  1.0,  1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-1.0,  1.0,  1.0);	# // Top Left Of The Texture and Quad

	# // Back Face
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0,  1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0,  1.0, -1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Bottom Left Of The Texture and Quad

	# // Right face
	glTexCoord2f(1.0, 0.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f( 1.0,  1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0,  1.0,  1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad

	# // Left Face
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0,  1.0,  1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-1.0,  1.0, -1.0);	# // Top Left Of The Texture and Quad

	glEnd();
	glEndList();

	print "List built."
	return cube


