#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid box LCP-based simulator
(rotation parameterization with the exponential map)
"""
from numpy import *
from math import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from lemke import *
from SymbolicMC import *
from SymbolicForce import *
from SymbolicPenetration import *
from SymbolicTensor import *
from glprim import *
import sys
import matplotlib.pyplot as pit
import ExpBody
from dRdv_real import QuatdFromV
from quat import quat_mult, quat_conj
import lwp
import ctypes as ct
import time

libsimcore = ct.CDLL('/home/johnu/pymuscle/bin/Release/libsimcore_release.so')
#libsimcore = ct.CDLL('/home/johnu/pymuscle/bin/Debug/libsimcore_debug.so')
C_lemke = libsimcore.lemke_Python
C_MassMatrixAndCqdVector = libsimcore.MassMatrixAndCqdVector
C_GeneralizedForce = libsimcore.GeneralizedForce
LCP_exp_Python = libsimcore.LCP_exp_Python

C_DBL66  = (ct.c_double * 6) * 6
C_DBL6   =  ct.c_double * 6
C_DBL3   =  ct.c_double * 3
C_DBL4   =  ct.c_double * 4
C_DBL16  =  ct.c_double * (3+3+3+3+4)

'''
import rpy2.robjects as robjects
from rpy2.robjects.packages import importr
RMatrix = importr('Matrix')
'''

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

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

	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)
	glEnable(GL_NORMALIZE)
	
	noAmbient = [0.0, 0.0, 0.0, 1.0]
	whiteDiffuse = [1.0, 1.0, 1.0, 1.0]
	"""
	Directional light source (w = 0)
	The light source is at an infinite distance,
	all the ray are parallel and have the direction (x, y, z).
	"""
	position = [0.2, -1.0, 1.0, 0.0]
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, noAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glEnable (GL_COLOR_MATERIAL)

	return True

# Reshape The Window When It's Moved Or Resized
def ReSizeGLScene(Width, Height):
	if Height == 0:						# Prevent A Divide By Zero If The Window Is Too Small 
		Height = 1

	glViewport(0, 0, Width, Height)		# Reset The Current Viewport And Perspective Transformation
	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	# // field of view, aspect ratio, near and far
	# This will squash and stretch our objects as the window is resized.
	# Note that the near clip plane is 1 (hither) and the far plane is 1000 (yon)
	gluPerspective(45.0, float(Width)/float(Height), 1, 100.0)

	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	global g_quadratic
	global gWireframe
	global gResetState
	global gNextTestSet
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		gluDeleteQuadric (g_quadratic)
		sys.exit ()
	elif key == 'z':
		gWireframe = not gWireframe
	elif key == 'r':
		gResetState = True
	elif key == 'a':
		gBody.q[3:6] += array([0.05,0.05,0.05])
	
	try:
		keyNum = int(key)
	except:
		keyNum = None
	if keyNum is not None and 1 <= keyNum and keyNum <= len(TESTSET):
		gNextTestSet = int(key)-1
	
	

def SpecialKeyPressed(*args):
	global gNextTestSet
	# If escape is pressed, kill everything.
	key = args [0]

	
def main():
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	width = 320*2
	height = 240*2

	glutInitWindowSize(width, height)

	# the window starts at the upper left corner of the screen 
	glutInitWindowPosition(0, 0)

	# Okay, like the C version we retain the window id to use when closing, but for those of you new
	# to Python, remember this assignment would make the variable local and not global
	# if it weren't for the global declaration at the start of main.
	window = glutCreateWindow("Muscle")

	# Register the drawing function with glut, BUT in Python land, at least using PyOpenGL, we need to
	# set the function pointer and invoke a function to actually register the callback, otherwise it
	# would be very much like the C version of the code.	
	glutDisplayFunc(Draw)

	# Uncomment this line to get full screen.
	#glutFullScreen()

	# When we are doing nothing, redraw the scene.
	glutIdleFunc(Draw)

	# Register the function called when our window is resized.
	glutReshapeFunc(ReSizeGLScene)

	# Register the function called when the keyboard is pressed.  
	glutKeyboardFunc(keyPressed)
	glutSpecialFunc(SpecialKeyPressed)



	# We've told Glut the type of window we want, and we've told glut about
	# various functions that we want invoked (idle, resizing, keyboard events).
	# Glut has done the hard work of building up thw windows DC context and 
	# tying in a rendering context, so we are ready to start making immediate mode
	# GL calls.
	# Call to perform inital GL setup (the clear colors, enabling modes
	Initialize (width, height)

	glShadeModel(GL_SMOOTH)				# Enables Smooth Color Shading
	glClearColor(1.0, 1.0, 1.0, 0.5)	# This Will Clear The Background Color To Black
	glClearDepth(1.0)					# Enables Clearing Of The Depth Buffer
	glEnable(GL_DEPTH_TEST)				# Enables Depth Testing
	glDepthFunc(GL_LEQUAL)				# The Type Of Depth Test To Do
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) # Really Nice Perspective Calculations
	
	
	global cube
	cube = BuildList()

	# Start Event Processing Engine	
	glutMainLoop()

lastDrawTime = time.time()

def Draw():
	global gResetState
	global gNextTestSet
	global gFrame
	global gBodies
	global lastDrawTime
	
	drawTime = time.time()
	
	timeGap = drawTime - lastDrawTime
	#print 'Frame', gFrame, 'FPS', 1/timeGap, '(time gap :', timeGap, ')', 'ang', gBodies[0].q
	lastDrawTime = drawTime
	
	if gNextTestSet is not None:
		gBody = ExpBodyFromTestSet(gNextTestSet)
		gNextTestSet = None
	
	if gUseCversion:
		FrameMove_CVersion(gBodies, h, mu, alpha0, 0, C_NCONEBASIS, C_CONEBASIS)
	else:
		FrameMove_PythonVersion(gBodies, h, mu, alpha0, 0, di)
	
	RenderBox()
	
	gFrame = gFrame + 1
		
	if gResetState:
		gBody.q = hstack([pos0_wc, rot0_wc])
		gBody.qd = hstack([vel0_wc, angvel0_wc])
		gResetState = False

def DynamicReparam(v, vd):
	# v: rotation position
	# vd: rotation velocity
	assert len(v) == 3
	assert len(vd) == 3
	th = linalg.norm(v)
	
	if (th > pi):
		print 'reparmed.', gFrame
		v_r = (1.-2*pi/th)*v
		vnext = v + h*vd
		vnextmag = linalg.norm(vnext)
		vd_r = vd + 2*pi/(th*h)*v - 2*pi/(h*vnextmag)*vnext
		
		v_r_mag = linalg.norm(v_r)
		vnext_r_mag = linalg.norm(v_r+h*vd_r)
		
		return v_r, vd_r
	else:
		return v, vd

def NewDynamicReparam(v, vnext):
	assert len(v) == 3 and len(vnext) == 3
	th = linalg.norm(v)
	thnext = linalg.norm(vnext)
	v_r = (1.-2*pi/th)*v
	vnext_r = (1.-2*pi/thnext)*vnext
	vd_r = (vnext_r - v_r)/h
	return v_r, vd_r

class LemkeFailure:
	# Raised when Lemke's algorithm failed to find the solution
	def __init__(self, msg):
		self.msg = msg

def DoLemkeAndCheckSolution(LCP_M, LCP_q, z0, code):
	assert code in ['C', 'Python']
	if code == 'C':
		# C version
		n = LCP_q.shape[0]
		DBL_M          = (ct.c_double * n) * n
		DBL_q          =  ct.c_double * n
		C_LCP_M = DBL_M()
		C_LCP_q = DBL_q()
		C_LCP_n = ct.c_int(n)
		C_xopt  = DBL_q()
		for i in xrange(n):
			for j in xrange(n):
				C_LCP_M[i][j] = LCP_M[i,j]
			C_LCP_q[i] = LCP_q[i]
		C_lemke(C_LCP_n, C_xopt, C_LCP_M, C_LCP_q)
		err = 0
		x_opt = C_xopt[:]
	elif code == 'Python':
		# Python version
		x_opt, err = lemke(LCP_M, LCP_q, z0)
		
	# Check the sanity of solution
	checkW = dot(LCP_M, x_opt) + LCP_q
	# Allow a very small error (EPS) due to the numerical errors
	# in x_opt and checkW. (mathematically x_opt >= 0 and checkW >= 0)
	# TODO: IS THIS ESSENTIAL?
	EPS = 1e-1
	for i, x, w in zip(range(len(x_opt)), x_opt, checkW):
		# Treat the values between [-EPS,0) as the exact 0.
		if -EPS <= x and x < 0:
			x_opt[i] = 0
		if -EPS <= w and w < 0:
			checkW[i] = 0
	
	# Check the validity of the LCP solution
	all_x_opt_positive = all([val >= 0 for val in x_opt])
	all_w_positive = all([val >= 0 for val in checkW])
	
	wTx = linalg.norm(dot(checkW, x_opt)) # should be zero
	EPS2 = 1e-2
	if err != 0 or wTx > EPS2 or all_x_opt_positive == False or all_w_positive == False:
	#if err != 0 or wTx > EPS2:
	#if err != 0:
		raise LemkeFailure, 'Lemke\'s algorithm failed!!!!!!!!!!!!!!! Try again with rearranged one...'
	return x_opt, err


def GeneralizedForce(v, Fr, r):
	'''
	C wrapper function
	'''
	C_v  = C_DBL3(); C_v[:]  = v
	C_Fr = C_DBL3(); C_Fr[:] = Fr
	C_r  = C_DBL3(); C_r[:]  = r
	C_Q  = C_DBL6();
	
	C_GeneralizedForce(C_Q, C_v, C_Fr, C_r)
	return array(C_Q)


C_M = C_DBL66()
C_Cqd = C_DBL6()
C_P = C_DBL3()
C_V = C_DBL3()
C_PD = C_DBL3()
C_VD = C_DBL3()
C_I  = C_DBL4()
def MassMatrixAndCqdVector(p, v, pd, vd, I):
	global C_P, C_V, C_PD, C_VD, C_I, C_M, C_Cqd
	C_P[:] = p
	C_V[:] = v
	C_PD[:] = pd
	C_VD[:] = vd
	C_I[:] = I
	C_MassMatrixAndCqdVector(C_M, C_Cqd, C_P, C_V, C_PD, C_VD, C_I)
	return array(C_M), array(C_Cqd)

def FrameMove_CVersion(bodies, h, mu, alpha0, nMuscle, C_NCONEBASIS, C_CONEBASIS, contactForceInfoOnly = False):
	nd = 6
	n = len(bodies)
	m = nMuscle # does not matter
	nY = 2*nd*n + m
	C_nd               = ct.c_int(nd)
	C_n                = ct.c_int(n)
	C_m                = ct.c_int(m)
	C_Ynext            = (ct.c_double * nY)()
	C_penetration0     = ct.c_double(alpha0)
	C_Y                = (ct.c_double * nY)()
	C_I                = ((ct.c_double * 4) * n)()
	C_mass             = (ct.c_double * n)()
	C_corners          = (((ct.c_double * 3) * 8) * n)()
	C_mu               = ct.c_double(mu)
	C_h                = ct.c_double(h)
	C_contactForces    = (((ct.c_double * 3) * 8) * n)()
	C_lenContactForces = (ct.c_int * n)()
	C_contactPoints    = ((ct.c_int * 8) * n)()
	for i in xrange(n):
		C_Y[2*nd*i +  0 : 2*nd*i +  6] = bodies[i].q
		C_Y[2*nd*i +  6 : 2*nd*i + 12] = bodies[i].qd
		C_I[i][0:4] = bodies[i].I
		C_mass[i] = bodies[i].mass
		for j in xrange(8):
			C_corners[i][j][0:3] = bodies[i].corners[j]
	
	C_contactForceInfoOnly = ct.c_int( 1 if contactForceInfoOnly else 0 )
	LCP_exp_Python(C_nd, C_n, C_m, C_Ynext,
	               C_contactForces, C_lenContactForces, C_contactPoints,
	               C_penetration0, C_Y, C_I, C_mass, C_corners,
	               C_NCONEBASIS, C_CONEBASIS, C_mu, C_h, C_contactForceInfoOnly)
	
	# For contact force visualization
	for b in bodies:
		b.cf = [] # Clear contact force visualization data
	for i in xrange(n):
		ncf = C_lenContactForces[i]
		for j in xrange(ncf):
			bodies[i].cf.append( array(C_contactForces[i][j]) )
			#print array(C_contactForces[i][j])
		bodies[i].contactPoints = array(C_contactPoints[i])[0:C_lenContactForces[i]]

	if contactForceInfoOnly:
		return any([v>0 for v in C_lenContactForces])

	# Update body data with next step state
	for i in xrange(n):
		bodies[i].q  = array(C_Ynext[2*nd*i +  0 : 2*nd*i +  6])
		bodies[i].qd = array(C_Ynext[2*nd*i +  6 : 2*nd*i + 12])
	
	
	for k in range(n):
		# Angular velocity vector calculation (for visualization)
		r = bodies[k].q[3:6]
		th = linalg.norm(r)
		v1,v2,v3 = r
		if th < 1e-3: coeff = 0.5 - (th**2)/48.
		else: coeff = sin(0.5*th)/th
		quat = array([cos(0.5*th),
			          coeff*v1,
			          coeff*v2,
			          coeff*v3])
		quatd = QuatdFromV(bodies[k].q[3:6], bodies[k].qd[3:6])
		omega_wc = 2*quat_mult(quatd, quat_conj(quat))[1:4]
		bodies[k].omega_wc = omega_wc
		
	minZ = min([cps[2] for cps in bodies[0].getCorners_WC()])
	cfTotal = 0
	if hasattr(bodies[0], 'contactPoints'):
		wc = bodies[0].getCorners_WC()
		for cp, cf in zip(bodies[0].contactPoints, bodies[0].cf):
			cfTotal += linalg.norm(cf)
	
	#print gFrame, 'minZ :', minZ, '/ posz :', bodies[0].q[2], '/ velz :', bodies[0].qd[2], '/ cfTotal :', cfTotal
	#print linalg.norm(gBody.omega_wc)

	return False

def FrameMove_PythonVersion(bodies, h, mu, alpha0, nMuscle, di, contactForceInfoOnly = False):
	nb = len(bodies)

	qPrev = [b.q for b in bodies]
	qdPrev = [b.qd for b in bodies]
	
	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set([])
	
	for k in range(nb):
		bodies[k].contactPoints = []
		# Check all eight corners
		for i, c in zip(range(8), bodies[k].getCorners_WC()):
			if c[2] <= alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
				bodies[k].contactPoints.append(i)
	
	# Indices for active/inactive bodies
	inactiveBodies = list(set(range(nb)) - activeBodies)
	activeBodies = list(activeBodies)
	
	
	# Total number of contact points
	p = len(activeCorners)
	if contactForceInfoOnly and p == 0:
		return None
	
	# Total number of active/inactive bodies
	nba = len(activeBodies)
	nbi = len(inactiveBodies)
	Minv_a = zeros((6*nba, 6*nba))
	Minv_i = zeros((6*nbi, 6*nbi))
	M_i = zeros((6*nbi, 6*nbi)) # for debugging
	Cqd_a = zeros((6*nba))
	Cqd_i = zeros((6*nbi))
	fg_a = zeros((6*nba))
	fg_i = zeros((6*nbi))
	Q_a = zeros((6*nba))
	Q_i = zeros((6*nbi))
	Qd_a = zeros((6*nba))
	Qd_i = zeros((6*nbi))
	q_est = []
	qd_est = []
		
	for k in range(nb):
		bodyk = bodies[k]
		
		#q_est.append(bodyk.q + h*bodyk.qd)
		q_est.append(bodyk.q)
		qd_est.append(bodyk.qd);
		
		q_estk = q_est[k]
		qd_estk = qd_est[k]
		
		M, Cqd = MassMatrixAndCqdVector(q_estk[0:3], q_estk[3:6],
	                                    qd_estk[0:3], qd_estk[3:6], bodyk.I)
		
		Minv_k = linalg.inv(M)
		Cqd_k  = Cqd
		
		fg = GeneralizedForce(q_estk[3:6],
	                          (0., 0., -9.81 * bodyk.mass),
	                          (0., 0., 0.))
		if hasattr(bodyk, 'extForce') and bodyk.extForce is not 0:
			assert len(bodyk.extForce) == 6
			fg += bodyk.extForce
			
		if k in activeBodies:
			kk = activeBodies.index(k)
			Minv_a[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_a[6*kk:6*(kk+1)] = Cqd_k
			fg_a[6*kk:6*(kk+1)] = fg
			Q_a[6*kk:6*(kk+1)] = bodyk.q
			Qd_a[6*kk:6*(kk+1)] = bodyk.qd
		elif k in inactiveBodies:
			kk = inactiveBodies.index(k)
			M_i[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = M # For debugging
			Minv_i[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_i[6*kk:6*(kk+1)] = Cqd_k
			fg_i[6*kk:6*(kk+1)] = fg
			Q_i[6*kk:6*(kk+1)] = bodyk.q
			Qd_i[6*kk:6*(kk+1)] = bodyk.qd
		else:
			raise Exception, 'What the...'
		
	err = 0 # Lemke's algorithm return code: 0 means success
	if p > 0:
		# Basis for contact normal forces (matrix N)
		N = zeros((6*nba, p))
		# Basis for tangential(frictional) forces (matrix D)
		D = zeros((6*nba,8*p))
		for i in range(p):
			# kp: Body index
			# cp: Corner index
			kp, cp = activeCorners[i]
			k = activeBodies.index(kp)
			
			bodyk = bodies[kp]
			q_estk = q_est[kp]
			qd_estk = qd_est[kp]

			# Gradient of the admissible function f(q)
			#          d f(q)
			#         --------
			#           d q
			# { q | f(q) >= 0 }
			#
			N[6*k:6*(k+1), i] = GeneralizedForce(q_estk[3:6], array([0.,0,1]), bodyk.corners[cp])
			
			
			Di = zeros((6*nba,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[6*k:6*(k+1), j] = GeneralizedForce(q_estk[3:6], di[:,j], bodyk.corners[cp])

			D[:, 8*i:8*(i+1)] = Di

		
		M00 = dot(dot(N.T, Minv_a), N)
		M10 = dot(dot(D.T, Minv_a), N)
		M11 = dot(dot(D.T, Minv_a), D)
		Z0 = zeros((p,p))
		
		# Friction coefficient
		Mu = diag([mu]*p)
		# matrix E
		E = zeros((8*p, p))
		for i in range(p):
			E[8*i:8*(i+1),i] = 1.
			
		Minv_fg_Cqd = dot(Minv_a, fg_a - Cqd_a)
		
		LCP_M = vstack([hstack([ M00 ,  M10.T ,  Z0 ]),
	                    hstack([ M10 ,  M11   ,  E  ]),
	                    hstack([ Mu  ,  -E.T  ,  Z0 ])])
		LCP_q0 = dot(N.T, h * Minv_fg_Cqd + Qd_a)
		LCP_q1 = dot(D.T, h * Minv_fg_Cqd + Qd_a)
		LCP_q2 = zeros((p))
		
		# hstack() does not matter since it is a column vector
		LCP_q = hstack([ LCP_q0 ,
	                     LCP_q1 ,
	                     LCP_q2 ])
		
		z0 = zeros(LCP_M.shape[0])
		rearranged = 0
		nY = LCP_M.shape[0]
		
		try:
			x_opt, err = DoLemkeAndCheckSolution(LCP_M, LCP_q, z0, 'Python')
		except LemkeFailure:
			# SHIFT ROW DOWN 1
			LCP_M = vstack([hstack([ -E.T  , Mu  , Z0 ]),
		                    hstack([ M11   , M10 , E  ]),
		                    hstack([ M10.T , M00 , Z0 ])])
			LCP_q0 = zeros((p))
			LCP_q1 = dot(D.T, h * Minv_fg_Cqd + Qd_a)
			LCP_q2 = dot(N.T, h * Minv_fg_Cqd + Qd_a)
			try:
				x_opt, err = DoLemkeAndCheckSolution(LCP_M, LCP_q, z0, 'Python')
				rearranged = 1
				print '******************* REARRANGING(1) SUCCEEDED ........ *******************'
			except LemkeFailure:
				# Do again with rearranged matrix?
				# SWAP ROW 1 and ROW 2
				# SWAP COL 1 and COL 2
				LCP_M = vstack([hstack([ M11   , M10 , E  ]),
			                    hstack([ M10.T , M00 , Z0 ]),
			                    hstack([ -E.T  , Mu  , Z0 ])])
				LCP_q0 = dot(D.T, h * Minv_fg_Cqd + Qd_a)
				LCP_q1 = dot(N.T, h * Minv_fg_Cqd + Qd_a)
				LCP_q2 = zeros((p))
				
				try:
					x_opt, err = DoLemkeAndCheckSolution(LCP_M, LCP_q, z0, 'Python')
					rearranged = 2
					print '******************* REARRANGING(2) SUCCEEDED ........ *******************'
				except LemkeFailure:
					# SHIFT ROW DOWN 1
					LCP_M = vstack([hstack([ M10.T , M00 , Z0 ]),
				                    hstack([ -E.T  , Mu  , Z0 ]),
				                    hstack([ M11   , M10 , E  ])])
					LCP_q0 = dot(N.T, h * Minv_fg_Cqd + Qd_a)
					LCP_q1 = zeros((p))
					LCP_q2 = dot(D.T, h * Minv_fg_Cqd + Qd_a)
					try:
						x_opt, err = DoLemkeAndCheckSolution(LCP_M, LCP_q, z0, 'Python')
						rearranged = 3
						print '******************* REARRANGING(3) SUCCEEDED ........ *******************'
					except LemkeFailure:
						print 'Doriupda...'
						sys.exit(-100)
					
		z0 = x_opt
		
		
		if rearranged == 0:
			cn   = x_opt[0    :p]
			beta = x_opt[p    :p+8*p]
			lamb = x_opt[p+8*p:p+8*p+p]
		elif rearranged in [1,2,3]:
			beta = x_opt[0    :8*p]
			cn   = x_opt[8*p  :8*p+p]
			lamb = x_opt[8*p+p:8*p+p+p]
		else:
			print 'WTF in rearrange.'
			sys.exit(-100)
			#raise Exception, 'WTF...'

		Qimp_cont = dot(Minv_a, dot(N, cn) + dot(D, beta))
		Qd_a_next = h * Minv_fg_Cqd + Qimp_cont + Qd_a
		Q_a_next  = h * Qd_a_next + Q_a
		
		for k in activeBodies:
			kk = activeBodies.index(k)
			bodies[k].q  = Q_a_next[6*kk:6*(kk+1)]
			bodies[k].qd = Qd_a_next[6*kk:6*(kk+1)]
		
		# For contact force visualization
		for b in bodies:
			b.cf = [] # Clear contact force visualization data
		for (kp, cp), k in zip(activeCorners, range(p)):
			# kp: Body index
			# cp: Corner index
			kk = activeBodies.index(kp)
			fric = dot(D[6*kk:6*(kk+1),8*k:8*(k+1)], beta[8*k:8*(k+1)])
			nor  = dot(N[6*kk:6*(kk+1),k], cn[k])
			cf   = (fric[0:3] + nor[0:3]) / h
			bodies[kp].cf.append(cf)
		
		if contactForceInfoOnly:
			return activeCorners
		
	Minv_fg_Cqd = dot(Minv_i, fg_i - Cqd_i)
	Qd_i_next = h * Minv_fg_Cqd + Qd_i
	Q_i_next  = h * Qd_i_next + Q_i
	for k in inactiveBodies:
		kk = inactiveBodies.index(k)
		bodies[k].q = Q_i_next[6*kk:6*(kk+1)]
		bodies[k].qd = Qd_i_next[6*kk:6*(kk+1)]
	
	for k in range(nb):
		# Angular velocity vector calculation (for visualization)
		r = bodies[k].q[3:6]
		th = linalg.norm(r)
		v1,v2,v3 = r
		if th < 1e-3: coeff = 0.5 - (th**2)/48.
		else: coeff = sin(0.5*th)/th
		quat = array([cos(0.5*th),
			          coeff*v1,
			          coeff*v2,
			          coeff*v3])
		quatd = QuatdFromV(bodies[k].q[3:6], bodies[k].qd[3:6])
		omega_wc = 2*quat_mult(quatd, quat_conj(quat))[1:4]
		bodies[k].omega_wc = omega_wc
	
		# Reparameterize rotation value if needed
		th = linalg.norm(bodies[k].q[3:6])
		if th > pi:
			th = linalg.norm(bodies[k].q[3:6])
			bodies[k].q[3:6] = (1.-2*pi/th)*bodies[k].q[3:6]
			qprevmag = linalg.norm(qPrev[k][3:6])
			qPrev[k][3:6] = (1.-2*pi/qprevmag)*qPrev[k][3:6]
			bodies[k].qd[3:6] = (bodies[k].q[3:6] - qPrev[k][3:6])/h
			#print 'Body', k, ' reparmed at frame', gFrame
		
	minZ = min([cps[2] for cps in bodies[0].getCorners_WC()])
	cfTotal = 0
	if hasattr(bodies[0], 'contactPoints'):
		wc = bodies[0].getCorners_WC()
		for cp, cf in zip(bodies[0].contactPoints, bodies[0].cf):
			cfTotal += linalg.norm(cf)
	
	#print gFrame, 'minZ :', minZ, '/ posz :', bodies[0].q[2], '/ velz :', bodies[0].qd[2], '/ cfTotal :', cfTotal
	#print linalg.norm(gBody.omega_wc)

	# Plotting...
	'''
	for i in range(6):
		plotvalues[i].append(gBody.q[i])
	plotvalues[6].append(linalg.norm(gBody.q[3:6]))
	'''
	return False
	
def RenderBox():
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(18,18,18,
	          0,0,4,
	          0,0,1);
	
	for bodyk in gBodies:
		glColor3f(1,0,0)
		glPushMatrix()
		glTranslatef(bodyk.q[0], bodyk.q[1], bodyk.q[2])
		A_homo = identity(4)
		A = bodyk.getRotMat()
		A_homo[0:3,0:3] = A
		glMultMatrixd(A_homo.T.flatten())
		# box(body) frame indicator
		RenderAxis()
		glScalef(bodyk.boxsize[0], bodyk.boxsize[1], bodyk.boxsize[2])
		glColor3f(0.5,0.7,0.9)
		if gWireframe:
			glutWireCube(1)
		else:
			glutSolidCube(1)
		glPopMatrix()
		
		if hasattr(bodyk, 'contactPoints'):
			wc = bodyk.getCorners_WC()
			for cp, cf in zip(bodyk.contactPoints, bodyk.cf):
				glColor3f(0.9,0.2,0.2) # Contact point indicator color
				glPushMatrix()
				glTranslate(wc[cp][0], wc[cp][1], wc[cp][2])
				glutSolidSphere(0.1,8,8)
				glPopMatrix()
				glColor3f(0.9,0.2,0.2) # Contact force vector indicator color
				glBegin(GL_LINES)
				glVertex(wc[cp][0]        , wc[cp][1]        , wc[cp][2]        )
				glVertex(wc[cp][0] + cf[0], wc[cp][1] + cf[1], wc[cp][2] + cf[2])
				glEnd()
			
		if hasattr(bodyk, 'omega_wc'):
			omega_wc = bodyk.omega_wc
			glColor(0,0,0)
			glPushMatrix()
			glTranslatef(bodyk.q[0], bodyk.q[1], bodyk.q[2])
			glBegin(GL_LINES)
			glVertex( omega_wc[0],  omega_wc[1],  omega_wc[2])
			glVertex(-omega_wc[0], -omega_wc[1], -omega_wc[2])
			glEnd()
			glPopMatrix()
	
		
	# Plane
	glColor3f(0,0,0)
	plane=3
	glBegin(GL_LINE_STRIP)
	glVertex3f(-plane, -plane, 0)
	glVertex3f( plane, -plane, 0)
	glVertex3f( plane,  plane, 0)
	glVertex3f(-plane,  plane, 0)
	glVertex3f(-plane, -plane, 0)
	glEnd()
	
	glFlush ()
	glutSwapBuffers()

def ExpBodyFromTestSet(i):
	pos0_wc    = array(TESTSET[i][0], dtype=float64)
	rot0_wc    = array(TESTSET[i][1], dtype=float64)
	vel0_wc    = array(TESTSET[i][2], dtype=float64)
	angvel0_wc = array(TESTSET[i][3], dtype=float64)
	boxsize    = array(TESTSET[i][4], dtype=float64)
	mass       = float(TESTSET[i][5])
	return ExpBody.ExpBody('singlerb', None, mass, boxsize,
	                       hstack([ pos0_wc, rot0_wc ]),
	                       vel0_wc, angvel0_wc,
	                       [0.1,0.2,0.3])
def GetTestSetCount(): return len(TESTSET)

def GoTest():
	bodies = [ ExpBodyFromTestSet(0), ExpBodyFromTestSet(1), ExpBodyFromTestSet(8), ExpBodyFromTestSet(7) ]
	testFrame = 1000
	for i in range(testFrame):
		FrameMove(bodies)
		print i, '/', testFrame, '...'

def BuildConeBasis():
	di = array([ [        1.,         0., 0],
		         [ cos(pi/4),  sin(pi/4), 0],
		         [        0.,         1., 0],
		         [-cos(pi/4),  sin(pi/4), 0],
		         [       -1.,         0., 0],
		         [-cos(pi/4), -sin(pi/4), 0],
		         [         0,        -1., 0],
		         [ cos(pi/4), -sin(pi/4), 0] ])
	return di.T
def BuildConeBasisForC():
	di = BuildConeBasis()
	C_NCONEBASIS       = ct.c_int(8)
	C_CONEBASIS        = ((ct.c_double * 3) * 8)()
	for i in xrange(8):
		C_CONEBASIS[i][0:3] = di[:,i]
	return C_NCONEBASIS, C_CONEBASIS
################################################################################
################################################################################
################################################################################
	
if __name__ == '__main__':
	plotvalue1 = []
	plotvalue2 = []
	plotvalue3 = []
	
	plotvalue4 = []
	plotvalue5 = []
	plotvalue6 = []
	
	# Friction coefficient
	mu = 1.5
	# Simulation Timestep
	h = 0.0025
	# contact level threshold
	alpha0 = 0
	
	# Always raise an exception when there is a numerically
	# incorrect value appeared in NumPy module.
	seterr(all='raise')
	
	plotvalues = [ [],[],[],[],[],[],[] ]
	
	
	# Eight basis of friction force
	di = BuildConeBasis()
	
	#--------------------------------------------------------------------------------------------------------
	# TEST SET      POS0               ROT0           VEL0        ANGVEL0        BOXSIZE      MASS
	#--------------------------------------------------------------------------------------------------------
	TESTSET = (  # Stationary box
		         ( (0,-2,0.5)      , (0,0,0)       ,  (0,0,0)    , (0,0,0)   ,    (1,1,1)  ,     1 ),
		         # Stationary box slightly rotated in z-axis
		         ( (5,5,10)      , (0.3,0,0)       ,  (0,0,0)    , (0,0,0)   ,    (1,1,1)  ,      2 ),
		         # Straight free-fall
		         ( (0,0,5)        , (0.1,0.2,0.3) ,  (0,0,0)    , (0,0,0)   ,    (1,2,3)  ,     1 ),
		         # z-axis rotating free fall
		         ( (0,0,3)        , (0,0,0)       ,  (0,0,0)    ,  (0,0,40) ,   (3,1,1)   ,     1 ),
		         # projectile test
		         ( (-3,0,3)       ,  (0,0,0)      ,  (10,0,0)   , (0,0,0)   ,   (1,1,1)   ,     1 ),
		         # projectile test (faster)
		         ( (-5,0,3)       , (0,0,0)       ,  (20,0,0)   , (0,0,0)   ,   (1,1,1)   ,     1 ),
		         # free-fall of arbitrary rotated state with no angular velocity
		         ( (0,0,3)        , (1,2,3)       ,  (0,0,0)    , (0,0,0)   ,    (1,1,1)  ,     1 ),
		         # free-fall of arbitrary rotated state with some angular velocity
		         ( (0,0,1.5)        , (0.1,0.4,0.8)       ,  (0,0,0)    , (1,-3,30),    (1,1,1)  ,     1 ),
		         # free-fall of arbitrary rotated state with some angular velocity (general box shape)
		         ( (0,0,10)       ,(0.3,0.2,0.1)  ,  (0,0,0)    , (10,10,10),(0.5,0.9,3.5) ,    1 ),
	             # Stationary box
		         ( (0,0,0.5)      , (0,0,0)       ,  (0,0,5)    , (0,0,0)   ,    (1,1,1)  ,     1 ),
		    )

	#gBodies = [ ExpBodyFromTestSet(0), ExpBodyFromTestSet(1), ExpBodyFromTestSet(2), ExpBodyFromTestSet(3) ]
	#gBodies = [ ExpBodyFromTestSet(0), ExpBodyFromTestSet(5), ExpBodyFromTestSet(8), ExpBodyFromTestSet(7) ]
	#gBodies = [ ExpBodyFromTestSet(2), ExpBodyFromTestSet(1) ]
	#gBodies = [ ExpBodyFromTestSet(0),ExpBodyFromTestSet(9) ]
	gBodies = [ ExpBodyFromTestSet(1) ]
	#gBodies = [ ExpBodyFromTestSet(0), ExpBodyFromTestSet(0), ExpBodyFromTestSet(0), ExpBodyFromTestSet(0), ExpBodyFromTestSet(0) ]
	gNextTestSet = None
	z0 = 0
	print 'Initial position :', gBodies[0].q[0:3]
	# Current frame number
	gFrame = 0
	gWireframe = False
	gResetState = False
	gUseCversion = True
	nd = 6
	n = len(gBodies)
	m = 0
	nY = 2*nd*n+m
	alpha0 = 0
	
	C_NCONEBASIS, C_CONEBASIS = BuildConeBasisForC()
		
	# Let's go!
	main()
