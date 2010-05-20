#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Rigid box LCP-based simulator
(without MOSEK optimizer)
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
import PmBody
from quat import *
import ctypes as ct
import lwp

#from mlabwrap import mlab

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

libLcpWrapper = ct.CDLL('/home/johnu/Desktop/libLcpWrapper.so.1.0.1')
C_init2  = libLcpWrapper.init2
C_lemke = libLcpWrapper.lemke

#C_init2()

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
	aspectRatio = float(Width) / Height
	#gluPerspective(45.0, float(Width)/float(Height), 1, 100.0)
	glOrtho(-aspectRatio*2, aspectRatio*2, -1.*2, 1.*2, 1, 1000)

	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	global g_quadratic
	global gWireframe
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		gluDeleteQuadric (g_quadratic)
		sys.exit ()
	elif key == 'z':
		gWireframe = not gWireframe


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
	
	# Turn on wireframe mode
	'''
	glPolygonMode(GL_FRONT, GL_LINE)
	glPolygonMode(GL_BACK, GL_LINE)
	'''
	
	global cube
	cube = BuildList()

	# Start Event Processing Engine	
	glutMainLoop()

def Draw ():
	FrameMove()
	RenderBox()

def FrameMove():
	global h, mu, di, frame, alpha0, z0
	global configured
	global cube
	
	gBody.q[3:7] = quat_normalize(gBody.q[3:7])

	# Copy the state
	curQ = gBody.q
	curQd = gBody.qd
	
	
	gBody.contactPoints = []
	for i, cw in zip(range(8), gBody.getCorners_WC()):
		if cw[2] < 0.0005:
			gBody.contactPoints.append(i)
	K = len(gBody.contactPoints)
	N = 1 # number of rigid bodies
	
	M = gBody.getMassMatrix_WC(curQ + h*curQd/2)
	fext = gBody.getExtVector_WC(curQ + h*curQd/2, curQd)
	#M = gBody.getMassMatrix_WC()
	#fext = gBody.getExtVector_WC()
	
	print linalg.eig(M)[0]
	
	J = array(zeros((6*N, 3*K)))
	for i in range(N):
		for j, cp in zip(range(K), gBody.contactPoints):
			cp_body = gBody.getCorners_WC()[cp]
			Jij = vstack([  identity(3),
				            cross_op_mat(cp_body - gBody.q[0:3]) ])
			J[i*6:(i+1)*6, j*3:(j+1)*3] = Jij
	'''
	if J.shape[0] * J.shape[1] != 0:
		for i in range(J.shape[0]):
			for j in range(J.shape[1]):
				if j%3==0:
					print '|',
				print '%15.6f' % J[i,j],
			print
	'''
	MU    = identity(K) * mu
	Eeta  = array(zeros((eta*K, K)))
	N_mat = array(zeros((3*K, K)))
	D     = array(zeros((3*K, eta*K)))
	
	for j in range(K):
		D[3*j:3*(j+1), eta*j:eta*(j+1)] = di
	
		N_mat[3*j:3*(j+1), j] = array([0.,0,1])
		
		Eeta[eta*j:eta*(j+1), j] = 1.
	
		
	Minv = linalg.inv(M)
	JTMinvJ = dot(dot(J.T, Minv), J)
	JTMinvJD = dot(JTMinvJ, D)
	JTMinvJN = dot(JTMinvJ, N_mat)
	
	linAngVel_WC = gBody.getLinearAngularVelocity_WC()
	
	
	#JTvnext = dot(J.T, linAngVel_WC + dot(Minv, h/2*fext))
	JTvnext = dot(J.T, linAngVel_WC + dot(Minv, h*fext))
	#JTvnext = dot(J.T, linAngVel_WC)
	
	JTv = dot(J.T, linAngVel_WC)
	NTJTv = dot(N_mat.T, JTv)
	DTJTv = dot(D.T, JTv)
	
	#print NTJTv
	print gBody.q[0:3]
	
	
	
	M_11 = dot(D.T, JTMinvJD)
	M_12 = dot(D.T, JTMinvJN)
	M_21 = dot(N_mat.T, JTMinvJD)
	M_22 = dot(N_mat.T, JTMinvJN)
	
	LCP_M = vstack([  hstack([ M_11   , M_12, Eeta         ]),
                      hstack([ M_21   , M_22, zeros((K,K)) ]),
                      hstack([ -Eeta.T, MU  , zeros((K,K)) ])  ])
	LCP_q = hstack([ dot(D.T    , JTvnext),
                     dot(N_mat.T, JTvnext),
                     zeros(K)               ])
	z0 = zeros(LCP_M.shape[0])

	if K > 0:
		
		n = len(LCP_q)
		"""
		# Case 1: Direct C shared object
		DBL_nn  = ct.c_double * (n*n)
		DBL_n  = ct.c_double * n
		C_LCP_M = DBL_nn()
		C_LCP_q = DBL_n()
		C_x_opt = DBL_n()
		for i in range(n):
			for j in range(n):
				C_LCP_M[n*i+j] = ct.c_double(LCP_M[i,j])
			C_LCP_q[i] = ct.c_double(LCP_q[i])
		C_lemke(ct.c_int(n), C_LCP_M, C_LCP_q, C_x_opt)
		"""
		'''
		# Case 2: Python C module
		C_LCP_M = list(LCP_M.flatten())
		C_LCP_q = list(LCP_q)
		C_x_opt = [0.,]*n
		lwp.clemke(1985, C_LCP_M, C_LCP_q, C_x_opt)
		x_opt = array(C_x_opt)
		err = 0
		'''
		
		x_opt, err = lemke(LCP_M, LCP_q, z0)
		
		#x_opt, err = mlab.lemke(LCP_M, LCP_q, z0, nout=2)
		#x_opt = x_opt.flatten()
		#err = err[0][0]
		'''
		try:
		except linalg.LinAlgError:
			t = arange(0.,(frame-1)*h, h)
			pit.figure(1)
			pit.plot(plotvalues[0], 'r',
		             plotvalues[1], 'g',
		             plotvalues[2], 'b')
			pit.ylabel('lin_pos')
			
			pit.figure(2)
			pit.plot(plotvalues[3], 'r',
		             plotvalues[4], 'g',
		             plotvalues[5], 'b',
		             plotvalues[6], 'k')
			pit.ylabel('quat_pos')
			
			# Show the plot and pause the app
			pit.show()
			sys.exit(-100)
		'''
		
		if err != 0:
			raise Exception('Lemke failed!')
	
		
		# Check the solution validity
		checkValid = linalg.norm(dot(dot(LCP_M, x_opt) + LCP_q, x_opt))
		if checkValid > 1.0:
			print 'checkValid =', checkValid
			t = arange(0.,(frame-1)*h, h)
			pit.figure(1)
			pit.plot(plotvalues[0], 'r',
		             plotvalues[1], 'g',
		             plotvalues[2], 'b')
			pit.ylabel('lin_pos')
			
			pit.figure(2)
			pit.plot(plotvalues[3], 'r',
		             plotvalues[4], 'g',
		             plotvalues[5], 'b',
		             plotvalues[6], 'k')
			pit.ylabel('quat_pos')
			
			# Show the plot and pause the app
			pit.show()
			sys.exit(-100)
			raise Exception('Lemke! what\'s wrong with you???')
		
		
		beta = x_opt[0: eta*K]             # impulse quantity
		fn = x_opt[eta*K: eta*K + K]       # impulse quantity

		Nfn_Dbeta = dot(N_mat, fn) + dot(D, beta)
		'''
		generalized_contact_force = array(zeros(6))
		for i, cps in zip(range(K), [gBody.getCorners_WC()[i] for i in gBody.contactPoints]):
			contactForce = Nfn_Dbeta[3*i:3*(i+1)]
			contactTorque = cross(cps - gBody.q[0:3], contactForce)
			generalized_contact_force += hstack([contactForce, contactTorque])
		'''
		Minvhk = dot(Minv, dot(J, Nfn_Dbeta) + h*fext)
	else:
		Minvhk = dot(Minv, h*fext)

	new_linvel_world = gBody.qd[0:3] + Minvhk[0:3]
	new_omega_world = QuatToAngularVel_WC(gBody.q[3:7], gBody.qd[3:7]) + Minvhk[3:6]
	
	# Convert lin(3)+ang(3) quantity to lin(3)+quat(4) form.
	newQd_lin = new_linvel_world
	# Care must be taken when calculating newQd_ang
	new_omega_world_x = quat_to_mat([0]+list(new_omega_world))
	newQd_ang = linalg.solve(2.*identity(4)-h*new_omega_world_x, dot(new_omega_world_x, gBody.q[3:7]))
	#newQd_ang = 0.5*quat_mult([0]+list(new_omega_world), gBody.q[3:7] + h*gBody.qd[3:7]/2)
	newQd = hstack([ newQd_lin, newQd_ang ]) # new velocity set
	
	
	# Copy the next state to the current state
	gBody.qd = newQd
	gBody.q += h*gBody.qd

	# Project the body to the admissible region
	minZ = min([cps[2] for cps in gBody.getCorners_WC()])
	if minZ < 0:
		gBody.q[2] += abs(minZ)
	
	for i in range(7):
		plotvalues[i].append(gBody.q[i])
		
	'''
	qdd = QddFromAngAcc_WC(gBody.q[3:7], gBody.qd[3:7], angacc_world)
	acc = list(linacc_world) + list(qdd)
	gBody.explicitStep(h, acc)
	'''
	
	frame += 1
	print 'Cur frame =', frame
	
def RenderBox():
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(0,-5,0,
	          0,0,0,
	          0,0,1);


		
	glColor3f(1,0,0)
	glPushMatrix()
	glTranslatef(gBody.q[0], gBody.q[1], gBody.q[2])
	A_homo = identity(4)
	A = RotationMatrixFromQuaternion(gBody.q[3], gBody.q[4], gBody.q[5], gBody.q[6])
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
	
	wc = gBody.getCorners_WC()
	glColor3f(0.9,0.2,0.2)
	for cp in gBody.contactPoints:
		glPushMatrix()
		glTranslate(wc[cp][0], wc[cp][1], wc[cp][2])
		glutSolidSphere(0.1,4,4)
		glPopMatrix()
		
	omega_wc = QuatToAngularVel_WC(gBody.q[3:7], gBody.qd[3:7])
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
	

	
	
################################################################################
# Friction coefficient
mu = 10.2
# Simulation Timestep
h = 0.001
# Contact threshold

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
eta = di.shape[1] # Column size equals to the number of basis

q0 = array([1.,0,0,0])
q0 /= linalg.norm(q0)
omega0_wc = array([0.,0,20])
qd0 = 0.5*quat_mult([0]+list(omega0_wc), q0)

gBody = PmBody.PmBody('singlerb', None, 2., array([1.,1,1]),
                      hstack([ array([0.,0,1.6]), q0  ]),
                      hstack([ array([0.,0,0]), qd0 ]),
                      [0.1,0.2,0.3], 'QUAT_WFIRST')
'''
gBody = PmBody.PmBody('singlerb', None, 10., array([0.5,0.6,0.7]), array([ 0.18242372,  1.61107801,  0.29919031, -0.64496142, -0.64510323,
        0.28953127,  0.2898935 ]), array([  1.53889248e-08,   8.54160424e-10,  -9.80998025e-03,
        -4.03499906e-01,  -4.03588608e-01,   1.81136147e-01,
         1.81362795e-01]), [0.1,0.2,0.3], 'QUAT_WFIRST')
'''
# Current frame number
frame = 0

gWireframe = False
# Let's go!
main()
