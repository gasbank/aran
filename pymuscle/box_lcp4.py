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
import pygame
from pygame.font import *
from PmBody import *
from PmMuscle import *
from GlobalContext import *

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'
LEFTARROW = 100
RIGHTARROW = 102

seterr(all='raise')

# A general OpenGL initialization function.  Sets all of the initial parameters. 
def InitializeGl (gCon):				# We call this right after our OpenGL window is created.
	glClearColor(gCon.clearColor[0],
	             gCon.clearColor[1],
	             gCon.clearColor[2],
	             1.0)
	glClearDepth(1.0)									# Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LEQUAL)								# The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST)								# Enables Depth Testing
	glShadeModel (GL_FLAT);								# Select Flat Shading (Nice Definition Of Objects)
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST) 	# Really Nice Perspective Calculations
	glEnable (GL_LIGHT0)
	glEnable (GL_LIGHTING)
	glEnable (GL_COLOR_MATERIAL)
	#glShadeModel(GL_SMOOTH)				# Enables Smooth Color Shading
	glEnable(GL_TEXTURE_2D)

	# Turn on wireframe mode
	#glPolygonMode(GL_FRONT, GL_LINE)
	#glPolygonMode(GL_BACK, GL_LINE)

	gndTex = glGenTextures(1)
	assert gndTex > 0
	im = pil.open('ground.png') # Open the ground texture image
	im = im.convert('RGB')
	ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBX", 0, -1)
	assert ix*iy*4 == len(image)
	glBindTexture(GL_TEXTURE_2D, gndTex)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, ix, iy, GL_RGBA, GL_UNSIGNED_BYTE, image );
	gCon.gndTex = gndTex

	gCon.quadric = gluNewQuadric(1)

	pygame.font.init()
	if not pygame.font.get_init():
		print 'Could not render font.'
		sys.exit(0)
	myFont = pygame.font.Font('ARIALN.TTF',30)
	gCon.myChar = []
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
		gCon.myChar.append( (letter, letter_w, letter_h) )
	gCon.myChar = tuple(gCon.myChar)
	gCon.myLw = gCon.myChar[ord('0')][1]
	gCon.myLh = gCon.myChar[ord('0')][2]

	return True

def TextView(winWidth, winHeight):
	glViewport(0, 0, winWidth, winHeight)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	glOrtho(0.0, winWidth - 1.0, 0.0, winHeight - 1.0, -1.0, 1.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	
def Print(gCon, s, x, y):
	s = str(s)
	i = 0
	lx = 0
	length = len(s)
	TextView(gCon.winWidth, gCon.winHeight)
	glPushMatrix()
	while i < length:
		glRasterPos2i(x + lx, y)
		ch = gCon.myChar[ ord( s[i] ) ]
		glDrawPixels(ch[1], ch[2], GL_RGBA, GL_UNSIGNED_BYTE, ch[0])
		lx += ch[1]
		i += 1
	glPopMatrix()

# Reshape The Window When It's Moved Or Resized
def ResizeGlScene(winWidth, winHeight):
	if winHeight == 0:						# Prevent A Divide By Zero If The Window Is Too Small 
		winHeight = 1

	global gCon
	gCon.winWidth  = winWidth
	gCon.winHeight = winHeight

# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def KeyPressed(*args):
	# If escape is pressed, kill everything.
	key = args [0]
	if key == ESCAPE:
		sys.exit ()
	#print 'Key pressed', key

def SpecialKeyPressed(*args):
	# If escape is pressed, kill everything.
	global gCon
	key = args [0]
	if key == LEFTARROW:
		if gCon.curFrame > 0:
			gCon.curFrame = gCon.curFrame - 1
			glutPostRedisplay()
	elif key == RIGHTARROW:
		if gCon.curFrame < gCon.noFrame-3:
			gCon.curFrame = gCon.curFrame + 1
			glutPostRedisplay()
	print 'Special key pressed', key, 'Frame', gCon.curFrame

def Main(gCon):
	# pass arguments to init
	glutInit(sys.argv)

	# Select type of Display mode:   
	#  Double buffer 
	#  RGBA color
	# Alpha components supported 
	# Depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

	glutInitWindowSize(gCon.winWidth, gCon.winHeight)

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
	glutReshapeFunc(ResizeGlScene)

	# Register the function called when the keyboard is pressed.  
	glutKeyboardFunc(KeyPressed)
	glutSpecialFunc(SpecialKeyPressed) # Non-ascii characters

	# We've told Glut the type of window we want, and we've told glut about
	# various functions that we want invoked (idle, resizing, keyboard events).
	# Glut has done the hard work of building up thw windows DC context and 
	# tying in a rendering context, so we are ready to start making immediate mode
	# GL calls.
	# Call to perform inital GL setup (the clear colors, enabling modes
	InitializeGl (gCon)

	# Start Event Processing Engine	
	glutMainLoop()

def Draw():
	FrameMove()
	
	Prerender()
	RenderPerspectiveWindow()
	RenderLegMonitorWindow('LeftAnkle', 'SIDE')
	RenderLegMonitorWindow('LeftAnkle', 'FRONT')
	RenderLegMonitorWindow('RightAnkle', 'SIDE')
	RenderLegMonitorWindow('RightAnkle', 'FRONT')
	RenderSideCameraWindow()
	RenderHud()
	Postrender()

def Prerender():
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)				# // Clear Screen And Depth Buffer
	#print frame, 'err', err, 'p', p, 'Act', activeBodies, 'Inact', inactiveBodies, 'Corners', activeCorners
	
def Postrender():
	
	glutSwapBuffers()

	# Take a shot of this frame
	"""
	glPixelStorei(GL_PACK_ALIGNMENT, 4)
	glPixelStorei(GL_PACK_ROW_LENGTH, 0)
	glPixelStorei(GL_PACK_SKIP_ROWS, 0)
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0)

	pixels = glReadPixels(0, 0, glWidth, glHeight, GL_RGBA, GL_UNSIGNED_BYTE)
	surface = pygame.image.fromstring(pixels, (glWidth,glHeight), 'RGBA', 1)
	pygame.image.save(surface,'/home/johnu/ss/%04d.jpg'%curFrame)
	"""
	
def RenderHud():
	global gCon
	curFrame = gCon.curFrame
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
	Print(gCon, 'Frame ' + str(gCon.curFrame) + ' ' + curPhase, 0, 0)

def RenderLegMonitorWindow(legName, viewDir):
	assert(legName in ['LeftAnkle', 'RightAnkle'])
	assert(viewDir in ['SIDE', 'FRONT'])

	global gCon
	ankleIdx = FindBodyIndex(legName)
	ankleGlobalPos = gCon.configured[ankleIdx].globalPos((0,0,0))
	
	glWidth, glHeight = gCon.winWidth, gCon.winHeight
	perpW, perpH      = gCon.perpW, gCon.perpH
	legW, legH        = int(glWidth-glWidth*perpW)/2, int(glHeight*perpH/2.)
	aspectRatio       = float(legW) / legH
	quadric           = gCon.quadric
	
	
	if legName == 'LeftAnkle':
		vpX, vpY = int(glWidth*perpW), 0
	elif legName == 'RightAnkle':
		vpX, vpY = int(glWidth*perpW), legH
	else:
		raise Exception('Unexpected leg name')
	if viewDir == 'FRONT':
			vpX = vpX + legW
	glViewport(vpX, vpY, legW, legH)
	
	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	glOrtho(-aspectRatio/2., aspectRatio/2., -1./2, 1./2, 1, 1000)
	
	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix
	
	if viewDir == 'SIDE':
		xeye, yeye, zeye = 5, ankleGlobalPos[1], 0.35
		xcenter, ycenter, zcenter = 0, ankleGlobalPos[1], 0.35
	elif viewDir == 'FRONT':
		xeye, yeye, zeye = ankleGlobalPos[0], -100, 0.35
		xcenter, ycenter, zcenter = ankleGlobalPos[0], 0, 0.35
	else:
		raise Exception('What the...')
	xup, yup, zup = 0, 0, 1
	gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);
	
	# Draw ground in cross section
	glPushAttrib(GL_LIGHTING_BIT)
	glDisable(GL_LIGHTING)
	glBindTexture(GL_TEXTURE_2D, gCon.gndTex)
	glColor3f(1, 1, 1)
	tex_repeat=10
	glBegin(GL_QUADS)
	texRep = gCon.gndTexRep
	plane = gCon.planeSize
	if viewDir == 'SIDE':
		gx = 0
		gy = plane
	else:
		gx = plane
		gy = 0
	glTexCoord2f(0, 0);               glVertex3f( gx, gy, 0)
	glTexCoord2f(texRep*2, 0);        glVertex3f(-gx,-gy, 0)
	glTexCoord2f(texRep*2, texRep*2); glVertex3f(-gx,-gy,-1)
	glTexCoord2f(0, texRep*2);        glVertex3f( gx, gy,-1)
	glEnd()
	glPopAttrib()
	
	DrawBiped(drawAxis=False, wireframe=True)
	DrawBipedFibers(drawAsLine=True)
	DrawBipedContactPoints()

def DrawBiped(drawAxis, wireframe):
	global gCon
	nb = len(gCon.configured)
	for i, bd in zip(range(nb), gCon.configured):
		mass, size, inertia = bd.mass, bd.boxsize, bd.I
		q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
		sx, sy, sz = size

		glPushMatrix()
		glTranslatef(q[0], q[1], q[2])
		A_homo = identity(4)
		A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
		A_homo[0:3,0:3] = A
		glMultMatrixd(A_homo.T.flatten())
		# box(body) frame indicator
		if drawAxis:
			RenderAxis(0.2)
		glScalef(sx, sy, sz)
		glColor3f(dc[0],dc[1],dc[2])
				
		if i >= 0:
			# Ankles and toes are rendered as solid cube
			if wireframe:
				glutWireCube(1.)
			else:
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
	
def RenderSideCameraWindow():
	global gCon
	glWidth, glHeight = gCon.winWidth, gCon.winHeight
	perpW, perpH      = gCon.perpW, gCon.perpH
	sideW, sideH      = glWidth, glHeight-int(glHeight*perpH)
	aspectRatio       = float(sideW) / sideH
	quadric           = gCon.quadric
	
	glViewport(0, int(glHeight*perpH), sideW, sideH)		# Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	glOrtho(-aspectRatio, aspectRatio, -1, 1, 1, 1000)
	#glOrtho(-15.5, 15.5, -15.5, 15.5, 1, 1000)
	
	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix

	xeye, yeye, zeye = 10, -5, 0.8
	xcenter, ycenter, zcenter = 0, -5, 0.8
	xup, yup, zup = 0, 0, 1
	gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);
	
	# Render the fancy global(inertial) coordinates
	RenderFancyGlobalAxis(quadric, 0.7/2, 0.3/2, 0.025)
	
	# Draw ground in cross section
	glPushAttrib(GL_LIGHTING_BIT)
	glDisable(GL_LIGHTING)
	glBindTexture(GL_TEXTURE_2D, gCon.gndTex)
	glColor3f(1, 1, 1)
	texRep = gCon.gndTexRep
	plane = gCon.planeSize
	
	glBegin(GL_QUADS)
	glTexCoord2f(0, 0);            glVertex3f(0, plane, 0)
	glTexCoord2f(texRep, 0);       glVertex3f(0,-plane, 0)
	glTexCoord2f(texRep, texRep);  glVertex3f(0,-plane,-1)
	glTexCoord2f(0, texRep);       glVertex3f(0, plane,-1)
	glEnd()
	glPopAttrib()
	
	DrawBiped(drawAxis=False, wireframe=False)
	DrawBipedFibers()
	DrawBipedContactPoints()
	DrawBipedContactForces()
	
def DrawBipedFibers(drawAsLine=False):
	global gCon
	# Draw muscle/ligament fibers
	for m in gCon.fibers:
		orgBodyIdx = FindBodyIndex(m.orgBody)
		insBodyIdx = FindBodyIndex(m.insBody)
		borg = gCon.configured[orgBodyIdx]
		bins = gCon.configured[insBodyIdx]
		
		localorg = array([b/2. * p for b,p in zip(borg.boxsize, m.orgPos)])
		localins = array([b/2. * p for b,p in zip(bins.boxsize, m.insPos)])
		
		globalorg = borg.globalPos(localorg)
		globalins = bins.globalPos(localins)
		
		if m.mType == 'MUSCLE':
			dc = (1,0,0)
			radius1 = 0.015
			radius2 = 0.020
		else:
			dc = (0.1,0.1,0.1)
			radius1 = 0.010
			radius2 = 0.010
		if drawAsLine:
			DrawMuscleFiber3(globalorg, globalins, 2.0, dc)
		else:
			DrawMuscleFiber2(gCon.quadric, globalorg, globalins,
			                 radius1, radius2, dc)
		
def DrawBipedContactPoints():
	global gCon
	glColor3f(0.1, 0.2, 0.9)
	for acp in gCon.activeCornerPoints:
		glPushMatrix()
		glTranslated(acp[0], acp[1], acp[2])
		glutSolidSphere(0.035, 8, 8)
		glPopMatrix()
		
def DrawBipedContactForces():
	global gCon
	glColor3f(1.0,0.7,0.5) # Contact force arrow color
	for cf, acp in zip(gCon.contactForces, gCon.activeCornerPoints):
		fricdirn, friclen = cf
		rotaxis = cross([0,0,1.], fricdirn)
		rotangle = acos(dot(fricdirn,[0,0,1.]))
		
		glPushMatrix()
		glTranslatef(acp[0], acp[1], acp[2])
		glRotatef(rotangle/math.pi*180,rotaxis[0],rotaxis[1],rotaxis[2])
		RenderArrow(gCon.quadric, friclen*0.8, friclen*0.2, 0.015)
		glPopMatrix()

def RenderPerspectiveWindow():
	global gCon
	glWidth, glHeight = gCon.winWidth, gCon.winHeight
	perpW, perpH      = gCon.perpW, gCon.perpW
	quadric           = gCon.quadric
	
	glViewport(0, 0, int(glWidth*perpW), int(glHeight*perpH))		# Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION)			# // Select The Projection Matrix
	glLoadIdentity()					# // Reset The Projection Matrix
	gluPerspective(45.0, float(glWidth)/float(glHeight), 1, 1000.0)
	
	glMatrixMode (GL_MODELVIEW);		# // Select The Modelview Matrix
	glLoadIdentity ();					# // Reset The Modelview Matrix

	xeye, yeye, zeye = 10, -5, 2.5
	xcenter, ycenter, zcenter = 2.90, -4, 0.5
	xup, yup, zup = 0, 0, 1
	gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

	# Plane
	gnd_texture = gCon.gndTex
	
	glDisable(GL_LIGHTING)
	glBindTexture(GL_TEXTURE_2D, gnd_texture)
	glColor3f(1,1,1)
	texRep = gCon.gndTexRep
	plane = gCon.planeSize
	
	glBegin(GL_QUADS)
	glTexCoord2f(0, 0);            glVertex3f(-plane, -plane, 0)
	glTexCoord2f(texRep, 0);       glVertex3f( plane, -plane, 0)
	glTexCoord2f(texRep, texRep);  glVertex3f( plane,  plane, 0)
	glTexCoord2f(0, texRep);       glVertex3f(-plane,  plane, 0)
	glEnd()
	glEnable(GL_LIGHTING)
	# Render the fancy global(inertial) coordinates
	RenderFancyGlobalAxis(quadric, 0.7/2, 0.3/2, 0.025)

	DrawBiped(drawAxis=True, wireframe=False)
	DrawBipedFibers()
	DrawBipedContactPoints()
	DrawBipedContactForces()

def FrameMove():
	global gCon
	# Total number of rigid bodies
	nb = len(gCon.configured)
	h  = gCon.h
	mu = gCon.mu
	
	
	### TRAJECTORY INPUT ###
	for k in range(nb):
		gCon.configured[k].q  = gCon.q_data[gCon.curFrame][k]
		gCon.configured[k].qd = gCon.qd_data[gCon.curFrame][k]
	
	
	# 'activeCorners' has tuples.
	# (body index, corner index)
	activeCorners = []
	activeBodies = set()
	activeCornerPoints = []
	for k in range(nb):
		# Check all eight corners
		bd = gCon.configured[k]
		mass, size, inertia = bd.mass, bd.boxsize, bd.I
		q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
		sx, sy, sz = size
		
		for i in range(8):
			A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
			c = q[0:3] + dot(A, corners[i])
			if c[2] < gCon.alpha0:
				activeCorners.append( (k, i) )
				activeBodies.add(k)
				activeCornerPoints.append(c)
	gCon.activeCornerPoints = activeCornerPoints
	
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
		bd = gCon.configured[k]
		mass, size, inertia = bd.mass, bd.boxsize, bd.I
		q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
		sx, sy, sz = size

		Minv_k = SymbolicMinv(q + h*qd, inertia)
		Cqd_k = SymbolicCqd(q + h*qd/2, qd, inertia)

		# Add gravitational force to the Coriolis term
		fg = SymbolicForce(q + h*qd/2, (0, 0, -9.81 * mass), (0, 0, 0))
		Cqd_k = Cqd_k + fg
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
			bd = gCon.configured[kp]
			mass, size, inertia = bd.mass, bd.boxsize, bd.I
			q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
			sx, sy, sz = size
			k = activeBodies.index(kp)

			# Which one is right?
			#N[6*k:6*(k+1), i] = SymbolicForce(q + h*qd/2, (0, 0, 1), corners[cp])
			N[6*k:6*(k+1), i] = SymbolicPenetration(q + h*qd/2, corners[cp])


			Di = zeros((6*nba,8)) # Eight basis for tangential forces
			for j in range(8):
				Di[6*k:6*(k+1), j] = SymbolicForce(q + h*qd/2, gCon.di[j], corners[cp])
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

		beta_reshaped = beta.reshape(p,8)
		gCon.contactForces = []
		for i, acp, cn_i, beta_i, ac_i in zip(range(p),
		                                      activeCornerPoints,
		                                      cn,
		                                      beta_reshaped,
		                                      activeCorners):
			bodyidx = activeBodies.index(ac_i[0])
			fric = dot(D[:, 8*i:8*(i+1)], beta_i)[6*bodyidx:6*(bodyidx+1)]
			fricdir = array([fric[0]*gCon.cfScaleFactor,
		                 fric[1]*gCon.cfScaleFactor,
		                 cn_i*gCon.cfScaleFactor])
			friclen = linalg.norm(fricdir)
			if friclen > 0:
				fricdirn = fricdir / friclen
				cf = (fricdirn, friclen)
				gCon.contactForces.append(cf)

		for k in activeBodies:
			kk = activeBodies.index(k)
			gCon.configured[k].q  = Q_a_next[6*kk:6*(kk+1)]
			gCon.configured[k].qd = Qd_a_next[6*kk:6*(kk+1)]


	for k in inactiveBodies:
		# No contact point
		kk = inactiveBodies.index(k)
		Qd_i_next = dot(Minv_i, h*Cqd_i) + Qd_i
		Q_i_next  = h * Qd_i_next + Q_i
		gCon.configured[k].q  = Q_i_next[6*kk:6*(kk+1)]
		gCon.configured[k].qd = Qd_i_next[6*kk:6*(kk+1)]
		z0 = 0

	"""
	### TRAJECTORY INPUT ###
	for k in range(nb):
		gCon.configured[k].q  = gCon.q_data[gCon.curFrame][k]
		gCon.configured[k].qd = gCon.qd_data[gCon.curFrame][k]
	"""
	
	if gCon.autoPlay:
		"""
		gCon.curFrame = gCon.curFrame + 1
		if gCon.curFrame >= gCon.noFrame-2:
			gCon.curFrame = 0
		"""
		gCon.curFrame = gCon.curFrame + 1

"""
def ang_vel(q, v):
	x, y, z, phi, theta, psi = q
	xd, yd, zd, phid, thetad, psid = v
	return array([phid*sin(theta)*sin(psi)+thetad*cos(psi),
		          phid*sin(theta)*cos(psi)-thetad*sin(psi),
		          phid*cos(theta)+psid])
"""

def FindBodyIndex(name):
	global gCon
	nb = len(gCon.bodyList)
	for i, n in zip(range(nb), gCon.bodyList):
		bodyName, pBodyName = n
		if bodyName == name:
			return i
	raise Exception('Wrong body name!')

################################################################################
################################################################################
################################################################################

# Let's go!
gCon = GlobalContext()
Main(gCon)
