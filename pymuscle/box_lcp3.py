#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Single particle LCP-based simulator
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

	width = 320
	height = 240

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
	global mass, Ixx, Iyy, Izz, Iww, q, qd, h, sx, sy, sz, corners, mu, di, frame, alpha0
	global cube
	
	###########################################################################
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-13,8,0,0,0,0,0,1);

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
	
	activeCorners = []
	for i in range(8):
		c = q[0:3] + dot(A, corners[i])
		if c[2] < alpha0:
			activeCorners.append(i)
	
	p = len(activeCorners)
	#M = SymbolicM(q + h*qd, (Ixx, Iyy, Izz, Iww))
	#Minv = linalg.inv(M)
	Minv = SymbolicMinv(q + h*qd, (Ixx, Iyy, Izz, Iww))
	Cqd = SymbolicCqd(q + h*qd/2, qd, (Ixx, Iyy, Izz, Iww))
	
	# Add gravitational force to the Coriolis term
	fg = SymbolicForce(q, (0, 0, -9.81 * mass), (0, 0, 0))
	Cqd = Cqd + fg
	err = 0
	if p > 0:
		# Basis for contact normal forces (matrix N)
		N = zeros((6,p))
		for i in range(p):
			N[:,i] = SymbolicForce(q, (0, 0, 1), corners[activeCorners[i]])
		
		# Basis for tangential(frictional) forces (matrix D)
		D = zeros((6,8*p))
		for i in range(p):
			Di = zeros((6,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[:,j] = SymbolicForce(q, di[j], corners[activeCorners[i]])
			D[:,8*i:8*(i+1)] = Di
			
		M00 = dot(dot(N.T, Minv), N)
		M10 = dot(dot(D.T, Minv), N)
		M11 = dot(dot(D.T, Minv), D)
		Z0 = zeros((p,p))
		
		# Friction coefficient
		Mu = diag([mu]*p)
		# matrix E
		E = zeros((8*p,p))
		for i in range(p):
			for j in range(8):
				E[8*i+j,i] = 1

		LCP_M = vstack([hstack([ M00 ,  M10.T ,  Z0 ]),
			            hstack([ M10 ,  M11   ,  E  ]),
			            hstack([ Mu  ,  -E.T  ,  Z0 ])])
	
		LCP_q0 = h * dot(dot(N.T, Minv), Cqd) + dot(N.T, qd)
		LCP_q1 = h * dot(dot(D.T, Minv), Cqd) + dot(D.T, qd)
		LCP_q2 = zeros((p))
		# hstack() does not matter since it is a column vector
		LCP_q = hstack([ LCP_q0 ,
			             LCP_q1 ,
			             LCP_q2 ])
		
		z0 = zeros((LCP_M.shape[0]))
		x_opt, err = lemke(LCP_M, LCP_q, z0)
		if err != 0:
			raise Exception, 'Lemke\'s algorithm failed'
		cn   = x_opt[0    :p]
		beta = x_opt[p    :p+8*p]
		lamb = x_opt[p+8*p:p+8*p+p]
		qd_next = dot(Minv, dot(N, cn) + dot(D, beta) + h*Cqd) + qd
	else:
		# No contact point
		qd_next = dot(Minv, h*Cqd) + qd
		
	q_next  = h * qd_next + q
	q = q_next
	qd = qd_next
	
	#frame = frame + 1
	#print 'Frame', frame, 'err', err, 'p', p
	

def ang_vel(q, v):
	x, y, z, phi, theta, psi = q
	xd, yd, zd, phid, thetad, psid = v
	return array([phid*sin(theta)*sin(psi)+thetad*cos(psi),
	              phid*sin(theta)*cos(psi)-thetad*sin(psi),
	              phid*cos(theta)+psid])
	
################################################################################
# Friction coefficient
mu = 1.0
# timestep
h = 0.001
alpha0 = 0.0005
mass = 1.1
sx = 3
sy = 2
sz = 1
rho = mass / (sx*sy*sz)
Ixx, Iyy, Izz, Iww = SymbolicTensor(sx, sy, sz, rho)

q = array([0, 0, 7, 1.01, 1.02, 0.03])
qd = array([1, 1, 0, 0, 0, 0])

corners = [ ( sx/2,  sy/2,  sz/2),
            ( sx/2,  sy/2, -sz/2),
            ( sx/2, -sy/2,  sz/2),
            ( sx/2, -sy/2, -sz/2),
            (-sx/2,  sy/2,  sz/2),
            (-sx/2,  sy/2, -sz/2),
            (-sx/2, -sy/2,  sz/2),
            (-sx/2, -sy/2, -sz/2) ]

di = [ (1, 0, 0),
       (cos(pi/4), sin(pi/4), 0),
       (0, 1, 0),
       (-cos(pi/4), sin(pi/4), 0),
       (-1, 0, 0),
       (-cos(pi/4), -sin(pi/4), 0),
       (0, -1, 0),
       (cos(pi/4), -sin(pi/4), 0) ]

frame = 0
cube = 0

main()
