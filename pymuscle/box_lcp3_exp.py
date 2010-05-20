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
from dRdv_real import GeneralizedForce, dfxdX, QuatdFromV
from ExpBodyMoEq_real import MassMatrixAndCqdVector
from quat import quat_mult, quat_conj


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
	
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		gluDeleteQuadric (g_quadratic)
		sys.exit ()
	elif key == 'z':
		gWireframe = not gWireframe
	elif key == 'r':
		gResetState = True

def main():
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	width = 320*3
	height = 240*3

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

def Draw():
	global gResetState
	
	FrameMove()
	try:
		pass
	except:
		t = arange(0.,(frame-1)*h, h)
		for i in range(7):
			plotvalues[i] = plotvalues[i][0:frame-10]
		pit.figure(1)
		pit.plot(plotvalues[0], 'r',
	             plotvalues[1], 'g',
	             plotvalues[2], 'b')
		pit.ylabel('lin_pos')
		
		pit.figure(2)
		pit.plot(plotvalues[3], 'r',
	             plotvalues[4], 'g',
	             plotvalues[5], 'b')
		pit.ylabel('rot_pos')
		
		pit.figure(3)
		pit.plot(plotvalues[6], 'r')
		pit.ylabel('rot_mag')
		
		# Show the plot and pause the app
		pit.show()
		sys.exit(-100)
		
	RenderBox()
	
	#print frame, gBody.qd[3:6]
	
	
	if gResetState:
		gBody.q = hstack([pos0_wc, rot0_wc])
		gBody.setQd(vel0_wc, omega0_wc)
		gResetState = False
		
	
		
def FrameMove():
	global frame
	
	nb = 1 # Single rigid body test case
	
	# Dynamic reparameterization of rotation vector
	r = gBody.q[3:6]
	th = linalg.norm(r)
	if (th > pi):
		r = (1.-2*pi/th)*r
		gBody.q[3:6] = r
	
	th = linalg.norm(r)
	v1,v2,v3 = r
	if th < 0.01: coeff = 0.5 - (th**2)/48.
	else: coeff = sin(0.5*th)/th
	quat = array([cos(0.5*th),
                  coeff*v1,
                  coeff*v2,
                  coeff*v3])
	quatd = QuatdFromV(gBody.q[3:6], gBody.qd[3:6])
	omega_wc = 2*quat_mult(quatd, quat_conj(quat))[1:4]
	gBody.omega_wc = omega_wc
	
	
	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set([])
	gBody.contactPoints = []
	for k in range(nb):
		# Check all eight corners
		for i, c in zip(range(8), gBody.getCorners_WC()):			
			if c[2] <= alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
				gBody.contactPoints.append(i)
	
	# Indices for active/inactive bodies
	inactiveBodies = list(set(range(nb)) - activeBodies)
	activeBodies = list(activeBodies)
	
	# Total number of contact points
	p = len(activeCorners)
	# Total number of active/inactive bodies
	nba = len(activeBodies)
	nbi = len(inactiveBodies)
	Minv_a = zeros((6*nba, 6*nba))
	Minv_i = zeros((6*nbi, 6*nbi))
	Cqd_a = zeros((6*nba))
	Cqd_i = zeros((6*nbi))
	Q_a = zeros((6*nba))
	Q_i = zeros((6*nbi))
	Qd_a = zeros((6*nba))
	Qd_i = zeros((6*nbi))
	for k in range(nb):
		q_est  = gBody.q + h*gBody.qd;
		#M, Cqd = MassMatrixAndCqdVector(q_est[0:3], q_est[3:6], gBody.qd[0:3], gBody.qd[3:6], gBody.I)
		M, Cqd = MassMatrixAndCqdVector(gBody.q[0:3], gBody.q[3:6], gBody.qd[0:3], gBody.qd[3:6], gBody.I)
		Minv_k = linalg.inv(M)
		Cqd_k  = Cqd
		
		
		# Add gravitational force to the Coriolis term
		# v, Fr, r
		fg = GeneralizedForce(gBody.q[3:6] + h*gBody.qd[3:6]/2,
		                      (0., 0., -9.81 * gBody.mass),
		                      (0., 0., 0.))
		#Cqd_k = Cqd_k + fg
		
		if k in activeBodies:
			kk = activeBodies.index(k)
			Minv_a[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_a[6*kk:6*(kk+1)] = Cqd_k
			Q_a[6*kk:6*(kk+1)] = gBody.q
			Qd_a[6*kk:6*(kk+1)] = gBody.qd
		elif k in inactiveBodies:
			kk = inactiveBodies.index(k)
			Minv_i[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_i[6*kk:6*(kk+1)] = Cqd_k
			Q_i[6*kk:6*(kk+1)] = gBody.q
			Qd_i[6*kk:6*(kk+1)] = gBody.qd
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

			q_est  = gBody.q + h/2*gBody.qd;
			# Gradient of the admissible function f(q)
			#          d f(q)
			#         --------
			#           d q
			# { q | f(q) >= 0 }
			#
			N[6*k:6*(k+1), i] = dfxdX(q_est[0:3], q_est[3:6], gBody.corners[cp])
			
			Di = zeros((6*nba,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[6*k:6*(k+1), j] = GeneralizedForce(q_est[3:6], di[:,j], gBody.corners[cp])
			D[:, 8*i:8*(i+1)] = Di

		#print '-----------------------------------------------------'
		#print Minv_a, N, D
		
		M00 = dot(dot(N.T, Minv_a), N)
		M10 = dot(dot(D.T, Minv_a), N)
		M11 = dot(dot(D.T, Minv_a), D)
		Z0 = zeros((p,p))
		
		# Friction coefficient
		Mu = diag([mu]*p)
		# matrix E
		E = zeros((8*p, p))
		for i in range(p):
			for j in range(8):
				E[8*i+j, i] = 1
		
		"""
		# Assertion for positive definite mass matrix
		Minve, Minvu = linalg.eig(Minv_cp)
		assert all([me > 0 for me in Minve])
		"""
		
		LCP_M = vstack([hstack([ M00 ,  M10.T ,  Z0 ]),
			            hstack([ M10 ,  M11   ,  E  ]),
			            hstack([ Mu  ,  -E.T  ,  Z0 ])])
	
		LCP_q0 = h * dot(dot(N.T, Minv_a), Cqd_a) + dot(N.T, Qd_a)
		LCP_q1 = h * dot(dot(D.T, Minv_a), Cqd_a) + dot(D.T, Qd_a)
		LCP_q2 = zeros((p))
		# hstack() does not matter since it is a column vector
		LCP_q = hstack([ LCP_q0 ,
			             LCP_q1 ,
			             LCP_q2 ])
		
		# Case 1: Python code (from Matlab)
		z0 = zeros((LCP_M.shape[0]))
		x_opt, err = lemke(LCP_M, LCP_q, z0)
		
		checkValidity = linalg.norm(dot(dot(LCP_M, x_opt) + LCP_q, x_opt))
		if err != 0 or checkValidity > 1.:
			raise Exception, 'Lemke\'s algorithm failed'
		z0 = x_opt
		#print frame, x_opt
		cn   = x_opt[0    :p]
		beta = x_opt[p    :p+8*p]
		lamb = x_opt[p+8*p:p+8*p+p]
		
		Qd_a_next = dot(Minv_a, dot(N, cn) + dot(D, beta) + h*Cqd_a) + Qd_a
		Q_a_next  = h * Qd_a_next + Q_a
		
		for k in activeBodies:
			kk = activeBodies.index(k)
			gBody.q = Q_a_next[6*kk:6*(kk+1)]
			gBody.qd = Qd_a_next[6*kk:6*(kk+1)]

		"""
		contactforce = dot(N, cn) + dot(D, beta)		
		assert all([cf - z == 0 for cf, z in zip(contactforce[6:12], zeros((6)))])
		print contactforce[6:12]
		"""
	
	for k in inactiveBodies:
		# No contact point
		kk = inactiveBodies.index(k)
		Qd_i_next = dot(Minv_i, -h*Cqd_i) + Qd_i
		Q_i_next  = h * Qd_i_next + Q_i
		gBody.q = Q_i_next[6*kk:6*(kk+1)]
		gBody.qd = Qd_i_next[6*kk:6*(kk+1)]
		z0 = 0
		
		print Qd_i_next, gBody.omega_wc
	
	for i in range(6):
		plotvalues[i].append(gBody.q[i])
	plotvalues[6].append(linalg.norm(gBody.q[3:6]))
		
	frame = frame + 1
	
def RenderBox():
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-6,5,
	          0,0,0,
	          0,0,1);
		
	glColor3f(1,0,0)
	glPushMatrix()
	glTranslatef(gBody.q[0], gBody.q[1], gBody.q[2])
	A_homo = identity(4)
	A = gBody.getRotMat()
	A_homo[0:3,0:3] = A
	glMultMatrixd(A_homo.T.flatten())
	# box(body) frame indicator
	RenderAxis()
	glScalef(gBody.boxsize[0], gBody.boxsize[1], gBody.boxsize[2])
	glColor3f(0.5,0.7,0.9)
	if gWireframe:
		glutWireCube(1)
	else:
		glutSolidCube(1)
	glPopMatrix()
	
	if hasattr(gBody, 'contactPoints'):
		wc = gBody.getCorners_WC()
		glColor3f(0.9,0.2,0.2)
		minZ = 10000000.
		for cp in gBody.contactPoints:
			glPushMatrix()
			glTranslate(wc[cp][0], wc[cp][1], wc[cp][2])
			glutSolidSphere(0.1,4,4)
			glPopMatrix()
			if minZ > wc[cp][2]:
				minZ = wc[cp][2]
		print minZ
		if minZ < 0:
			gBody.q[2] -= minZ
	omega_wc = gBody.omega_wc
	glColor(0,0,0)
	glPushMatrix()
	glTranslatef(gBody.q[0], gBody.q[1], gBody.q[2])
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
	

plotvalue1 = []
plotvalue2 = []
plotvalue3 = []

plotvalue4 = []
plotvalue5 = []
plotvalue6 = []
	
################################################################################
# Friction coefficient
mu = 1.2
# Simulation Timestep
h = 0.0025
# contact level threshold
alpha0 = 0

# Always raise an exception when there is a numerically
# incorrect value appeared in NumPy module.
seterr(all='raise')

plotvalues = [ [],[],[],[],[],[],[] ]


# Eight basis of friction force
di = array([ [        1.,         0., 0],
             [ cos(pi/4),  sin(pi/4), 0],
             [        0.,         1., 0],
             [-cos(pi/4),  sin(pi/4), 0],
             [       -1.,         0., 0],
             [-cos(pi/4), -sin(pi/4), 0],
             [         0,        -1., 0],
             [ cos(pi/4), -sin(pi/4), 0] ])
di = di.T

pos0_wc   = array([0.,0.,3])
vel0_wc   = array([0.,0,0])
rot0_wc   = array([0.0,0.0,0.0])
omega0_wc = array([0.,1,1])

gBody = ExpBody.ExpBody('singlerb', None, 2., array([0.5,0.7,0.9]),
                      hstack([ pos0_wc, rot0_wc ]),
                      vel0_wc, omega0_wc,
                      [0.1,0.2,0.3])

# Current frame number
frame = 0
gWireframe = False
gResetState = False
# Let's go!
main()
