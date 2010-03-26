#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Main entry file
"""
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys
from optimization import *
from ArcBall import *
from glprim import *

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

# Number of the glut window.
window = 0

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
	g_ArcBall.setBounds (Width, Height)	# //*NEW* Update mouse bounds for arcball
	return


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	global g_quadratic
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		gluDeleteQuadric (g_quadratic)
		sys.exit ()

def runge_kutta(pos, posd, ang, angd, dt):
	"""
	Fourth-order Runge-Kutta integrator:
	A rigid body's position, velocity, angular position, angular velocity, time step are
	used as inputs.
	Integrate to get next time step's state variable.
	"""


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

	width = 320
	height = 240

	# get a 640 x 480 window 
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


	# GLUT When mouse buttons are clicked in window
	glutMouseFunc (Upon_Click)

	# GLUT When the mouse mvoes
	glutMotionFunc (Upon_Drag)


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

def add2(x,y):
	global dt
	return x+dt/2.0*y
def add3(x,y):
	global dt
	return x+dt*y

def RollbackIfPenetrated(pos0, posd0, ang0, angd0, pos1, posd1, ang1, angd1, box_size, dt):
	"""
	현재 타임 스텝(t=1)에 penetration이 생겼으므로
	지난 타임 스텝(t=0)을 참고하여 정확한 contact point가 나오도록
	새로운 현재 타임 스텝의 스테이트를 계산한다.
	덧붙여 이런 경우에 한해 dt값을 임시적으로 바꿔야 한다.
	정확한 contact point라 함은 z값이 0인 것이다.
	"""
	
def ProcessFD(pos, posd, ang, angd):
	global mass, V, mu_k, box_size, H_com, dt
	
	cornersX, cornersXd = BoxCorners(pos, posd, ang, angd, box_size)
	
	penetrated = []
	for i in range(cornersX.shape[1]):
		if cornersX[2,i] < 0:
			penetrated.append(list(cornersX[0:3,i]))
	
	static_contacts, dynamic_contacts = DetermineContactPoints(cornersX, cornersXd)
	
	n_cs = len(static_contacts)
	n_cd = len(dynamic_contacts)
	n_u = 0

	# For COM
	A_a_com, b_a_com = LinearAccelerationComMatrix(mass, cornersXd, static_contacts, dynamic_contacts, V, mu_k)
	A_com_next, b_com_next = NextPointPositionMatrix(A_a_com, b_a_com, pos, posd, dt)
	A_alpha, VAI = AngularAccelerationComMatrix(cornersX, cornersXd, static_contacts, dynamic_contacts, V, pos, mu_k, H_com)

	assert A_a_com is 0 or A_a_com.shape == (3, 1+4*n_cs+n_cd)
	assert b_a_com.shape in [(3, 1), (3,)]
	assert A_com_next is 0 or A_com_next.shape == (3, 1+4*n_cs+n_cd)
	assert b_com_next.shape in [(3, 1), (3,)]
	assert A_alpha is 0 or A_alpha.shape == (3, 1+4*n_cs+n_cd)

	
	# For contact points
	friction_constraints = []
	for i in static_contacts + dynamic_contacts:
		A_a_q, b_a_q = LinearAccelerationMatrix(A_a_com, b_a_com, cornersX[0:3,i] - pos, A_alpha, angd)
		A_q_next, b_q_next = NextPointPositionMatrix(A_a_q, b_a_q, cornersX[0:3,i], cornersXd[0:3,i], dt)
		assert A_a_q.shape == (3, 1+4*n_cs+n_cd)
		assert b_a_q.shape in [(3, 1), (3,)]
		assert A_q_next.shape == (3, 1+4*n_cs+n_cd)
		assert b_q_next.shape in [(3, 1), (3,)]
		friction_constraints.append((A_q_next, b_q_next))

	x_opt = 0
	linacc = 0
	angacc = 0

	# only if there are one or more contacts occurred
	if len(static_contacts + dynamic_contacts):
		# solve SOCP
		try:
			global env
			P2 = BuildP2Matrix(mu_k, n_cd, cornersXd, dynamic_contacts)
			x_opt = SolveSocp(env, friction_constraints, n_cs, n_cd, n_u)
		except mosek.Exception, e:
			print "ERROR: %s" % str(e.errno)
			if e.msg is not None:
				import traceback
				traceback.print_exc()
				print "\t%s" % e.msg
				sys.exit(1)
		except:
			import traceback
			traceback.print_exc()
			sys.exit(1)

	# accleration calculated from the optimal values
	linacc = (dot(A_a_com,x_opt)+b_a_com.T).flatten()
	angacc = dot(A_alpha,x_opt).flatten()

	assert size(linacc) is 3, 'but %d' % size(linacc)
	if size(angacc) is not 3:
		angacc = [0, 0, 0]

	"""
	print 'Optimal linear acceleration'
	print linacc
	print 'Optimal angular acceleration'
	print angacc
	"""
	
	# contact point and force list calculation for debug rendering
	fc_list = 0 # force
	rc_list = 0 # point
	if x_opt is not 0:
		VAIx = concatenate((zeros((VAI.shape[0],1)), VAI),axis=1)
		fc = dot(VAIx, x_opt) # contact force vectors
		scp = [list(cornersX[0:3,i]) for i in static_contacts]
		dcp = [list(cornersX[0:3,i]) for i in dynamic_contacts]
		fc_list = [list(fc[3*i:3*i+3]) for i in range(size(fc)/3)]
		rc_list = scp+dcp

	return linacc, angacc, fc_list, rc_list


def Draw ():
	global box_size
	global pos, posd, ang, angd

	cornersX, cornersXd = BoxCorners(pos, posd, ang, angd, box_size)

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-13,8,0,0,0,0,0,1);

	#glTranslatef(0,0.0,-6.0)									# // Move Left 1.5 Units And Into The Screen 6.0

	"""
	glPushMatrix();													# // NEW: Prepare Dynamic Transform
	glMultMatrixf(g_Transform);										# // NEW: Apply Dynamic Transform
	glColor3f(0.75,0.75,1.0);
	Torus(0.30,1.00);
	glPopMatrix();													# // NEW: Unapply Dynamic Transform

	glLoadIdentity();												# // Reset The Current Modelview Matrix
	glTranslatef(1.5,0.0,-6.0);										# // Move Right 1.5 Units And Into The Screen 7.0

	glPushMatrix();													# // NEW: Prepare Dynamic Transform
	glMultMatrixf(g_Transform);										# // NEW: Apply Dynamic Transform
	glColor3f(1.0,0.75,0.75);
	gluSphere(g_quadratic,1.3,20,20);
	glPopMatrix();													# // NEW: Unapply Dynamic Transform
	"""

	glColor3f(1,0,0)
	global cube
	glPushMatrix()
	glTranslatef(pos[0],pos[1],pos[2])
	glRotatef(math.degrees(ang[0]), 1, 0, 0)
	glRotatef(math.degrees(ang[1]), 0, 1, 0)
	glRotatef(math.degrees(ang[2]), 0, 0, 1)
	glScalef(box_size[0]/2.0,box_size[1]/2.0,box_size[2]/2.0)
	glCallList(cube)
	glPopMatrix()

	# Axis indicator
	axis=[[1,0,0],[0,1,0],[0,0,1]]
	for a in axis:
		glColor3f(a[0],a[1],a[2])
		glBegin(GL_LINES)
		glVertex3f(0,0,0)
		glVertex3f(a[0],a[1],a[2])
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

	"""
	# Contact point candidates
	glPointSize(6)
	glBegin(GL_POINTS)
	#ii = 0
	for cx in cornersX.T:
		#print ii, cx
		#ii += 1
		glVertex3f(cx[0],cx[1],cx[2])
	glEnd()
	"""

	newPos, newPosd, newAng, newAngd, fc_list, rc_list = RungeKuttaFD(pos, posd, ang, angd)
	
	# Contact point and force visualization
	if fc_list is not 0:
		assert len(fc_list) == len(rc_list)
		glBegin(GL_LINES)
		for i in range(len(fc_list)):
			glVertex3f(rc_list[i][0], rc_list[i][1], rc_list[i][2])
			glVertex3f(rc_list[i][0]+fc_list[i][0], rc_list[i][1]+fc_list[i][1], rc_list[i][2]+fc_list[i][2])
		glEnd()

	# Contact points
	if fc_list is not 0:
		glPointSize(6)
		glBegin(GL_POINTS)
		#ii = 0
		for i in range(len(fc_list)):
			glVertex3f(rc_list[i][0], rc_list[i][1], rc_list[i][2])
		glEnd()
	
	glFlush ()
	glutSwapBuffers()
	
	pos = newPos
	posd = newPosd
	ang = newAng
	angd = newAngd

	global frame
	#print 'Frame', frame, 'drawn.'
	frame+=1
	return

frame=0

def RungeKuttaFD(pos, posd, ang, angd):
	rk = 1
	global dt
	a_linacc, a_angacc, fc_list, rc_list = ProcessFD(pos, posd, ang, angd)
	newPosd = [posd[i] + dt*a_linacc[i] for i in range(3)]	
	newPos = [pos[i] + dt*newPosd[i] for i in range(3)]
	newAngd = [angd[i] + dt*a_angacc[i] for i in range(3)]	
	newAng = [ang[i] + dt*newAngd[i] for i in range(3)]

	if rk is not 0:
		pos_b  = map(add2,pos,posd)
		posd_b = map(add2,posd,a_linacc)
		ang_b  = map(add2,ang,angd)
		angd_b = map(add2,angd,a_angacc)
	
		b_linacc, b_angacc = ProcessFD(pos_b, posd_b, ang_b, angd_b)[0:2]
	
		pos_c  = map(add2,pos,posd_b)
		posd_c = map(add2,posd,b_linacc)
		ang_c  = map(add2,ang,angd_b)
		angd_c = map(add2,angd,b_angacc)
	
		c_linacc, c_angacc = ProcessFD(pos_c, posd_c, ang_c, angd_c)[0:2]
	
		pos_d  = map(add2,pos,posd_c)
		posd_d = map(add2,posd,c_linacc)
		ang_d  = map(add2,ang,angd_c)
		angd_d = map(add2,angd,c_angacc)
	
		d_linacc, d_angacc = ProcessFD(pos_d, posd_d, ang_d, angd_d)[0:2]
	
		newPos  = [pos[i]  + dt/6.0 * (posd[i] + 2*posd_b[i] + 2*posd_c[i] + posd_d[i]) for i in range(3)]
		newPosd = [posd[i] + dt/6.0 * (a_linacc[i] + 2*b_linacc[i] + 2*c_linacc[i] + d_linacc[i]) for i in range(3)]
		newAng =  [ang[i]  + dt/6.0 * (angd[i] + 2*angd_b[i] + 2*angd_c[i] + angd_d[i]) for i in range(3)]
		newAngd = [angd[i] + dt/6.0 * (a_angacc[i] + 2*b_angacc[i] + 2*c_angacc[i] + d_angacc[i]) for i in range(3)]

	return newPos, newPosd, newAng, newAngd, fc_list, rc_list



# Print message to console, and kick off the main to get it rolling.
print "Hit ESC key to quit."

cube = 0
cornersX = 0

mass = 10
box_size = [2,2,1]
H_com = BoxInertia(box_size, mass)
dt = 0.0001
mu_s = 1.6 # static friction coefficient
mu_k = 1.5 # dynamic(kinematic) friction coefficient
V = BuildFrictionConeBasis(mu_s)

# initial Box position(pos) and velocity(posd)
pos = [0, 0, 1+box_size[2]/2.0]
posd = [0, 0, 0]
# Box angle(ang) and angular velocity(angd) in radian
ang = [pi/4+0.1, 0, 0]
angd = [0, 0, 0]

# Make a MOSEK environment
env = mosek.Env ()
mosek.iparam.intpnt_num_threads = 4

main()

