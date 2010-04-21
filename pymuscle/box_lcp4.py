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
from MathUtil import *
import sys
import PIL.Image as pil

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'
LEFTARROW = 100
RIGHTARROW = 102

# A general OpenGL initialization function.  Sets all of the initial parameters. 
def Initialize (Width, Height):				# We call this right after our OpenGL window is created.
	glClearColor(222./255, 227./255, 216./255, 1.0)					# This Will Clear The Background Color To Black
	glClearDepth(1.0)									# Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL)								# The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST)								# Enables Depth Testing
	glShadeModel (GL_FLAT);								# Select Flat Shading (Nice Definition Of Objects)
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) 	# Really Nice Perspective Calculations
	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)
	glEnable (GL_COLOR_MATERIAL)
	glShadeModel(GL_SMOOTH)				# Enables Smooth Color Shading
	glEnable(GL_TEXTURE_2D)
	#glDisable(GL_LIGHTING)
	# Turn on wireframe mode
	#glPolygonMode(GL_FRONT, GL_LINE)
	#glPolygonMode(GL_BACK, GL_LINE)
	global gnd_texture
	gnd_texture = glGenTextures(1)
	assert gnd_texture > 0
	im = pil.open('ground.png') # Open the ground texture image
	im = im.convert('RGB')
	ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBX", 0, -1)
	assert ix*iy*4 == len(image)
	glBindTexture(GL_TEXTURE_2D, gnd_texture)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, ix, iy, GL_RGBA, GL_UNSIGNED_BYTE, image );
	
	global quadric
	quadric = gluNewQuadric(1)
	
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
	gluPerspective(45.0, float(Width)/float(Height), 1, 1000.0)

	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		sys.exit ()
	#print 'Key pressed', key

def specialKeyPressed(*args):
	# If escape is pressed, kill everything.
	key = args [0]
	if key == LEFTARROW:
		frame = frame - 1
	elif key == RIGHTARROW:
		frame = frame + 1
	#print 'Special key pressed', key

def main():
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	width = int(320*2.5)
	height = int(240*2.5)

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
	glutSpecialFunc(specialKeyPressed) # Non-ascii characters



	# We've told Glut the type of window we want, and we've told glut about
	# various functions that we want invoked (idle, resizing, keyboard events).
	# Glut has done the hard work of building up thw windows DC context and 
	# tying in a rendering context, so we are ready to start making immediate mode
	# GL calls.
	# Call to perform inital GL setup (the clear colors, enabling modes
	Initialize (width, height)

	# Start Event Processing Engine	
	glutMainLoop()

def Draw ():
	#global mass, Ixx, Iyy, Izz, Iww, q, qd, h, sx, sy, sz, corners, mu, di, frame, alpha0, z0
	global h, mu, di, frame, alpha0, z0, torque
	global configured
	global cube
	
	###########################################################################
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	glLoadIdentity()											# // Reset The Current Modelview Matrix

	xeye, yeye, zeye = 10, -5, 2.5
	xcenter, ycenter, zcenter = 2.90, -4, 0.5
	xup, yup, zup = 0, 0, 1
	gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

	# Plane
	global gnd_texture
	glDisable(GL_LIGHTING)
	glBindTexture(GL_TEXTURE_2D, gnd_texture)
	glColor3f(1,1,1)
	plane=40
	tex_repeat=10
	glBegin(GL_QUADS)
	glTexCoord2f(0, 0);                   glVertex3f(-plane, -plane, 0)
	glTexCoord2f(tex_repeat, 0);          glVertex3f( plane, -plane, 0)
	glTexCoord2f(tex_repeat, tex_repeat); glVertex3f( plane,  plane, 0)
	glTexCoord2f(0, tex_repeat);          glVertex3f(-plane,  plane, 0)
	glEnd()
	glEnable(GL_LIGHTING)
	# Render the fancy global(inertial) coordinates
	RenderFancyGlobalAxis(quadric, 0.7/2, 0.3/2, 0.025)
		
	for i, cfg in zip(range(len(configured)), configured):
		mass, size, inertia, q, qd, corners, dc = cfg
		sx, sy, sz = size
		
		glPushMatrix()
		glTranslatef(q[0], q[1], q[2])
		A_homo = identity(4)
		A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
		A_homo[0:3,0:3] = A
		glMultMatrixd(A_homo.T.flatten())
		# box(body) frame indicator
		RenderAxis(0.2)
		glScalef(sx, sy, sz)
		glColor3f(dc[0],dc[1],dc[2])
		
		if i >= 8:
			# Ankles and toes are rendered as solid cube
			glutSolidCube(1.)
		else:
			glRotatef(-90,1,0,0)
			glTranslatef(0,0,-0.5)
			
			# Bottom cap
			glPushMatrix()
			glRotatef(180,1,0,0) # Flip the bottom-side cap to invert the normal
			gluDisk(quadric, 0, 0.5, 6, 1)
			glPopMatrix()
			
			gluCylinder(quadric, 0.5, 0.5, 1.0, 6, 8);
			
			# Top cap
			glTranslate(0,0,1)
			gluDisk(quadric, 0, 0.5, 6, 1)
		glPopMatrix()
	
	
	
	
	
	###########################################################################

	# Total number of rigid bodies
	nb = len(configured)

	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set([])
	activeCornerPoints = []
	for k in range(nb):
		# Check all eight corners
		mass, size, inertia, q, qd, corners, dc = configured[k]
		for i in range(8):
			A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
			c = q[0:3] + dot(A, corners[i])
			if c[2] < alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
				activeCornerPoints.append(c)

	glColor3f(0.9, 0.2, 0.1)
	#glPointSize(5.)
	#glBegin(GL_POINTS)
	for acp in activeCornerPoints:
		#glVertex3f(acp[0], acp[1], acp[2])
		glPushMatrix()
		glTranslated(acp[0], acp[1], acp[2])
		glutSolidSphere(0.05, 8, 8)
		glPopMatrix()
	#glEnd()
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
		mass, size, inertia, q, qd, corners, dc = configured[k]
		
		Minv_k = SymbolicMinv(q + h*qd, inertia)
		Cqd_k = SymbolicCqd(q + h*qd/2, qd, inertia)
		
		# Add gravitational force to the Coriolis term
		fg = SymbolicForce(q + h*qd/2, (0, 0, -9.81 * mass), (0, 0, 0))
		#Cqd_k = Cqd_k + fg
		#Cqd_k = Cqd_k - torque[(int)(frame*h)][k]*h
		
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
			mass, size, inertia, q, qd, corners, dc = configured[kp]
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
		ground_reaction_force = dot(N, cn) + dot(D, beta)
		Qd_a_next = dot(Minv_a, ground_reaction_force + h*Cqd_a) + Qd_a
		Q_a_next  = h * Qd_a_next + Q_a
		
		# Contact force visualization
		glColor3f(0.8,0.3,0.2)
		beta_reshaped = beta.reshape(p,8)
		scaleFactor = 250
		#glBegin(GL_LINES)
		for i, acp, cn_i, beta_i, ac_i in zip(range(p), activeCornerPoints, cn, beta_reshaped, activeCorners):
			#bv = dot(D, beta_reshaped)
			bodyidx = activeBodies.index(ac_i[0])
			fric = dot(D[:, 8*i:8*(i+1)], beta_i)[6*bodyidx:6*(bodyidx+1)]
			fricdir = array([fric[0]*scaleFactor,
			                 fric[1]*scaleFactor,
			                 cn_i*scaleFactor])
			friclen = linalg.norm(fricdir)			
			fricdirn = fricdir / friclen
			rotaxis = cross([0,0,1.], fricdirn)
			rotangle = acos(dot(fricdirn,[0,0,1.]))
			glPushMatrix()
			glTranslatef(acp[0], acp[1], acp[2])
			glRotatef(rotangle/math.pi*180,rotaxis[0],rotaxis[1],rotaxis[2])
			RenderArrow(quadric, friclen*0.8, friclen*0.2, 0.015)
			glPopMatrix()
			"""
			glVertex3f(acp[0],
			           acp[1],
			           acp[2])
			glVertex3f(acp[0] + fricdir[0],
			           acp[1] + fricdir[1],
			           acp[2] + fricdir[2])
			"""
			#print fric[0], fric[1], cn_i
		#glEnd()
		
		
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
	#print frame, 'err', err, 'p', p, 'Act', activeBodies, 'Inact', inactiveBodies, 'Corners', activeCorners
	
	if frame >= noFrame-2:
		frame = 0
		
	### TRAJECTORY INPUT ###
	for k in range(nb):
		configured[k][ 3 ] = q_data[frame][k]
		configured[k][ 4 ] = qd_data[frame][k]
	
	
	glFlush ()
	glutSwapBuffers()

	
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
h = 1.
# Contact threshold
alpha0 = 0.025
# Eight basis of friction force
di = [ (1, 0, 0),
       (cos(pi/4), sin(pi/4), 0),
       (0, 1, 0),
       (-cos(pi/4), sin(pi/4), 0),
       (-1, 0, 0),
       (-cos(pi/4), -sin(pi/4), 0),
       (0, -1, 0),
       (cos(pi/4), -sin(pi/4), 0) ]


bodyCfg = [ [ 0.115, 0.144, 0.085, 3,  (0.2,0.1,0.2) ],     # Hips (root)
            [ 0.427, 0.720, 0.184, 30, (0.2,0.1,0.2) ],     # lowerback (trunk)
            [ 0.054, 0.274, 0.054, 3,  (0.2,0.1,0.2) ],     # LHipJoint
            [ 0.054, 0.274, 0.054, 3,  (0.2,0.1,0.2) ],     # RHipJoint
            [ 0.145, 0.450, 0.145, 3,  (0.1,0.6,0.0) ],     # LeftHip
            [ 0.145, 0.450, 0.145, 3,  (0.0,0.2,0.6) ],     # RightHip
            [ 0.145, 0.450, 0.145, 3,  (0.1,0.6,0.0) ],     # LeftKnee
            [ 0.145, 0.450, 0.145, 3,  (0.0,0.2,0.6) ],     # RightKnee
            [ 0.184, 0.210, 0.090, 3,  (0.1,0.6,0.0) ],     # LeftAnkle
            [ 0.184, 0.210, 0.090, 3,  (0.0,0.2,0.6) ],     # RightAnkle
            [ 0.184, 0.105, 0.090, 3,  (0.1,0.6,0.0) ],     # LeftToe
            [ 0.184, 0.105, 0.090, 3,  (0.0,0.2,0.6) ] ]    # RightToe

q_file = open('/media/vm/devel/aran/pymuscle/traj_q.txt', 'r')
qd_file = open('/media/vm/devel/aran/pymuscle/traj_qd.txt', 'r')
qdd_file = open('/media/vm/devel/aran/pymuscle/traj_qdd.txt', 'r')
torque_file = open('/media/vm/devel/aran/pymuscle/torque.txt', 'r')
# Ignore the first line
noFrame, nb = map(int, q_file.readline().strip().split())
qd_file.readline()
qdd_file.readline()
torque_file.readline()
q_data = []
qd_data = []
torque_data = []
for i in range(noFrame-2):
	q_i = []
	qd_i = []
	t_i = []
	for j in range(nb):
		q_ij = array( map(float, q_file.readline().strip().split()) )
		q_i.append(q_ij)
		qd_ij = array( map(float, qd_file.readline().strip().split()) )
		qd_i.append(qd_ij)
		t_ij = array( map(float, torque_file.readline().strip().split()) )
		t_i.append(t_ij)
	q_data.append(q_i)
	qd_data.append(qd_i)
	torque_data.append(t_i)

q_file.seek(0, 0)
q_file.readline()
qd_file.seek(0, 0)
qd_file.readline()
qdd_file.seek(0, 0)
qdd_file.readline()
torque_file.seek(0, 0)
torque_file.readline()
# Body specific parameters and state vectors
configured = []
for bc in bodyCfg:
	sx, sy, sz, mass, dc = bc
	rho = mass / (sx*sy*sz) # density of the body
	Ixx, Iyy, Izz, Iww = SymbolicTensor(sx, sy, sz, rho)
	
	q_ij = array( map(float, q_file.readline().strip().split()) )
	qd_ij = array( map(float, qd_file.readline().strip().split()) )
	#qd_ij = zeros((6))
	
	corners = [ ( sx/2,  sy/2,  sz/2),
		        ( sx/2,  sy/2, -sz/2),
		        ( sx/2, -sy/2,  sz/2),
		        ( sx/2, -sy/2, -sz/2),
		        (-sx/2,  sy/2,  sz/2),
		        (-sx/2,  sy/2, -sz/2),
		        (-sx/2, -sy/2,  sz/2),
		        (-sx/2, -sy/2, -sz/2) ]

	c = [ mass,                       # Mass
	      (sx, sy, sz),               # Size
	      (Ixx, Iyy, Izz, Iww),       # Inertia tensor
	      array(q_ij),                # q (position)
	      array(qd_ij),               # qd (velocity)
	      corners,                    # Corners
	      dc ]                        # Drawing color
	      
	configured.append(c)

	

#configured = configured[0:1]
# For advanced(warm) start of solving LCP
z0 = 0
# Current frame number
frame = 0
# Cube display list
cube = 0        # Cube(display list)
gnd_texture = 0 # Ground texture
cylinder = 0    # GLU quadric
arrowtip = 0    # GLU quadric
quadric = 0
# Let's go!
main()
