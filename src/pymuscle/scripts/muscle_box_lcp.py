#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A rigid box connected with a muscle fiber
(LCP-based simulator)
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
import pygame
from pygame.font import *
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

	pygame.font.init()
	if not pygame.font.get_init():
		print 'Could not render font.'
		sys.exit(0)
	myFont = pygame.font.Font('ARIALN.TTF',40)
	global myChar
	myChar = []
	for c in range(256):
		s = chr(c)
		try:
			letter_render = myFont.render(s, 1, (255,255,255), (0,0,0))
			letter = pygame.image.tostring(letter_render, 'RGBA', 1)
			letter_w, letter_h = letter_render.get_size()
		except:
			letter = None
			letter_w = 0
			letter_h = 0
		myChar.append( (letter, letter_w, letter_h) )
	myChar = tuple(myChar)
	myLw = myChar[ord('0')][1]
	myLh = myChar[ord('0')][2]

	return True


def textView():
	global glWidth, glHeight
	glViewport(0,0,glWidth,glHeight)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	glOrtho(0.0, glWidth - 1.0, 0.0, glHeight - 1.0, -1.0, 1.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
def Print(s,x,y):
	global myChar
	s = str(s)
	i = 0
	lx = 0
	length = len(s)
	textView()
	glPushMatrix()
	while i < length:
		glRasterPos2i(x + lx, y)
		ch = myChar[ ord( s[i] ) ]
		glDrawPixels(ch[1], ch[2], GL_RGBA, GL_UNSIGNED_BYTE, ch[0])
		lx += ch[1]
		i += 1
	glPopMatrix()

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
	
	global glWidth, glHeight
	glWidth = Width
	glHeight = Height


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		sys.exit ()
	#print 'Key pressed', key

def specialKeyPressed(*args):
	# If escape is pressed, kill everything.
	global curFrame, noFrame
	key = args [0]
	if key == LEFTARROW:
		if curFrame > 0:
			curFrame = curFrame - 1
			InnerDraw()
	elif key == RIGHTARROW:
		if curFrame < noFrame-3:
			curFrame = curFrame + 1
			InnerDraw()
	print 'Special key pressed', key, 'Frame', curFrame

def main():
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	global glWidth, glHeight
	glutInitWindowSize(glWidth, glHeight)

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
	Initialize (glWidth, glHeight)

	# Start Event Processing Engine	
	glutMainLoop()

def Draw():
	global autoPlay, drawCoupon
	if autoPlay or drawCoupon>0:
		drawCoupon = drawCoupon - 1
		InnerDraw()


def InnerDraw ():
	global h, mu, di, curFrame, alpha0, torque
	global configured
	global glWidth, glHeight
	# Total number of rigid bodies
	nb = len(configured)
	
	"""
	### TRAJECTORY INPUT ###
	for k in range(nb):
		configured[k][ 3 ] = q_data[curFrame][k]
		configured[k][ 4 ] = qd_data[curFrame][k]
	"""
	
	###########################################################################

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	
	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	# // field of view, aspect ratio, near and far
	# This will squash and stretch our objects as the window is resized.
	# Note that the near clip plane is 1 (hither) and the far plane is 1000 (yon)
	gluPerspective(45.0, float(glWidth)/float(glHeight), 1, 1000.0)

	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix

	xeye, yeye, zeye = 5, -5, 5
	xcenter, ycenter, zcenter = 0,0,0
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

	# Muscle fixed position
	glPushMatrix()
	muscleFixPos = array([0,0,3])
	glTranslatef(muscleFixPos[0], muscleFixPos[1], muscleFixPos[2])
	glutSolidSphere(0.1, 8, 8)
	glPopMatrix()
	
	for i, cfg in zip(range(len(configured)), configured):
		mass, size, inertia, q, qd, corners, dc = cfg
		sx, sy, sz = size
		
		

		glPushMatrix()
		glTranslatef(q[0], q[1], q[2])
		A_homo = identity(4)
		A = RotationMatrixFromEulerAngles_zxz(q[3], q[4], q[5])
		A_homo[0:3,0:3] = A
		glMultMatrixd(A_homo.T.flatten())
		# box(body) frame indicator
		RenderAxis(0.2)
		glScalef(sx, sy, sz)
		glColor3f(dc[0],dc[1],dc[2])


		if i >= 0:
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

	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set([])
	activeCornerPoints = []
	for k in range(nb):
		# Check all eight corners
		mass, size, inertia, q, qd, corners, dc = configured[k]
		for i in range(8):
			A = RotationMatrixFromEulerAngles_zxz(q[3], q[4], q[5])
			c = q[0:3] + dot(A, corners[i])
			if c[2] < alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
				activeCornerPoints.append(c)

	glColor3f(0.1, 0.2, 0.9)
	#glPointSize(5.)
	#glBegin(GL_POINTS)
	for acp in activeCornerPoints:
		#glVertex3f(acp[0], acp[1], acp[2])
		glPushMatrix()
		glTranslated(acp[0], acp[1], acp[2])
		glutSolidSphere(0.035, 8, 8)
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

		oRb = RotationMatrixFromEulerAngles_zxz(q[3], q[4], q[5])
		omega = AngularVelocityFromEulerAngles_zxz( q[3],  q[4],  q[5],
		                                           qd[3], qd[4], qd[5])
		
		#Minv_k = SymbolicMinv(q + h*qd, inertia)
		#Minv_k = linalg.inv(SymbolicM_Manual(q + h*qd, size[0], size[1], size[2], mass))
		Minv_k = linalg.inv(SymbolicM_Global(oRb, size[0], size[1], size[2], mass))
			
		#Cqd_k = SymbolicCqd(q + h*qd/2, qd, inertia)
		#Cqd_k = SymbolicCqd_Manual(q + h*qd/2, qd, size[0], size[1], size[2], mass)
		Cqd_k = SymbolicC_Global(oRb, omega, size[0], size[1], size[2], mass)
		
		
		# Add muscle fiber tension
		muscleAttachedPosLocal = array([0, 0.1, 0])
		
		muscleLenRate = linalg.norm(qd[0:3] + cross(omega, muscleAttachedPosLocal))
		muscleAttachedPosGlobal =  q[0:3] + dot(A, muscleAttachedPosLocal)
		muscleDir = muscleFixPos - muscleAttachedPosGlobal
		muscleLen = linalg.norm(muscleDir)
		muscleDir = muscleDir/muscleLen # Normalize direction
		tension = MuscleFiberTension(0, muscleLen, muscleLenRate)
		#print curFrame, q[3],q[4],q[5], tension, omega, muscleLenRate
		tensionG = SymbolicForce_Manual(q + h*qd/2, tuple(muscleDir*tension), tuple(muscleAttachedPosLocal))
		#Cqd_k = Cqd_k + tensionG
		
		f1 = SymbolicForce_Manual(q + h*qd/2, (0,0,45), (0,1,0))
		#Cqd_k = Cqd_k + f1
		f2 = SymbolicForce_Manual(q, (0,0,5), (0,-0.1,0))
		Cqd_k = Cqd_k + f2
		
		
		# Add gravitational force to the Coriolis term
		fg = SymbolicForce_Manual(q + h*qd/2, (0, 0, -9.81 * mass), (0, 0, 0))
		#Cqd_k = Cqd_k + fg

		# (wanna be) Control input
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
			# This is WRONG
			#N[6*k:6*(k+1), i] = SymbolicForce_Manual(q + h*qd/2, (0, 0, 1), corners[cp])
			# RIGHT
			N[6*k:6*(k+1), i] = SymbolicPenetration(q + h*qd/2, corners[cp])


			Di = zeros((6*nba,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[6*k:6*(k+1), j] = SymbolicForce_Manual(q + h*qd/2, di[j], corners[cp])
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

		"""
		if (z0 is 0) or (len(z0) != LCP_M.shape[0]):
			z0 = zeros((LCP_M.shape[0]))
		"""
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
		Qd_a_next = dot(Minv_a, ground_reaction_force + h*Cqd_a) + Qd_a # Qd_a, Qd_a_next's rotation part represent angular velocity
		Qd_a_next_Euler = Qd_a_next.copy()
		for ii in range(nba):
			ome = dot(oRb.T, Qd_a_next_Euler[6*ii+3:6*(ii+1)])
			print curFrame
			print Qd_a_next_Euler[6*ii+3:6*(ii+1)]
			ome = EulerAngleRateFromAngularVelocity_zxz(Q_a[6*ii+3],
			                                            Q_a[6*ii+4],
			                                            Q_a[6*ii+5],
			                                            ome)
			print Qd_a_next_Euler[6*ii+3:6*(ii+1)]
			Qd_a_next_Euler[6*ii+3:6*(ii+1)] = dot(oRb, ome)
		Q_a_next  = h * Qd_a_next_Euler + Q_a

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
		Qd_i_next_Euler = Qd_i_next.copy()
		for ii in range(nbi):
			ome = dot(oRb.T, Qd_i_next_Euler[6*ii+3:6*(ii+1)])
			ome = EulerAngleRateFromAngularVelocity_zxz(Q_i[6*ii+3],
			                                            Q_i[6*ii+4],
			                                            Q_i[6*ii+5],
			                                            ome)
			Qd_i_next_Euler[6*ii+3:6*(ii+1)] = dot(oRb, ome)
			
		Q_i_next  = h * Qd_i_next_Euler + Q_i
		configured[k][ 3 ] = Q_i_next[6*kk:6*(kk+1)]
		configured[k][ 4 ] = Qd_i_next[6*kk:6*(kk+1)]
		z0 = 0

	if autoPlay:
		curFrame = curFrame + 1
		
	#print frame, 'err', err, 'p', p, 'Act', activeBodies, 'Inact', inactiveBodies, 'Corners', activeCorners
	#print curFrame
	
	if curFrame <= 26:
		curPhase = '.....'
	elif curFrame <= 31:
		curPhase = 'initial contact'
	elif curFrame <= 44:
		curPhase = 'loading response'
	elif curFrame <= 68:
		curPhase = 'mid stance'
	elif curFrame <= 92:
		curPhase = 'terminal stance'
	elif curFrame <= 99:
		curPhase = 'pre swing'
	elif curFrame <= 107:
		curPhase = 'initial swing'
	elif curFrame <= 128:
		curPhase = 'mid swing'
	elif curFrame <= 161:
		curPhase = 'terminal swing'
	else:
		curPhase = '.....'
	Print(curPhase, 100, 100)
	

	#glFlush ()
	glutSwapBuffers()

	# To save each frame to an image file
	"""
	glPixelStorei(GL_PACK_ALIGNMENT, 4)
	glPixelStorei(GL_PACK_ROW_LENGTH, 0)
	glPixelStorei(GL_PACK_SKIP_ROWS, 0)
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0)
	global glWidth, glHeight
	pixels = glReadPixels(0, 0, glWidth, glHeight, GL_RGBA, GL_UNSIGNED_BYTE)
	surface = pygame.image.fromstring(pixels, (glWidth,glHeight), 'RGBA', 1)
	pygame.image.save(surface,'/home/johnu/ss/%04d.jpg'%curFrame)
	"""

################################################################################
# Friction coefficient
mu = 0
# Simulation Timestep
h = 0.001
# Contact threshold
alpha0 = 0.05
# Eight basis of friction force
di = [ (1, 0, 0),
       (cos(pi/4), sin(pi/4), 0),
       (0, 1, 0),
       (-cos(pi/4), sin(pi/4), 0),
       (-1, 0, 0),
       (-cos(pi/4), -sin(pi/4), 0),
       (0, -1, 0),
       (cos(pi/4), -sin(pi/4), 0) ]


bodyCfg = [ [ 0.5, 1.0, 0.2, 10.,  (0.2,0.1,0.2) ]     # Box
            ]
# Body specific parameters and state vectors
configured = []
for bc in bodyCfg:
	sx, sy, sz, mass, dc = bc
	rho = mass / (sx*sy*sz) # density of the body
	Ixx, Iyy, Izz, Iww = SymbolicTensor(sx, sy, sz, rho)

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
		  array([0,0,1,0,0,0]),                # q (position)
		  array([0,0,0,0,0,0]),       # qd (velocity)
		  corners,                    # Corners
		  dc ]                        # Drawing color

	configured.append(c)

cube = 0               # Cube(display list)
gnd_texture = 0        # Ground texture
quadric = 0            # OpenGL quadric object
curFrame = 0           # Current frame number
autoPlay = True
drawCoupon = 1
glWidth = int(320*2.5)
glHeight = int(240*2.5)
myChar = 0

KSE = 800             # Serial spring constant
KPE = 0.01            # Parallel spring constant
Kb = 0.05                # Viscosity constant
xRest = 1.5             # Muscle fiber rest length
tau = Kb/(KSE+KPE)      # Time constant for the muscle fiber system
historyMax = 100
accumTerm = [0]*historyMax
def MuscleFiberTension(u, x, xd):
	"""
	u = actuator force
	x = muscle length for this time step
	xd = the rate of muscle length for this timestep
	
	Returns current step's muscle fiber tension (force)
	"""
	global h, KSE, KPE, Kb, xRest, tau, historyMax, accumTerm
	cur = (KSE*u + KPE*KSE*(x-xRest) + Kb*KSE*xd)/Kb
	prev = sum([math.exp(-h*(historyMax-i)/tau) * v for i,v in zip(range(historyMax), accumTerm)])
	accumTerm.pop(0)
	accumTerm.append(cur)
	return cur+prev
	


# Let's go!
main()
