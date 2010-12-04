#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

OpenGL test area
"""
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys
from glprim import *
from dynamics import *
import math
from numpy import *
from ArcBall import *		# // *NEW* ArcBall header
# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

# Number of the glut window.
window = 0

g_Transform = Matrix4fT ()
g_LastRot = Matrix3fT ()
g_ThisRot = Matrix3fT ()

g_ArcBall = ArcBallT (640, 480)
g_isDragging = False
g_quadratic = None


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
		g_LastRot = copy.copy (g_ThisRot);							# // Set Last Static Rotation To Last Dynamic One
	elif (button == GLUT_LEFT_BUTTON and button_state == GLUT_DOWN):
		# Left button clicked down
		g_LastRot = copy.copy (g_ThisRot);							# // Set Last Static Rotation To Last Dynamic One
		g_isDragging = True											# // Prepare For Dragging
		mouse_pt = Point2fT (cursor_x, cursor_y)
		g_ArcBall.click (mouse_pt);								# // Update Start Vector And Prepare For Dragging

	return

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
	return


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		sys.exit ()


# A general OpenGL initialization function.  Sets all of the initial parameters. 
def Initialize (Width, Height):				# We call this right after our OpenGL window is created.
	glClearColor(0.0, 0.0, 0.0, 1.0)					# This Will Clear The Background Color To Black
	glClearDepth(1.0)									# Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL)								# The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST)								# Enables Depth Testing
	glShadeModel (GL_FLAT);								# Select Flat Shading (Nice Definition Of Objects)
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) 	# Really Nice Perspective Calculations

	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)

	glEnable (GL_COLOR_MATERIAL)

	return True

def main():
	global window
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	width = 820
	height = 240

	# get a 640 x 480 window 
	glutInitWindowSize(width, height)

	# the window starts at the upper left corner of the screen 
	glutInitWindowPosition(0, 0)

	# Okay, like the C version we retain the window id to use when closing, but for those of you new
	# to Python, remember this assignment would make the variable local and not global
	# if it weren't for the global declaration at the start of main.
	window = glutCreateWindow("gltest")

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

	# GLUT When mouse buttons are clicked in window
	glutMouseFunc (Upon_Click)

	# GLUT When the mouse mvoes
	glutMotionFunc (Upon_Drag)

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

def RenderAxis():
	axis=[[1,0,0],[0,1,0],[0,0,1]]
	glBegin(GL_LINES)
	for a in axis:
		glColor3f(a[0],a[1],a[2])
		glVertex3f(0,0,0)
		glVertex3f(a[0],a[1],a[2])
	glEnd()

def Draw ():
	global box_size, r_CM, v_CM, A, L_CM, h, M, I_CM_inv, I_CM_inv_bar, omega

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-13,8,0,0,0,0,0,1);
	glMultMatrixf(g_Transform);
	#glTranslatef(0,0.0,-6.0)									# // Move Left 1.5 Units And Into The Screen 6.0


	glColor3f(1,0,0)
	global cube
	glPushMatrix()
	glTranslatef(r_CM[0], r_CM[1], r_CM[2])
	A_homo = identity(4)
	A_homo[0:3,0:3] = A
	glMultMatrixd(A_homo.T.flatten())
	# box(body) frame indicator
	RenderAxis()
	glScalef(box_size[0]/2.0,box_size[1]/2.0,box_size[2]/2.0)
	glColor3f(1,0,0)
	glCallList(cube)
	glPopMatrix()

	# Global(inertial) frame indicator
	RenderAxis()

	r_corners, v_corners, cc = BoxCorners2(r_CM, v_CM, A, omega, box_size)
	glPointSize(6)
	glBegin(GL_POINTS)
	for i in range(8):
		if i in cc:
			glColor3f(1,0,0)
		else:
			glColor3f(0,0,1)
		glVertex3f(r_corners[0,i], r_corners[1,i], r_corners[2,i])
	glEnd()
	glColor3f(0,0,1)
	glBegin(GL_LINES)
	for i in range(8):
		glVertex3f(r_corners[0,i],
		           r_corners[1,i],
		           r_corners[2,i])
		glVertex3f(r_corners[0,i] + v_corners[0,i],
		           r_corners[1,i] + v_corners[1,i],
		           r_corners[2,i] + v_corners[2,i])
	glEnd()

	# Plane
	glColor3f(0,0,0)
	plane = 6
	glBegin(GL_LINE_STRIP)
	glVertex3f(-plane, -plane, 0)
	glVertex3f( plane, -plane, 0)
	glVertex3f( plane,  plane, 0)
	glVertex3f(-plane,  plane, 0)
	glVertex3f(-plane, -plane, 0)
	glEnd()

	glFlush ()
	glutSwapBuffers()

	r_CM += h*v_CM
	v_CM += h*array([0,0,-9.81])/M
	A += h*dot(cross_op_mat(omega),A)
	L_CM += h*0

	A = OrthonormalizedOfOrientation(A)

	I_CM_inv = dot(A, dot(I_CM_inv_bar, A.T))
	omega = dot(I_CM_inv, L_CM)

# Print message to console, and kick off the main to get it rolling.
print "Hit ESC key to quit."

cube = 0

# Initialization:
#   Determine body constants:
box_size = [6.,2.,4.]
M = 10.0
I_CM_inv_bar = linalg.inv(BoxInertia(box_size, M))
h = 0.0005 # integration time-step

#   Determine initial conditions:
r_CM = array([0., 0, 2+box_size[2]/2.0])
v_CM = array([0., 0, 2])
A = array([[1.,0,0],
           [0,1,0],
           [0,0,1]])
L_CM = array([10., 10, 20])
#   Compute initial auxiliary quantities:
I_CM_inv = dot(A, dot(I_CM_inv_bar, A.T))
omega = dot(I_CM_inv, L_CM)


A=array([[1.,2,3],[4,5,6],[7,8,9]])
x=array([10.,20,30])

print cross_op_mat(dot(A,x))
print dot(A, cross_op_mat(x))

main()

