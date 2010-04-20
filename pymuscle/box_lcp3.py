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
	# Why? this tutorial never maps any textures?! ? 
	# gluQuadricTexture(g_quadratic, GL_TRUE);			# // Create Texture Coords

	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)

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
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		gluDeleteQuadric (g_quadratic)
		sys.exit ()


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
	glDisable(GL_LIGHTING)
	# Turn on wireframe mode
	glPolygonMode(GL_FRONT, GL_LINE)
	glPolygonMode(GL_BACK, GL_LINE)
	
	
	global cube
	cube = BuildList()

	# Start Event Processing Engine	
	glutMainLoop()

def RotationMatrixFromEulerAngles(phi, theta, psix):
	return array([[ cos(phi)*cos(psix) - cos(theta)*sin(phi)*sin(psix), - cos(psix)*sin(phi) - cos(phi)*cos(theta)*sin(psix),  sin(psix)*sin(theta)],
	              [ cos(phi)*sin(psix) + cos(psix)*cos(theta)*sin(phi),   cos(phi)*cos(psix)*cos(theta) - sin(phi)*sin(psix), -cos(psix)*sin(theta)],
	              [                                sin(phi)*sin(theta),                                  cos(phi)*sin(theta),            cos(theta)]])
	
def Draw ():
	#global mass, Ixx, Iyy, Izz, Iww, q, qd, h, sx, sy, sz, corners, mu, di, frame, alpha0, z0
	global h, mu, di, frame, alpha0, z0
	global configured
	global cube
	
	###########################################################################
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(5,-10,5,0,0,0,0,0,1);

	for cfg in configured:
		mass, size, inertia, q, qd, corners = cfg
		sx, sy, sz = size
		
		glColor3f(1,0,0)
		glPushMatrix()
		glTranslatef(q[0], q[1], q[2])
		A_homo = identity(4)
		A = RotationMatrixFromEulerAngles(q[3], q[4], q[5])
		A_homo[0:3,0:3] = A
		glMultMatrixd(A_homo.T.flatten())
		# box(body) frame indicator
		RenderAxis()
		glScalef(sx/2.0, sy/2.0, sz/2.0)
		glColor3f(1,0,0)
		glCallList(cube)
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
	
	###########################################################################

	# Total number of rigid bodies
	nb = len(configured)

	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set([])
	for k in range(nb):
		# Check all eight corners
		mass, size, inertia, q, qd, corners = configured[k]
		for i in range(8):
			A = RotationMatrixFromEulerAngles(q[3], q[4], q[5])
			c = q[0:3] + dot(A, corners[i])
			if c[2] < alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
	
	# Indices for active/inactive bodies
	inactiveBodies = list(set(range(nb)) - activeBodies)
	activeBodies = list(activeBodies)

	"""
	# Debug purpose
	inactiveBodies = []
	activeBodies = range(nb)
	"""
	
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
		mass, size, inertia, q, qd, corners = configured[k]
		
		Minv_k = SymbolicMinv(q + h*qd, inertia)
		Cqd_k = SymbolicCqd(q + h*qd/2, qd, inertia)
		# Add gravitational force to the Coriolis term
		fg = SymbolicForce(q + h*qd/2, (0, 0, -9.81 * mass), (0, 0, 0))
		Cqd_k = Cqd_k + fg
		
		if k in activeBodies:
			kk = activeBodies.index(k)
			Minv_a[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_a[6*kk:6*(kk+1)] = Cqd_k
			Q_a[6*kk:6*(kk+1)] = q
			Qd_a[6*kk:6*(kk+1)] = qd
		elif k in inactiveBodies:
			kk = inactiveBodies.index(k)
			Minv_i[6*kk:6*(kk+1), 6*kk:6*(kk+1)] = Minv_k
			Cqd_i[6*kk:6*(kk+1)] = Cqd_k
			Q_i[6*kk:6*(kk+1)] = q
			Qd_i[6*kk:6*(kk+1)] = qd
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
			mass, size, inertia, q, qd, corners = configured[kp]
			k = activeBodies.index(kp)

			# Which one is right?
			#N[6*k:6*(k+1), i] = SymbolicForce(q + h*qd/2, (0, 0, 1), corners[cp])
			N[6*k:6*(k+1), i] = SymbolicPenetration(q + h*qd/2, corners[cp])
			
			
			Di = zeros((6*nba,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[6*k:6*(k+1), j] = SymbolicForce(q + h*qd/2, di[j], corners[cp])
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
		
		if (z0 is 0) or (len(z0) != LCP_M.shape[0]):
			z0 = zeros((LCP_M.shape[0]))
		z0 = zeros((LCP_M.shape[0]))
		x_opt, err = lemke(LCP_M, LCP_q, z0)
		if err != 0:
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
			configured[k][ 3 ] = Q_a_next[6*kk:6*(kk+1)]
			configured[k][ 4 ] = Qd_a_next[6*kk:6*(kk+1)]

		"""
		contactforce = dot(N, cn) + dot(D, beta)		
		assert all([cf - z == 0 for cf, z in zip(contactforce[6:12], zeros((6)))])
		print contactforce[6:12]
		"""
	
	for k in inactiveBodies:
		# No contact point
		kk = inactiveBodies.index(k)
		Qd_i_next = dot(Minv_i, h*Cqd_i) + Qd_i
		Q_i_next  = h * Qd_i_next + Q_i
		configured[k][ 3 ] = Q_i_next[6*kk:6*(kk+1)]
		configured[k][ 4 ] = Qd_i_next[6*kk:6*(kk+1)]
		z0 = 0

	frame = frame + 1
	print frame, 'err', err, 'p', p, 'Act', activeBodies, 'Inact', inactiveBodies, 'Corners', activeCorners

def ang_vel(q, v):
	x, y, z, phi, theta, psi = q
	xd, yd, zd, phid, thetad, psid = v
	return array([phid*sin(theta)*sin(psi)+thetad*cos(psi),
	              phid*sin(theta)*cos(psi)-thetad*sin(psi),
	              phid*cos(theta)+psid])
	
################################################################################
# Friction coefficient
mu = 1.7
# Simulation Timestep
h = 0.0025
# Contact threshold
alpha0 = 0.001
# Eight basis of friction force
di = [ (1, 0, 0),
       (cos(pi/4), sin(pi/4), 0),
       (0, 1, 0),
       (-cos(pi/4), sin(pi/4), 0),
       (-1, 0, 0),
       (-cos(pi/4), -sin(pi/4), 0),
       (0, -1, 0),
       (cos(pi/4), -sin(pi/4), 0) ]



# Body specific parameters and state vectors
config = [ ( 1.1,                                  # Mass
            (0.3, 0.2, 0.1),                       # Size
            (0, 0, 0, 0),                          # Inertia tensor
            array([0, 0, 2, 0.3,   0.2,   0.1]),   # q  (position)
            array([-4, 0, 0, 0, 2, 2]),             # qd (velocity)
            [] ),                                  # Corners
           ( 1.1,
            (0.3, 0.2, 0.1),
            (0, 0, 0, 0),
            array([0, 0,  2, 0.0,  0.0001,  0.0]),
            array([0, 0, 0, 0, 3, 0]),
            [] ),
           ( 1.1,
            (0.3, 0.2, 0.1),
            (0, 0, 0, 0),
            array([-1, 1,  2, 0.3, 0.2, 0.1]),
            array([0, 2, 0, 0, 0, 0]),
            [] ) ]

config = config[0:2]

configured = []
for cfg in config:
	mass, size, inertia, q, qd, corners = cfg
	sx, sy, sz = size
	rho = mass / (sx*sy*sz)
	Ixx, Iyy, Izz, Iww = SymbolicTensor(sx, sy, sz, rho)	
	inertia = (Ixx, Iyy, Izz, Iww)
	# Corner points of a box in local coordinates
	corners = [ ( sx/2,  sy/2,  sz/2),
		        ( sx/2,  sy/2, -sz/2),
		        ( sx/2, -sy/2,  sz/2),
		        ( sx/2, -sy/2, -sz/2),
		        (-sx/2,  sy/2,  sz/2),
		        (-sx/2,  sy/2, -sz/2),
		        (-sx/2, -sy/2,  sz/2),
		        (-sx/2, -sy/2, -sz/2) ]
	configured.append( [mass, size, inertia, q, qd, corners] )


# For advanced(warm) start of solving LCP
z0 = 0
# Current frame number
frame = 0
# Cube display list
cube = 0

# Let's go!
main()