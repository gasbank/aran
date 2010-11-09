#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A rigid box connected with a muscle fiber
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
import cPickle

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
	#glShadeModel(GL_SMOOTH)				# Enables Smooth Color Shading
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
	elif key == 't':
		global Ten
		print Ten
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
	global oRb, size, mass, omega, rcm, vcm, muscleFixPos, muscleAttachedPosLocal
	
	
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

	xeye, yeye, zeye = 8, -8, 8
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


	glPushMatrix()
	A_homo = identity(4)
	A_homo[0:3,0:3] = oRb
	A_homo[0:3,3] = rcm
	glMultMatrixd(A_homo.T.flatten())
	# box(body) frame indicator
	RenderAxis(0.2)
	glScalef(size[0], size[1], size[2])
	glColor3f(0.9, 0.2, 0.1)
	glutSolidCube(1.)
	glPopMatrix()
	
	glPushMatrix()
	glTranslated(muscleFixPos[0], muscleFixPos[1], muscleFixPos[2])
	glutSolidSphere(0.05, 8, 8)
	glPopMatrix()
	
	
	
	M = SymbolicM_Global(oRb, size[0], size[1], size[2], mass)
	Minv = linalg.inv(M)
	C = SymbolicC_Global(oRb, omega, size[0], size[1], size[2], mass)
	ext=0
	#ext = ext + SymbolicForce_Manual2(oRb, (0, 0, 9.81*mass/2), (0, 0.2, 0))
	#ext = ext + SymbolicForce_Manual2(oRb, (0, 0, 9.81*mass/2), (0, -0.2, 0))
	ext = ext + SymbolicForce_Manual2(oRb, (0, 0, -9.81*mass), (0, 0, 0))
	
	# Add muscle fiber tension
	muscleAttachedPosGlobal = rcm + dot(oRb, muscleAttachedPosLocal)
	muscleDir = muscleFixPos - muscleAttachedPosGlobal
	muscleLen = linalg.norm(muscleDir)
	muscleDir = muscleDir/muscleLen # Normalize direction

	muscleAttachedVel = vcm + cross(omega, muscleAttachedPosLocal)
	muscleLenRate = dot(muscleDir, muscleAttachedVel)
	global Ten, xRest
	#Ten = MuscleFiberTension(0, muscleLen, muscleLenRate, Ten)
	Ten = -200.5*(xRest-muscleLen)
	#print curFrame, q[3],q[4],q[5], tension, omega, muscleLenRate
	tensionG = SymbolicForce_Manual2(oRb, tuple(muscleDir*Ten), tuple(muscleAttachedPosLocal))
	ext = ext + tensionG
	#print Ten, xRest-muscleLen, muscleLenRate
	
	acc = dot(Minv, ext - C)
	linacc = acc[0:3]
	angacc = acc[3:6]
	
	rcm = rcm + h*vcm
	vcm = vcm + h*linacc
	oRb = oRb + h*dot(cross_op_mat(omega),oRb)
	omega = omega + h*angacc
	#print rcm
	
	#omega = omega*0.9999
	#vcm = vcm*0.9995

	oRb = OrthonormalizedOfOrientation(oRb)

	############################################################################
	DrawMuscleFiber(quadric,
	                muscleAttachedPosGlobal,
	                muscleDir,
	                muscleLen,
	                0.05,
	                0.08,
	                (1.0,0.1,0.2) )
	glutSwapBuffers()
	############################################################################

################################################################################

cube = 0               # Cube(display list)
gnd_texture = 0        # Ground texture
quadric = 0            # OpenGL quadric object
curFrame = 0           # Current frame number


# Determine body constants
size = array([0.3, 0.5, 0.2])
s1, s2, s3 = size
mass = 2.
Hcm = diag([mass*(s2*s2+s3*s3)/12, mass*(s1*s1+s3*s3)/12, mass*(s1*s1+s2*s2)/12])
Hcminv = linalg.inv(Hcm)
h = 0.001
# Determine initial conditions
rcm = array([0.,0,0.5])      # Initial linear position
vcm = array([0.,0,0])      # Initial linear velocity
oRb = identity(3)          # Initial orientation
Lcm = array([0.,0,0])      # Initial angular momentum
# Compute initial auxiliary quantities
HcmX = dot(dot(oRb, Hcminv), oRb.T)
omega = dot(HcmX, Lcm)

muscleFixPos = array([0.,0,3])
#muscleAttachedPosLocal = array(size/2)
muscleAttachedPosLocal = zeros(3)

glWidth = 800
glHeight = 600


KSE = 0.51             # Serial spring constant
KPE = 0.01            # Parallel spring constant
Kb = 0.0001                # Viscosity constant
xRest = 1.5             # Muscle fiber rest length
tau = Kb/(KSE+KPE)      # Time constant for the muscle fiber system
Ten = 0

def MuscleFiberTension(u, x, xd, Ten):
	"""
	u = actuator force
	x = muscle length for this time step
	xd = the rate of muscle length for this timestep
	Ten = current tension
	
	Returns current step's muscle fiber tension (force)
	"""
	global h, KSE, KPE, Kb, xRest
	Tend = KSE/Kb*(KPE*(x-xRest)+Kb*xd-(1+KPE/KSE)*Ten+u)
	return Ten+h*Tend

# Let's go!
main()
