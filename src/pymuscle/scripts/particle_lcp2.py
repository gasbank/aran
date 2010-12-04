#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Single particle LCP-based simulator
"""
from numpy import *
from math import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys
import mosek

# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

def sqrtm(M):
	u,s,vh = linalg.svd(M)
	D = diag([sqrt(i) for i in s])
	Msqrt = dot(dot(u,D),vh)
	
	d = [d*d for d in (dot(Msqrt,Msqrt.T) - M).flatten()]
	ds = sum(d)
	print ds
	return dot(dot(u,D),vh)




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

	# Start Event Processing Engine	
	glutMainLoop()


	# Define a stream printer to grab output from MOSEK
def streamprinter(text):
	sys.stdout.write(text)
	sys.stdout.flush()

def optimize(A, b, Msym):
	global env

	# Attach a printer to the environment
	env.set_Stream (mosek.streamtype.log, streamprinter)

	# Create a task
	task = env.Task(0,0)

	# Attach a printer to the task
	task.set_Stream (mosek.streamtype.log, streamprinter)

	bkc = [ mosek.boundkey.lo ] * 10
	blc = list(-b)
	buc = [       +inf      ] * 10

	c   = list(b/2)
	bkx = [mosek.boundkey.lo] * 10
	blx = [        0.0      ] * 10
	bux = [       +inf      ] * 10

	asub = [ array(range(A.shape[0])) ] * A.shape[1]
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

	for j in range(len(aval)):
		# Input column j of A 
		task.putavec(mosek.accmode.var,  # Input columns of A.
				     j,                  # Variable (column) index.
				     asub[j],            # Row index of non-zeros in column j.
				     aval[j])            # Non-zero Values of column j. 
	for i in range(NUMCON):
		task.putbound(mosek.accmode.con,i,bkc[i],blc[i],buc[i])

	# Input the cones
	#task.appendcone(mosek.conetype.quad, 0.0, [ 0, 1 ])
	
	# Set up and input quadratic objective
	qsubi = []
	qsubj = []
	qval = []
	for i in range(Msym.shape[0]):
		for j in range(i+1):
			qsubi.append(i)
			qsubj.append(j)
			qval.append(Msym[i,j])
	task.putqobj(qsubi, qsubj, qval)
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

	if not solsta == mosek.solsta.optimal and not solsta == mosek.solsta.near_optimal:
		print("Other solution status")
		sys.exit(1)

	return xx



def Draw ():
	global rl, vl, n, h, m, D, alpha_0, Msym, M
	
	q1 = dot(D.T, vl + h/m*fg)
	q2 = (dot(n, rl) - alpha_0)/h + dot(n, vl + h/m*fg)
	q = array(list(q1) + [q2] + [0.])
	
	
	print 'M=============================================='
	print '[',
	for i in range(M.shape[0]):
		for j in range(M.shape[1]):
			print '%e ' % M[i,j],
			if j != M.shape[1]-1:
				print ',',
		if i != M.shape[0]-1:
			print ';'
	print ']'
	print 'q=============================================='
	print '[',
	for i in range(len(q)):
		print '%e ' % q[i],
		if i != len(q)-1:
			print ',',
	print ']'

	
	try:
		x_opt = optimize(M, q, Msym)
	except mosek.Exception, e:
		print " ERROR : %s" % str(e.errno)
		if e.msg is not None:
			print "\t%s" % e.msg
			sys.exit(1)
	
	beta = x_opt[0:8]
	cn = x_opt[8]
	lamb = x_opt[9]
	
	rl2 = rl + h*(vl + 1.0/m*(cn*n+dot(D, beta)+h*fg))	
	vl2 = vl + 1.0/m*(cn*n + dot(D, beta) + h*fg)

	rl = rl2
	vl = vl2
	
	#print rl, vl, cn, beta, lamb

	###########################################################################
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-13,8,0,0,0,0,0,1);

	glPointSize(10.0)
	glColor3f(1,0,0)
	glBegin(GL_POINTS)
	glVertex3f(rl[0], rl[1], rl[2])
	glEnd()
	
	f = n*cn + dot(D,beta)
	glColor3f(0,0,1)
	glBegin(GL_LINES)
	glVertex3f(rl[0], rl[1], rl[2])
	glVertex3f(rl[0]+f[0], rl[1]+f[1], rl[2]+f[2])
	glEnd()
	
	
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

# Contact normal
n = array([0.,0,1])
# Friction cone direction basis

D = array([[1, 0, 0],
           [cos(pi/4), sin(pi/4), 0],
           [0, 1, 0],
           [-cos(pi/4), sin(pi/4), 0],
           [-1, 0, 0],
           [-cos(pi/4), -sin(pi/4), 0],
           [0, -1, 0],
           [cos(pi/4), -sin(pi/4), 0]]).T

#D = array([[1,0,0],]*8).T
# Friction coefficient
mu = 1.5
e = array([1.]*8)
alpha_0 = 0.001
h = 0.01
m = 0.1
fg = array([0.,0,-9.81])

M = zeros((10,10))
M[0:8,0:8] = dot(D.T, D)/m
M[0:8,8] = dot(D.T,n)/m
M[0:8,9] = e
M[8,0:8] = dot(n.T, D)/m
M[8,8] = dot(n.T,n)/m
M[9,0:8] = -e.T
M[9,8] = mu

# Symmetric portion of matrix M is calculated.
Msym = (M + M.T)/2

Msyme, Msymv = linalg.eig(Msym)

#print Msyme

rl = array([0.,0,0.0005])
vl = array([20.,0,-10])

# Make a MOSEK environment
env = mosek.Env ()

main()
