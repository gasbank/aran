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


<<<<<<< .mine
def ProcessFD(pos, posd, A, omega, dt):
	global mass, V, mu_k, box_size, H_com_bar, H_com_inv_bar
	
=======
def ProcessFD(pos, posd, ang, angd):
	global box_size
>>>>>>> .r532
	cornersX, cornersXd, cc = BoxCorners2(pos, posd, A, omega, box_size)
<<<<<<< .mine
	
	static_contacts, dynamic_contacts, impulse_corner = DetermineContactPoints(cornersX, cornersXd)
	"""	
	while len(impulse_corner) is not 0:
		H_com_inv = dot(A, dot(H_com_inv_bar, A.T))
		for ic in impulse_corner:
			r_AP = cornersX.T[ic] - pos
			r_AP_mat = cross_op_mat(r_AP)
			lamb1 = identity(3) / mass - dot(r_AP_mat, dot(H_com_inv, r_AP_mat))
			lamb1 = linalg.inv(lamb1)
			E = diag([0, 0, 0])
			lamb = dot(lamb1, dot(-(E+identity(3)), cornersXd.T[ic]))
			
			posd += lamb / mass
			omega += dot(H_com_inv, cross(cornersX.T[ic], lamb))
	
		cornersX, cornersXd, cc = BoxCorners2(pos, posd, A, omega, box_size)
		static_contacts, dynamic_contacts, impulse_corner = DetermineContactPoints(cornersX, cornersXd)	
	"""
	
	n_cs = len(static_contacts)
	n_cd = len(dynamic_contacts)
	n_u = 0
=======
	A_a, b_a, A_com, b_cs, b_cd, A_alpha, VAI, static_contacts, dynamic_contacts = BuildMatrices(mu, cornersX, cornersXd, dt, H_com, mass, pos, V)
	A_com_pz = dot(array([0, 0, 1]), A_com)
	b_c = b_cs + b_cd;
	b_c2 = 0
	for b_cc in b_c:
		if b_c2 is not 0:
			b_c2 = concatenate((b_c2,b_cc),axis=1)
		else:
			b_c2 = b_cc
>>>>>>> .r532

<<<<<<< .mine
	# For COM
	A_a_com, b_a_com = LinearAccelerationComMatrix(mass, cornersXd, static_contacts, dynamic_contacts, V, mu_k)
	A_com_next, b_com_next = NextPointPositionMatrix(A_a_com, b_a_com, pos, posd, dt)
	A_alpha, b_alpha, VAI = AngularAccelerationComMatrix(cornersX, cornersXd, static_contacts, dynamic_contacts,
	                                                     V, pos, A, omega, H_com_bar, H_com_inv_bar, mu_k)
=======
	#print b_c2
>>>>>>> .r532

<<<<<<< .mine
	assert A_a_com is 0 or A_a_com.shape == (3, 1+4*n_cs+n_cd)
	assert b_a_com.shape in [(3, 1), (3,)]
	assert A_com_next is 0 or A_com_next.shape == (3, 1+4*n_cs+n_cd)
	assert b_com_next.shape in [(3, 1), (3,)]
	assert A_alpha is 0 or A_alpha.shape == (3, 1+4*n_cs+n_cd)

	
	# For contact points
	friction_constraints = []
	for i in static_contacts + dynamic_contacts:
		A_a_q, b_a_q = LinearAccelerationMatrix(A_a_com, b_a_com, cornersX[0:3,i] - pos, A_alpha, b_alpha, omega)
		A_q_next, b_q_next = NextPointPositionMatrix(A_a_q, b_a_q, cornersX[0:3,i], cornersXd[0:3,i], dt)
		assert A_a_q.shape == (3, 1+4*n_cs+n_cd)
		assert b_a_q.shape in [(3, 1), (3,)]
		assert A_q_next.shape == (3, 1+4*n_cs+n_cd)
		assert b_q_next.shape in [(3, 1), (3,)]
		friction_constraints.append((A_q_next, b_q_next))

=======
>>>>>>> .r532
	x_opt = 0
	linacc = 0
	angacc = 0

	# only if there are one or more contacts occurred
	if len(b_c):
		b_pz = min(b_c2[2,:])
		# solve SOCP
		try:
			x_opt = SolveSocp(A_com_pz, b_pz, len(b_cs), len(b_cd))
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
	linacc = (dot(A_a,x_opt)+b_a.T).flatten()
	angacc = dot(A_alpha,x_opt).flatten()

	assert size(linacc) is 3, 'but %d' % size(linacc)
	if size(angacc) is not 3:
		angacc = array([0., 0, 0])

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
	global pos, posd, A, omega

	cornersX, cornersXd, cc = BoxCorners2(pos, posd, A, omega, box_size)

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	gluLookAt(8,-13,8,0,0,0,0,0,1);


	glColor3f(1,0,0)
	global cube
	glPushMatrix()
	glTranslatef(pos[0],pos[1],pos[2])
	A_homo = identity(4)
	A_homo[0:3,0:3] = A
	glMultMatrixf(A_homo.T.flatten())
	RenderAxis()
	glScalef(box_size[0]/2.0,box_size[1]/2.0,box_size[2]/2.0)
	glCallList(cube)
	glPopMatrix()

	RenderAxis()

	# Plane
	glColor3f(0,0,0)
	plane=6
	glBegin(GL_LINE_STRIP)
	glVertex3f(-plane, -plane, 0)
	glVertex3f( plane, -plane, 0)
	glVertex3f( plane,  plane, 0)
	glVertex3f(-plane,  plane, 0)
	glVertex3f(-plane, -plane, 0)
	glEnd()

<<<<<<< .mine
=======
	# Contact point candidates
>>>>>>> .r532
	glPointSize(6)
	glBegin(GL_POINTS)
	for i in range(8):
		if i in cc:
			glColor3f(1,0,0)
		else:
			glColor3f(0,0,1)
		glVertex3f(cornersX[0,i], cornersX[1,i], cornersX[2,i])
	glEnd()
<<<<<<< .mine
	glColor3f(0,0,1)
	glBegin(GL_LINES)
	for i in range(8):
		glVertex3f(cornersX[0,i],
		           cornersX[1,i],
		           cornersX[2,i])
		glVertex3f(cornersX[0,i] + cornersXd[0,i],
		           cornersX[1,i] + cornersXd[1,i],
		           cornersX[2,i] + cornersXd[2,i])
	glEnd()
=======
>>>>>>> .r532

<<<<<<< .mine
	newPos, newPosd, newA, newOmega, fc_list, rc_list = RungeKuttaFD(pos, posd, A, omega)
=======

	newPos, newPosd, newAng, newAngd, fc_list, rc_list = RungeKuttaFD(pos, posd, ang, angd)
>>>>>>> .r532
	
	
	if fc_list is not 0:
		glColor3f(0,0,0)
		glBegin(GL_LINES)
		for i in range(len(fc_list)):
			glVertex3f(rc_list[i][0],
				       rc_list[i][1],
				       rc_list[i][2])
			glVertex3f(rc_list[i][0] + fc_list[i][0],
				       rc_list[i][1] + fc_list[i][1],
				       rc_list[i][2] + fc_list[i][2])
		glEnd()
	
	glFlush ()														# // Flush The GL Rendering Pipeline
	glutSwapBuffers()
	
	pos = newPos
	posd = newPosd
	A = newA
	omega = newOmega

	global frame
<<<<<<< .mine
	if fc_list is not 0:
		print 'Frame %d drawn. (nc=%d)' % (frame, len(fc_list))
	else:
		print 'Frame %d drawn.' % (frame)
=======
	print 'Frame', frame, 'drawn.'
>>>>>>> .r532
	frame+=1
	return


<<<<<<< .mine
def RungeKuttaFD(pos, posd, A, omega):
	global default_dt, box_size, frame
	global frame
	
	rk = 0 # 0 for now
	dt = default_dt
	simulated = 0
	while simulated < dt:
		a_linacc, a_angacc, fc_list, rc_list = ProcessFD(pos, posd, A, omega, dt)
		
		newPos = pos + dt*posd + dt*dt/2.0*a_linacc
		newPosd = posd + dt*a_linacc
		
		newA = A + dt*dot(cross_op_mat(omega), A)
		newOmega = omega + dt*a_angacc
		
		newA = OrthonormalizedOfOrientation(newA)
		
		break
		
		corners, cornersX, cc = BoxCorners2(newPos, newPosd, newA, newOmega, box_size)
		penetrated = False
		for c in corners.T:
			if c[2] < 0:
				penetrated = True
				break
		if penetrated:
			dt = dt/2.0
			#print '[Frame %d] Penetration detected. Set divide dt by 2 (%f)' % (frame, dt)
		else:
			simulated += dt
			
=======
def RungeKuttaFD(pos, posd, ang, angd):
	
	a_linacc, a_angacc, fc_list, rc_list = ProcessFD(pos, posd, ang, angd)
>>>>>>> .r532

<<<<<<< .mine
	if rk is 1:
		pos_b   = pos + dt/2.0*posd
		posd_b  = posd + dt/2.0*a_linacc
		A_b     = A + dt/2.0*dot(cross_op_mat(omega), A)
		omega_b = omega + dt/2.0*a_angacc
=======
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

	"""
	print 'newPos'
	print newPos
	print 'newPosd'
	print newPosd
	print 'newAng'
	print newAng
	print 'newAngd'
	print newAngd
	"""
>>>>>>> .r532
	
<<<<<<< .mine
		b_linacc, b_angacc = ProcessFD(pos_b, posd_b, A_b, omega_b)[0:2]
=======
>>>>>>> .r532
	
<<<<<<< .mine
		pos_c   = pos + dt/2.0*posd_b
		posd_c  = posd + dt/2.0*b_linacc
		A_c     = A + dt/2.0*dot(cross_op_mat(omega_b), A)
		omega_c = omega + dt/2.0*b_angacc
		
		c_linacc, c_angacc = ProcessFD(pos_c, posd_c, A_c, omega_c)[0:2]
	
		pos_d   = pos + dt*posd_c
		posd_d  = posd + dt*c_linacc
		A_d     = A + dt*dot(cross_op_mat(omega_c), A)
		omega_d = omega + dt*c_angacc
	
		d_linacc, d_angacc = ProcessFD(pos_d, posd_d, A_d, omega_d)[0:2]

		"""
		newPos  = [pos[i]  + dt/6.0 * (posd[i] + 2*posd_b[i] + 2*posd_c[i] + posd_d[i]) for i in range(3)]
		newPosd = [posd[i] + dt/6.0 * (a_linacc[i] + 2*b_linacc[i] + 2*c_linacc[i] + d_linacc[i]) for i in range(3)]
		newAng =  [ang[i]  + dt/6.0 * (angd[i] + 2*angd_b[i] + 2*angd_c[i] + angd_d[i]) for i in range(3)]
		newAngd = [angd[i] + dt/6.0 * (a_angacc[i] + 2*b_angacc[i] + 2*c_angacc[i] + d_angacc[i]) for i in range(3)]
=======
>>>>>>> .r532
		"""
		
		newPos   = pos   + dt/6.0 * (posd + 2*posd_b + 2*posd_c + posd_d)
		newPosd  = posd  + dt/6.0 * (a_linacc + 2*b_linacc + 2*c_linacc + d_linacc)
		newA     = A     + dt/6.0 * (  dot(cross_op_mat(omega), A)
		                             + 2*dot(cross_op_mat(omega_b), A)
		                             + 2*dot(cross_op_mat(omega_c), A)
		                             + dot(cross_op_mat(omega_d), A) )
		newOmega = omega + dt/6.0 * (a_angacc + 2*b_angacc + 2*c_angacc + d_angacc)
		
	return newPos, newPosd, newA, newOmega, fc_list, rc_list

# Print message to console, and kick off the main to get it rolling.
print "Hit ESC key to quit."


frame=0

cube = 0
cornersX = 0

<<<<<<< .mine
mass = 1
box_size = array([2.,2,1])
H_com_bar = BoxInertia(box_size, mass)
H_com_inv_bar = linalg.inv(H_com_bar)
default_dt = 0.01 # default time-step (can be divided by 2 subsequently when penetration occurrs)
mu_s = 1.6 # static friction coefficient
mu_k = 1.5 # dynamic(kinematic) friction coefficient
V = BuildFrictionConeBasis(mu_s)
=======
mass = 10
box_size = [2,1,1]
H_com = BoxInertia(box_size, mass)
dt = 0.01
mu = 0.8
V = BuildFrictionConeBasis(mu)
>>>>>>> .r532

# initial Box position(pos) and velocity(posd)
<<<<<<< .mine
pos = array([0., 0, box_size[2]/2.0+colliding_tolerance/2.0])
posd = array([0., 0, 0])
# Box orientation(A) and angular velocity(omega)
A = identity(3)
omega = array([0., 0, 0])
=======
pos = [0, 0, 5+box_size[2]/2.0]
posd = [0, 0, 0]
# Box angle(ang) and angular velocity(angd) in radian
ang = [0, 0, 0]
angd = [0, 0, 0]
>>>>>>> .r532

main()

