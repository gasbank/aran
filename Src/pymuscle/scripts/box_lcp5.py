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
import pygame
from pygame.font import *
import RigidBody
from PmMuscle import *
from GlobalContext import *
import ctypes as ct
from BipedParameter import *
import box_lcp3_exp_multibody as BLEM

from dRdv_real import GeneralizedForce, QuatdFromV
import copy
import matplotlib.pyplot as pit
import cPickle
from WriteSimcoreConfFile import *

# A general OpenGL initialization function.  Sets all of the initial parameters. 
def InitializeGl (gCon):                # We call this right after our OpenGL window is created.
    glClearColor(gCon.clearColor[0],
                 gCon.clearColor[1],
                 gCon.clearColor[2],
                 1.0)
    glClearDepth(1.0)                                   # Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LEQUAL)                              # The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST)                             # Enables Depth Testing
    glShadeModel (GL_FLAT);                             # Select Flat Shading (Nice Definition Of Objects)
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)  # Really Nice Perspective Calculations
    glEnable (GL_LIGHT0)
    glEnable (GL_LIGHTING)

    noAmbient = [0.0, 0.0, 0.0, 1.0]
    whiteDiffuse = [1.0, 1.0, 1.0, 1.0]
    """
    Directional light source (w = 0)
    The light source is at an infinite distance,
    all the ray are parallel and have the direction (x, y, z).
    """
    position = [0.2, -1.0, 1.0, 0.0]

    glLightfv(GL_LIGHT0, GL_AMBIENT, noAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    #glEnable (GL_COLOR_MATERIAL)
    #glShadeModel(GL_SMOOTH)                # Enables Smooth Color Shading
    glEnable(GL_TEXTURE_2D)
    glEnable(GL_NORMALIZE)
    # Turn on wireframe mode
    #glPolygonMode(GL_FRONT, GL_LINE)
    #glPolygonMode(GL_BACK, GL_LINE)

    gndTex = glGenTextures(1)
    assert gndTex > 0
    im = pil.open(gWorkDir + 'ground.png') # Open the ground texture image
    im = im.convert('RGB')
    ix, iy, image = im.size[0], im.size[1], im.tostring("raw", "RGBX", 0, -1)
    assert ix * iy * 4 == len(image)
    glBindTexture(GL_TEXTURE_2D, gndTex)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, ix, iy, GL_RGBA, GL_UNSIGNED_BYTE, image);
    gCon.gndTex = gndTex

    gCon.quadric = gluNewQuadric(1)

    pygame.font.init()
    if not pygame.font.get_init():
        print 'Could not render font.'
        sys.exit(0)
    myFont = pygame.font.Font(gWorkDir + 'ARIALN.TTF', 30)
    gCon.myChar = []
    for c in range(256):
        s = chr(c)
        try:
            letter_render = myFont.render(s, 1, (255, 255, 255), (0, 0, 0))
            letter = pygame.image.tostring(letter_render, 'RGBA', 1)
            letter_w, letter_h = letter_render.get_size()
        except:
            letter = None
            letter_w = 0
            letter_h = 0
        gCon.myChar.append((letter, letter_w, letter_h))
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
        ch = gCon.myChar[ ord(s[i]) ]
        glDrawPixels(ch[1], ch[2], GL_RGBA, GL_UNSIGNED_BYTE, ch[0])
        lx += ch[1]
        i += 1
    glPopMatrix()

# Reshape The Window When It's Moved Or Resized
def ResizeGlScene(winWidth, winHeight):
    if winHeight == 0:                      # Prevent A Divide By Zero If The Window Is Too Small 
        winHeight = 1

    global gCon
    gCon.winWidth = winWidth
    gCon.winHeight = winHeight

# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def KeyPressed(*args):
    # If escape is pressed, kill everything.
    key = args [0]
    if key == ESCAPE:
        sys.exit ()
    elif key == 'k':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[1] += 0.01
    elif key == 'l':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[1] -= 0.01
    elif key == 'd':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[4] += 0.01
        gCon.body0[idx].q[3:7] = quat_normalize(gCon.body0[idx].q[3:7])
    elif key == 'f':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[4] -= 0.01
        gCon.body0[idx].q[3:7] = quat_normalize(gCon.body0[idx].q[3:7])
    elif key == 'j':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[2] += 0.01
    elif key == 'n':
        idx = gCon.findBodyIndex('calfL')
        gCon.body0[idx].q[2] -= 0.01
    elif key == 's':
        stateFile = open('box_lcp5_lastState.dat','w')
        state = (gCon.body, gCon.fibers)
        cPickle.dump(state, stateFile)
        stateFile.close()
        print 'State written.'
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
        if gCon.curFrame < gCon.noFrame - 3:
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
    global gFrame, gUseOpenGl
    assert gUseOpenGl == True

    FrameMove()

    #print gCon.body[0].q, gCon.body[0].qd
    #print gCon.body[1].q, gCon.body[1].qd
    '''
    if gFrame == 375:
        sys.exit(0)
    '''


    Prerender()
    RenderPerspectiveWindow()
    if gRunMode == 'MOCAP':
        RenderLegMonitorWindow('LeftAnkle', 'SIDE')
        RenderLegMonitorWindow('LeftAnkle', 'FRONT')
        RenderLegMonitorWindow('RightAnkle', 'SIDE')
        RenderLegMonitorWindow('RightAnkle', 'FRONT')
    elif gRunMode in ['IMPINT', 'CONTROL']:
        RenderFrontCameraWindow()
        #RenderTopCameraWindow()
    RenderSideCameraWindow()
    RenderHud()
    Postrender()

    gFrame += 1
    return

def Prerender():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)              # // Clear Screen And Depth Buffer
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

def RenderFrontCameraWindow():
    global gCon

    glWidth, glHeight = gCon.winWidth, gCon.winHeight
    perpW, perpH = gCon.perpW, gCon.perpH
    frontW, frontH = int(glWidth - glWidth * perpW), int(glHeight * perpH)
    aspectRatio = float(frontW) / frontH
    quadric = gCon.quadric

    vpX, vpY = int(glWidth * perpW), 0
    glViewport(vpX, vpY, frontW, frontH)        # Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    glOrtho(-aspectRatio, aspectRatio, -1, 1, 1, 1000)

    glMatrixMode (GL_MODELVIEW);        # // Select The Modelview Matrix
    glLoadIdentity ();                  # // Reset The Modelview Matrix

    # Point Of Interest
    poi = gCon.findBodyIndex2(poiList)
    poiPos = array(gCon.body[poi].q[0:3])

    xeye, yeye, zeye = poiPos + array([0, -10, -0.5])
    xcenter, ycenter, zcenter = poiPos + array([0, 0, -0.5])
    xup, yup, zup = 0, 0, 1
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

    glBegin(GL_LINES)
    glVertex(-10,0,0)
    glVertex(10,0,0)
    glEnd()

    # Draw ground in cross section
    glPushAttrib(GL_LIGHTING_BIT)
    glDisable(GL_LIGHTING)
    glBindTexture(GL_TEXTURE_2D, gCon.gndTex)
    glColor3f(1, 1, 1)
    texRep = gCon.gndTexRep
    plane = gCon.planeSize

    glBegin(GL_QUADS)
    glTexCoord2f(0, 0);            glVertex3f(0, plane, 0)
    glTexCoord2f(texRep, 0);       glVertex3f(0, -plane, 0)
    glTexCoord2f(texRep, texRep);  glVertex3f(0, -plane, -1)
    glTexCoord2f(0, texRep);       glVertex3f(0, plane, -1)
    glEnd()
    glPopAttrib()

    DrawBiped(gCon.body, drawAxis=True, wireframe=True)
    #DrawBiped(gCon.body0, drawAxis=True, wireframe=True)
    DrawBipedFibers()
    DrawBipedContactPoints()
    DrawBipedContactForces()

def RenderLegMonitorWindow(legName, viewDir):
    assert(legName in ['LeftAnkle', 'RightAnkle'])
    assert(viewDir in ['SIDE', 'FRONT'])

    global gCon
    ankleIdx = gCon.findBodyIndex(legName)
    ankleGlobalPos = gCon.body[ankleIdx].globalPos((0, 0, 0))

    glWidth, glHeight = gCon.winWidth, gCon.winHeight
    perpW, perpH = gCon.perpW, gCon.perpH
    legW, legH = int(glWidth - glWidth * perpW) / 2, int(glHeight * perpH / 2.)
    aspectRatio = float(legW) / legH
    quadric = gCon.quadric


    if legName == 'LeftAnkle':
        vpX, vpY = int(glWidth * perpW), 0
    elif legName == 'RightAnkle':
        vpX, vpY = int(glWidth * perpW), legH
    else:
        raise Exception('Unexpected leg name')
    if viewDir == 'FRONT':
        vpX = vpX + legW
    glViewport(vpX, vpY, legW, legH)

    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    glOrtho(-aspectRatio / 2., aspectRatio / 2., -1. / 2, 1. / 2, 1, 1000)

    glMatrixMode (GL_MODELVIEW);        # // Select The Modelview Matrix
    glLoadIdentity ();                  # // Reset The Modelview Matrix

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
    tex_repeat = 10
    glBegin(GL_QUADS)
    texRep = gCon.gndTexRep
    plane = gCon.planeSize
    if viewDir == 'SIDE':
        gx = 0
        gy = plane
    else:
        gx = plane
        gy = 0
    glTexCoord2f(0, 0);               glVertex3f(gx, gy, 0)
    glTexCoord2f(texRep * 2, 0);        glVertex3f(-gx, -gy, 0)
    glTexCoord2f(texRep * 2, texRep * 2); glVertex3f(-gx, -gy, -1)
    glTexCoord2f(0, texRep * 2);        glVertex3f(gx, gy, -1)
    glEnd()
    glPopAttrib()

    DrawBiped(drawAxis=False, wireframe=True)
    DrawBipedFibers(drawAsLine=True)
    DrawBipedContactPoints()

def DrawBiped(drawBody, drawAxis, wireframe):
    global gCon
    nb = len(drawBody)
    for i, bd in zip(range(nb), drawBody):
        mass, size, inertia = bd.mass, bd.boxsize, bd.I
        q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
        sx, sy, sz = size

        glPushMatrix()
        glTranslatef(q[0], q[1], q[2])
        A_homo = identity(4)
        if gCon.rotParam == 'EULER_XYZ':
            A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
        elif gCon.rotParam == 'EULER_ZXZ':
            A = RotationMatrixFromEulerAngles_zxz(q[3], q[4], q[5])
        elif gCon.rotParam == 'QUAT_WFIRST':
            A = RotationMatrixFromQuaternion(q[3], q[4], q[5], q[6])
        else:
            raise Exception('unknown rotation parameterization.')
        A_homo[0:3, 0:3] = A
        glMultMatrixd(A_homo.T.flatten())
        # box(body) frame indicator
        if drawAxis:
            RenderAxis(0.2)
        glScalef(sx, sy, sz)
        glColor3f(dc[0], dc[1], dc[2])

        if i >= 0:
            # Ankles and toes are rendered as solid cube
            if wireframe:
                glutWireCube(1.)
            else:
                glutSolidCube(1.)
        else:
            glRotatef(-90, 1, 0, 0)
            glTranslatef(0, 0, -0.5)

            # Bottom cap
            glPushMatrix()
            glRotatef(180, 1, 0, 0) # Flip the bottom-side cap to invert the normal
            gluDisk(gCon.quadric, 0, 0.5, 6, 1)
            glPopMatrix()

            gluCylinder(gCon.quadric, 0.5, 0.5, 1.0, 6, 8);

            # Top cap
            glTranslate(0, 0, 1)
            gluDisk(gCon.quadric, 0, 0.5, 6, 1)
        glPopMatrix()

def RenderTopCameraWindow():        
    global gCon
    glWidth, glHeight = gCon.winWidth, gCon.winHeight
    perpW, perpH = gCon.perpW, gCon.perpH
    sideW, sideH = glWidth, glHeight - int(glHeight * perpH)
    if gRunMode == 'IMPINT':
        sideW /= 2.
    aspectRatio = float(sideW) / sideH
    quadric = gCon.quadric

    glViewport(int(sideW), int(glHeight * perpH), int(sideW), int(sideH))       # Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    zoomLevel = 3.0
    glOrtho(-aspectRatio/zoomLevel, aspectRatio/zoomLevel, -1./zoomLevel, 1./zoomLevel, 1, 1000)
    #glOrtho(-15.5, 15.5, -15.5, 15.5, 1, 1000)

    glMatrixMode (GL_MODELVIEW);        # // Select The Modelview Matrix
    glLoadIdentity ();                  # // Reset The Modelview Matrix

    # Point Of Interest
    poi = gCon.findBodyIndex2(poiList)
    poiPos = array(gCon.body[poi].q[0:3])

    xeye, yeye, zeye = poiPos + array([0, 0, 10])
    xcenter, ycenter, zcenter = poiPos + array([0, 0, 0])
    xup, yup, zup = 0, 1, 0
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

    '''
    xeye, yeye, zeye = 10, -3, 0.8
    xcenter, ycenter, zcenter = 0, -5, 0.8
    xup, yup, zup = 0, 0, 1
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);
    '''

    # Draw ground in cross section
    glPushAttrib(GL_LIGHTING_BIT)
    glDisable(GL_LIGHTING)
    glBindTexture(GL_TEXTURE_2D, gCon.gndTex)
    glColor3f(1, 1, 1)
    texRep = gCon.gndTexRep
    plane = gCon.planeSize

    glBegin(GL_QUADS)
    glTexCoord2f(0, 0);            glVertex3f(0, plane, 0)
    glTexCoord2f(texRep, 0);       glVertex3f(0, -plane, 0)
    glTexCoord2f(texRep, texRep);  glVertex3f(0, -plane, -1)
    glTexCoord2f(0, texRep);       glVertex3f(0, plane, -1)
    glEnd()
    glPopAttrib()

    DrawBiped(gCon.body, drawAxis=True, wireframe=True)
    DrawBiped(gCon.body0, drawAxis=True, wireframe=True)
    DrawBipedFibers()
    DrawBipedContactPoints()
    DrawBipedContactForces()

def RenderSideCameraWindow():
    global gCon
    glWidth, glHeight = gCon.winWidth, gCon.winHeight
    perpW, perpH = gCon.perpW, gCon.perpH
    sideW, sideH = glWidth, glHeight - int(glHeight * perpH)
    if gRunMode == 'IMPINT':
        sideW /= 2.
    aspectRatio = float(sideW) / sideH
    quadric = gCon.quadric

    glViewport(0, int(glHeight * perpH), int(sideW), int(sideH))        # Reset The Current Viewport And Perspective Transformation

    zoomLevel = 2.0
    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    glOrtho(-aspectRatio/zoomLevel, aspectRatio/zoomLevel, -1/zoomLevel, 1/zoomLevel, 1, 1000)
    #glOrtho(-15.5, 15.5, -15.5, 15.5, 1, 1000)

    glMatrixMode (GL_MODELVIEW);        # // Select The Modelview Matrix
    glLoadIdentity ();                  # // Reset The Modelview Matrix

    # Point Of Interest
    poi = gCon.findBodyIndex2(poiList)
    poiPos = array(gCon.body[poi].q[0:3])

    xeye, yeye, zeye = poiPos + array([3, 0, 0])
    xcenter, ycenter, zcenter = poiPos + array([0, 0, 0])
    xup, yup, zup = 0, 0, 1
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

    '''
    xeye, yeye, zeye = 10, -3, 0.8
    xcenter, ycenter, zcenter = 0, -5, 0.8
    xup, yup, zup = 0, 0, 1
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);
    '''

    # Render the fancy global(inertial) coordinates
    #RenderFancyGlobalAxis(quadric, 0.7 / 2, 0.3 / 2, 0.025)

    # Draw ground in cross section
    glPushAttrib(GL_LIGHTING_BIT)
    glDisable(GL_LIGHTING)
    glBindTexture(GL_TEXTURE_2D, gCon.gndTex)
    glColor3f(1, 1, 1)
    texRep = gCon.gndTexRep
    plane = gCon.planeSize

    glBegin(GL_QUADS)
    glTexCoord2f(0, 0);            glVertex3f(0, plane, 0)
    glTexCoord2f(texRep, 0);       glVertex3f(0, -plane, 0)
    glTexCoord2f(texRep, texRep);  glVertex3f(0, -plane, -1)
    glTexCoord2f(0, texRep);       glVertex3f(0, plane, -1)
    glEnd()
    glPopAttrib()

    DrawBiped(gCon.body, drawAxis=True, wireframe=True)
    #DrawBiped(gCon.body0, drawAxis=True, wireframe=True)
    DrawBipedFibers()
    DrawBipedContactPoints()
    DrawBipedContactForces()

def DrawBipedFibers(drawAsLine=False):
    global gCon
    # Draw muscle/ligament fibers
    for m in gCon.fibers:
        orgBodyIdx = gCon.findBodyIndex(m.orgBody)
        insBodyIdx = gCon.findBodyIndex(m.insBody)
        borg = gCon.body[orgBodyIdx]
        bins = gCon.body[insBodyIdx]

        if m.bAttachedPosNormalized:
            localorg = array([b / 2. * p for b, p in zip(borg.boxsize, m.orgPos)])
            localins = array([b / 2. * p for b, p in zip(bins.boxsize, m.insPos)])
        else:
            localorg = m.orgPos
            localins = m.insPos

        globalorg = borg.globalPos(localorg)
        globalins = bins.globalPos(localins)

        if m.mType == 'MUSCLE':
            dc = (1, 0, 0)
            radius1 = 0.015
            radius2 = 0.020
        else:
            dc = (0.1, 0.1, 0.1)
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
    if hasattr(gCon, 'activeCornerPoints'):
        for acp in gCon.activeCornerPoints:
            glPushMatrix()
            glTranslated(acp[0], acp[1], acp[2])
            glutSolidSphere(0.035, 8, 8)
            glPopMatrix()

def DrawBipedContactForces():
    global gCon
    glColor3f(1.0, 0.7, 0.5) # Contact force arrow color
    if hasattr(gCon, 'contactForces') and hasattr(gCon, 'activeCornerPoints'):
        for cf, acp in zip(gCon.contactForces, gCon.activeCornerPoints):
            fricdirn, friclen = cf
            rotaxis = cross([0, 0, 1.], fricdirn)
            rotangle = acos(dot(fricdirn, [0, 0, 1.]))

            glPushMatrix()
            glTranslatef(acp[0], acp[1], acp[2])
            glRotatef(rotangle / math.pi * 180, rotaxis[0], rotaxis[1], rotaxis[2])
            RenderArrow(gCon.quadric, friclen * 0.8, friclen * 0.2, 0.015)
            glPopMatrix()

    for bodyk in gCon.body:
        if hasattr(bodyk, 'contactPoints'):
            wc = bodyk.getCorners_WC()
            for cp, cf in zip(bodyk.contactPoints, bodyk.cf):
                glColor3f(0.9,0.2,0.2) # Contact point indicator color
                glPushMatrix()
                glTranslate(wc[cp][0], wc[cp][1], wc[cp][2])
                glutSolidSphere(0.02,8,8)
                glPopMatrix()
                glColor3f(0.9,0.2,0.2) # Contact force vector indicator color
                glBegin(GL_LINES)
                glVertex(wc[cp][0]        , wc[cp][1]        , wc[cp][2]        )
                glVertex(wc[cp][0] + cf[0], wc[cp][1] + cf[1], wc[cp][2] + cf[2])
                glEnd()

def RenderPerspectiveWindow():
    global gCon
    glWidth, glHeight = gCon.winWidth, gCon.winHeight
    perpW, perpH = gCon.perpW, gCon.perpH
    quadric = gCon.quadric

    glViewport(0, 0, int(glWidth * perpW), int(glHeight * perpH))       # Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    gluPerspective(45.0, float(glWidth * perpW) / float(glHeight * perpH), 1, 1000.0)

    glMatrixMode (GL_MODELVIEW);        # // Select The Modelview Matrix
    glLoadIdentity ();                  # // Reset The Modelview Matrix

    # Point Of Interest
    poi = gCon.findBodyIndex2(poiList)
    poiPos = array(gCon.body[poi].q[0:3])

    xeye, yeye, zeye = poiPos + array([2.5, -2.5, 0.3])
    xcenter, ycenter, zcenter = poiPos + array([0, 0, -0.5])
    xup, yup, zup = 0, 0, 1
    gluLookAt(xeye, yeye, zeye, xcenter, ycenter, zcenter, xup, yup, zup);

    # Plane
    gnd_texture = gCon.gndTex

    glPushAttrib(GL_LIGHTING_BIT)
    glDisable(GL_LIGHTING)
    glBindTexture(GL_TEXTURE_2D, gnd_texture)
    glColor3f(1, 1, 1)
    texRep = gCon.gndTexRep
    plane = gCon.planeSize

    glBegin(GL_QUADS)
    glTexCoord2f(0, 0);            glVertex3f(-plane, -plane, 0)
    glTexCoord2f(texRep, 0);       glVertex3f(plane, -plane, 0)
    glTexCoord2f(texRep, texRep);  glVertex3f(plane, plane, 0)
    glTexCoord2f(0, texRep);       glVertex3f(-plane, plane, 0)
    glEnd()
    glPopAttrib()

    # Render the fancy global(inertial) coordinates
    #RenderFancyGlobalAxis(quadric, 0.7 / 2, 0.3 / 2, 0.025)

    DrawBiped(gCon.body, drawAxis=True, wireframe=False)
    #DrawBiped(gCon.body0, drawAxis=True, wireframe=False)
    DrawBipedFibers()
    DrawBipedContactPoints()
    DrawBipedContactForces()

def FrameMove():
    if gRunMode in ['IMPINT', 'CONTROL']:
        FrameMove_ImpInt()
        #FrameMove_ImpInt_Revised()
    else:
        FrameMove_Mocap()

def GetFiberVector_WC(fiber):
    assert fiber.bAttachedPosNormalized == False
    orgBody = gCon.body[ gCon.findBodyIndex(fiber.orgBody) ]
    insBody = gCon.body[ gCon.findBodyIndex(fiber.insBody) ]

    orgPos = orgBody.globalPos(fiber.orgPos)
    insPos = insBody.globalPos(fiber.insPos)
    return insPos - orgPos

def FrameMove_ImpInt_Revised():
    global gCon
    nBody   = len(gCon.body)
    nMuscle = len(gCon.fibers)
    nd      = 3 + 4 # Quaternion version
    nY      = 2*nd*nBody + nMuscle

    #
    # Python ctype type definitions
    #
    DBL_nMuscle    = ct.c_double  *  nMuscle
    DBL_nY         = ct.c_double  *  nY
    DBL_nBodyx18   = (ct.c_double *  18) * nBody
    DBL_nBodyx6    = (ct.c_double *   6) * nBody
    DBL_nMusclex12 = (ct.c_double *  12) * nMuscle
    UINT_nMusclex2 = (ct.c_uint   *   2) * nMuscle

    # Input parameters
    C_h        = ct.c_double(gCon.h)
    C_nBody    = ct.c_int(len(gCon.body))
    C_nMuscle  = ct.c_int(len(gCon.fibers))
    C_extForce = DBL_nBodyx6()

    for i in range(nBody):
        # Renormalize rotation quaternion
        gCon.body[i].q[3:7] = quat_normalize(gCon.body[i].q[3:7])

        # Set gravitational forces
        C_extForce[i][2] = -gGravAcc * gCon.body[i].mass

    C_musclePair = UINT_nMusclex2()
    for i in range(nMuscle):
        C_musclePair[i][0] = gCon.findBodyIndex(gCon.fibers[i].orgBody)
        C_musclePair[i][1] = gCon.findBodyIndex(gCon.fibers[i].insBody)
    # Input/output parameters
    ## Body
    C_body = DBL_nBodyx18()
    for i in range(nBody):
        body = gCon.body[i]
        C_body[i][0:7] = body.q
        C_body[i][7:14] = body.qd
        C_body[i][14] = body.mass
        inertiaMat = BoxInertia(body.boxsize, body.mass)
        C_body[i][15:18] = inertiaMat.diagonal()
    ## Muscle
    C_muscle = DBL_nMusclex12()
    for i in range(nMuscle):
        muscle = gCon.fibers[i]

        orgBodyIdx = gCon.findBodyIndex(muscle.orgBody)
        insBodyIdx = gCon.findBodyIndex(muscle.insBody)
        borg = gCon.body[orgBodyIdx]
        bins = gCon.body[insBodyIdx]

        if muscle.bAttachedPosNormalized:
            localorg = array([b / 2. * p for b, p in zip(borg.boxsize, muscle.orgPos)])
            localins = array([b / 2. * p for b, p in zip(bins.boxsize, muscle.insPos)])
        else:
            localorg = muscle.orgPos
            localins = muscle.insPos

        C_muscle[i][0:6] = [muscle.KSE, muscle.KPE, muscle.b, muscle.xrest, muscle.T, muscle.A]
        C_muscle[i][6:9] = localorg
        C_muscle[i][9:12] = localins

    C_nd = ct.c_int(nd) # Degree-of-freedom for a rigid body
    C_nY = ct.c_int(nY) # Dimension of the state vector Y
    C_cost = ct.c_double(0)
    C_cost2 = ct.c_double(0)
    C_ustar = DBL_nMuscle()
    C_Ydesired = DBL_nY()
    C_w_y = DBL_nY()
    C_w_u = DBL_nMuscle()

    #
    # Setting Ydesired (C_Ydesired)
    #
    for i in range(nBody):
        C_Ydesired[ 14*i     : 14*i + 7  ] = gCon.body0[i].q
        C_Ydesired[ 14*i + 7 : 14*i + 14 ] = gCon.body0[i].qd
    #
    # Setting W_Y (C_w_y)
    #
    C_w_u[:] = [0,]*nMuscle

    #
    # Setting W_u (C_w_u)
    #
    for i in xrange(nBody):
        C_w_y[2*nd*i      : 2*nd*i +   nd] = [1,]*(nd)    # Position follow weight
        C_w_y[2*nd*i + nd : 2*nd*i + 2*nd] = [1,]*(nd)       # Velocity follow weight
    C_w_y[2*nd*nBody :           ] = [0,]*(nMuscle)          # Tension follow weight

    # ############################################################
    # Pass all the variables to simcore
    # (Output: C_body)
    # ############################################################
    C_SimCore(C_h, C_nBody, C_nMuscle, C_nd, C_nY,
              C_body, C_extForce, C_muscle, C_musclePair,
              ct.byref(C_cost), ct.byref(C_cost2), C_ustar, C_Ydesired, C_w_y, C_w_u)

    cost = C_cost.value
    print '%4d / %-20s (%s)' % ( gFrame, cost, cost.hex() )
    #print gFrame, '/', C_cost.value, '/', C_ustar[:]
    print 'Tension :', [f.T for f in gCon.fibers]
    print 'Actuation :', [a for a in C_ustar]
    if gPlot:
        global gCostHistory, gTensionHistory, gActuationHistory
        gCostHistory.append(C_cost.value)
        for i in xrange(nMuscle):
            gTensionHistory[i].append(gCon.fibers[i].T)
            #gActuationHistory[i].append(gCon.fibers[i].A)
            gActuationHistory[i].append(C_ustar[i])

        if gFrame == 4000:
            pit.figure(1)
            pit.plot(gCostHistory, 'r')
            pit.ylabel('Cost')
            pit.figure(2)
            for i in xrange(nMuscle):
                pit.plot(gTensionHistory[i])
            pit.ylabel('N')
            pit.figure(3)
            for i in xrange(nMuscle):
                pit.plot(gActuationHistory[i])
            pit.ylabel('N')
            # Show the plot and pause the app
            pit.show()

    # Estimated next velocity for every rigid bodies
    estNextVel = []
    for i in xrange(nBody):
        #C_body[i][0:7] = body.q
        #C_body[i][7:14] = body.qd
        pos = array(C_body[i][0: 7]) # Generalized position in 3+4.
        vel = array(C_body[i][7:14]) # Generalized velocity in 3+4.

        pos[3:7] = quat_normalize(pos[3:7])

        rot0_wc = QuatToV(pos[3:7])
        angvel0_wc = QuatdToVd(pos[3:7], vel[3:7], rot0_wc)
        vel33 = array([ vel        [0],
                        vel        [1],
                        vel        [2],
                        angvel0_wc [0],
                        angvel0_wc [1],
                        angvel0_wc [2] ])
        estNextVel.append( vel33 )

    #
    # Convert 3+4(quat) state vector to 3+3(exp) state vector
    # to pass the data to LCP code and get contact force information
    #
    footBodyNames = [b.name for b in gCon.body]
    footBodies_Py = [] # An array of 'ExpBody'
    footBodies_C  = []
    for fb in footBodyNames:
        fbi = gCon.findBodyIndex(fb)
        fbb = gCon.body[fbi]
        assert fbb.rotParam == 'QUAT_WFIRST'
        pos0_wc = fbb.q[0:3]
        rot0_wc = QuatToV(fbb.q[3:7])
        vel0_wc = fbb.qd[0:3]
        angvel0_wc = QuatdToVd(fbb.q[3:7], fbb.qd[3:7], rot0_wc)

        expBody = ExpBody.ExpBody(fbb.name, None, fbb.mass, fbb.boxsize,
                                  hstack([ pos0_wc, rot0_wc ]),
                                  vel0_wc, angvel0_wc,
                                  [0.1,0.2,0.3])
        expBody.extForce = 0
        # NOTE: expBody already affected by the gravitational force.
        #       We just need to add the fiber forces.
        for fiber in gCon.fibers:
            assert fiber.bAttachedPosNormalized == False
            if fiber.orgBody == expBody.name:
                tensionSign = 1
                attachedLocalPos = fiber.orgPos
            elif fiber.insBody == expBody.name:
                tensionSign = -1
                attachedLocalPos = fiber.insPos
            else:
                continue
            fiberVec = GetFiberVector_WC(fiber)
            fiberVecLen = linalg.norm(fiberVec)
            if fiberVecLen < 1e-8:
                # Degenerate case: assume no force generated from this fiber
                assert False
                pass
            else:
                fiberVec /= fiberVecLen
                #q_estk = expBody.q + gCon.h*expBody.qd # Estimated next step state position
                q_estk = expBody.q
                muscleTension = GeneralizedForce(q_estk[3:6],
                                                 tensionSign * fiberVec * fiber.T,
                                                 attachedLocalPos)
                #expBody.extForce += muscleTension

        gravForce = GeneralizedForce(q_estk[3:6],
                                     array([0,0,-gGravAcc*expBody.mass]),
                                     array([0,0,0]))
        expBody.extForce += gravForce
        footBodies_Py.append(expBody)
        footBodies_C.append(copy.deepcopy(expBody))

    # Run LCP (Python version)
    #BLEM.FrameMove_PythonVersion(footBodies_Py, gCon.h, gCon.mu, gCon.alpha0, nMuscle, di, True)
    # Run LCP (C version)
    contactForcesNotEmpty, C_Ynext = BLEM.FrameMove_CVersion(footBodies_C, estNextVel, gCon.h, gCon.mu, gCon.alpha0, nMuscle, C_NCONEBASIS, C_CONEBASIS, True)
    assert len(C_Ynext) == 2*(3+3)*nBody + nMuscle

    for i, isContacted in zip(xrange(nBody), contactForcesNotEmpty):
        if isContacted:
            # Use simcore(ImpInt) + LCP combined result
            linPos3 = C_Ynext[2*(3+3)*i            : 2*(3+3)*i +          3   ]
            angPos3 = C_Ynext[2*(3+3)*i + 3        : 2*(3+3)*i + (3+3)        ]
            linVel3 = C_Ynext[2*(3+3)*i + (3+3)    : 2*(3+3)*i + (3+3) +  3   ]
            angVel3 = C_Ynext[2*(3+3)*i + (3+3) + 3: 2*(3+3)*i + (3+3) + (3+3)]

            angPos4 = VtoQuat(angPos3)
            angVel4 = QuatdFromV(angPos3, angVel3)

            body = gCon.body[i] # alias
            body.q[0:3]  = linPos3
            body.q[3:7]  = angPos4
            body.qd[0:3] = linVel3
            body.qd[3:7] = angVel4
        else:
            # Use simcore(ImpInt) result
            # Retrieve the result from the simcore and update our states
            body = gCon.body[i] # alias
            body.q = array(C_body[i][0:7])
            body.qd = array(C_body[i][7:14])

    # Use simcore(ImpInt) result to update muscle fiber tension
    for i in range(nMuscle):
        muscle = gCon.fibers[i] # alias
        muscle.T = C_muscle[i][4]

def FrameMove_ImpInt():
    global gCon
    """
    void SimCore(const double h, const int nBody, const int nMuscle,
             double body[nBody][18], double extForce[nBody][6],
             double muscle[nMuscle][12], unsigned int musclePair[nMuscle][2])
    """
    nBody   = len(gCon.body)
    nMuscle = len(gCon.fibers)
    nd      = 3 + 4
    nY      = 2*nd*nBody + nMuscle

    #
    # Python ctype type definitions
    #
    DBL_nMuscle    = ct.c_double  *  nMuscle
    DBL_nY         = ct.c_double  *  nY
    DBL_nBodyx18   = (ct.c_double *  18) * nBody
    DBL_nBodyx6    = (ct.c_double *   6) * nBody
    DBL_nMusclex12 = (ct.c_double *  12) * nMuscle
    UINT_nMusclex2 = (ct.c_uint   *   2) * nMuscle

    # Input parameters
    C_h = ct.c_double(gCon.h)
    C_nBody = ct.c_int(len(gCon.body))
    C_nMuscle = ct.c_int(len(gCon.fibers))
    C_extForce = DBL_nBodyx6()

    for i in range(nBody):
        # Renormalize rotation quaternion
        gCon.body[i].q[3:7] = quat_normalize(gCon.body[i].q[3:7])

        # Set gravitational forces
        C_extForce[i][2] = -gGravAcc * gCon.body[i].mass
        
        gCon.body[i].updateCurrentStateDependentValues()
        
    C_musclePair = UINT_nMusclex2()
    for i in range(nMuscle):
        C_musclePair[i][0] = gCon.findBodyIndex(gCon.fibers[i].orgBody)
        C_musclePair[i][1] = gCon.findBodyIndex(gCon.fibers[i].insBody)
    # Input/output parameters
    ## Body
    C_body = DBL_nBodyx18()
    for i in range(nBody):
        body = gCon.body[i]
        C_body[i][0:7] = body.q
        C_body[i][7:14] = body.qd
        C_body[i][14] = body.mass
        inertiaMat = BoxInertia(body.boxsize, body.mass)
        C_body[i][15:18] = inertiaMat.diagonal()
    ## Muscle
    C_muscle = DBL_nMusclex12()
    for i in range(nMuscle):
        muscle = gCon.fibers[i]

        orgBodyIdx = gCon.findBodyIndex(muscle.orgBody)
        insBodyIdx = gCon.findBodyIndex(muscle.insBody)
        borg = gCon.body[orgBodyIdx]
        bins = gCon.body[insBodyIdx]

        if muscle.bAttachedPosNormalized:
            localorg = array([b / 2. * p for b, p in zip(borg.boxsize, muscle.orgPos)])
            localins = array([b / 2. * p for b, p in zip(bins.boxsize, muscle.insPos)])
        else:
            localorg = muscle.orgPos
            localins = muscle.insPos

        C_muscle[i][0:6] = [muscle.KSE, muscle.KPE, muscle.b, muscle.xrest, muscle.T, muscle.A]
        C_muscle[i][6:9] = localorg
        C_muscle[i][9:12] = localins

    #
    # Convert 3+4(quat) state vector to 3+3(exp) state vector
    # to pass the data to LCP code and get contact force information
    #
    footBodyNames = [b.name for b in gCon.body]
    footBodies_Py = [] # An array of 'ExpBody'
    footBodies_C  = []
    for fb in footBodyNames:
        fbi = gCon.findBodyIndex(fb)
        fbb = gCon.body[fbi]
        assert fbb.rotParam == 'QUAT_WFIRST'
        pos0_wc = fbb.q[0:3]
        rot0_wc = QuatToV(fbb.q[3:7])
        vel0_wc = fbb.qd[0:3]
        angvel0_wc = QuatdToVd(fbb.q[3:7], fbb.qd[3:7], rot0_wc)

        expBody = RigidBody.RigidBody(fbb.name, None, fbb.mass, fbb.boxsize,
                                  hstack([ pos0_wc, rot0_wc ]),
                                  hstack([vel0_wc, angvel0_wc]),
                                  [0.1,0.2,0.3], 'EXP')
        expBody.updateCurrentStateDependentValues()
        expBody.extForce = 0
        # NOTE: expBody already affected by the gravitational force.
        #       We just need to add the fiber forces.
        for fiber in gCon.fibers:
            assert fiber.bAttachedPosNormalized == False
            if fiber.orgBody == expBody.name:
                tensionSign = 1
                attachedLocalPos = fiber.orgPos
            elif fiber.insBody == expBody.name:
                tensionSign = -1
                attachedLocalPos = fiber.insPos
            else:
                continue
            fiberVec = GetFiberVector_WC(fiber)
            fiberVecLen = linalg.norm(fiberVec)
            if fiberVecLen < 1e-8:
                # Degenerate case: assume no force generated from this fiber
                assert False
                pass
            else:
                fiberVec /= fiberVecLen
                #q_estk = expBody.q + gCon.h*expBody.qd # Estimated next step state position
                q_estk = expBody.q
                muscleTension = GeneralizedForce(q_estk[3:6],
                                                 tensionSign * fiberVec * fiber.T,
                                                 attachedLocalPos)
                expBody.extForce += muscleTension
                

        gravForce = GeneralizedForce(q_estk[3:6],
                                     array([0,0,-gGravAcc*expBody.mass]),
                                     array([0,0,0]))
        expBody.extForce += gravForce
        
        footBodies_Py.append(expBody)
        footBodies_C.append(copy.deepcopy(expBody))

    # Run LCP (Python version)
    #BLEM.FrameMove_PythonVersion(footBodies_Py, gCon.h, gCon.mu, gCon.alpha0, nMuscle, di, True)
    # Run LCP (C version)
    BLEM.FrameMove_CVersion(footBodies_C, None, gCon.h, gCon.mu, gCon.alpha0, nMuscle, C_NCONEBASIS, C_CONEBASIS, True)

    footBodies = footBodies_C

    # Contact force visualization
    for gBodyk in gCon.body:
        if hasattr(gBodyk, 'contactPoints'):
            del gBodyk.contactPoints
        if hasattr(gBodyk, 'cf'):
            del gBodyk.cf

    for bodyk, gBodyk in zip(footBodies, gCon.body):
        if hasattr(bodyk, 'contactPoints') and len(bodyk.contactPoints) > 0:
            gBodyk.contactPoints = bodyk.contactPoints[:]
        if hasattr(bodyk, 'cf') and len(bodyk.cf) > 0:
            gBodyk.cf = bodyk.cf[:]

    # Accumulate contact forces computed in LCP to C_extForce.
    # C_extForce is the input of the implicit integrator.
    for b in footBodies:
        if not hasattr(b, 'contactPoints'):
            continue
        for c in xrange(len(b.contactPoints)):
            cf_wc = b.cf[c]
            assert len(cf_wc) == 3
            fbi = gCon.findBodyIndex(b.name)
            #fbb = gCon.body[fbi]
            cp = b.contactPoints[c]

            #print footBodyNames[kp], ':', cf

            # Resultant force in 'world' coordinates
            C_extForce[fbi][0:3] += cf_wc

            r_bc = b.corners[cp]
            cf_bc = b.localVec(cf_wc)
            resTorque_bc = cross(r_bc, cf_bc)
            # Resultant torque in 'body' coordinates
            C_extForce[fbi][3:6] += resTorque_bc

    C_nd       = ct.c_int(nd) # Degree-of-freedom for a rigid body
    C_nY       = ct.c_int(nY) # Dimension of the state vector Y
    C_cost     = ct.c_double(0)
    C_cost2    = ct.c_double(0)
    C_ustar    = DBL_nMuscle()
    C_Ydesired = DBL_nY()
    C_w_y      = DBL_nY()
    C_w_u      = DBL_nMuscle()

    #
    # Setting Ydesired (C_Ydesired)
    #
    for i in range(nBody):
        C_Ydesired[ 14*i     : 14*i + 7  ] = gCon.body0[i].q
        C_Ydesired[ 14*i + 7 : 14*i + 14 ] = gCon.body0[i].qd
    #
    # Setting W_Y (C_w_y)
    #
    C_w_u[:] = [1e-10,]*nMuscle
    #C_w_u[0] = 0
    '''
    
    C_w_u[1] = 1e-5
    C_w_u[2] = 1e-5
    C_w_u[3] = 1e-5
    C_w_u[4] = 1e-5
    '''
    
    #
    # Setting W_u (C_w_u)
    #
    for i in xrange(nBody):
        C_w_y[2*nd*i      : 2*nd*i +   3] = [10,]*(3 )
        C_w_y[2*nd*i +  3 : 2*nd*i +   7] = [10,]*(4 )
        C_w_y[2*nd*i +  7 : 2*nd*i +  10] = [1e-2,]*(3 )
        C_w_y[2*nd*i + 10 : 2*nd*i +  14] = [1e-2,]*(4 )

    i = 0
    C_w_y[2*nd*i      : 2*nd*i +   3] = [1,]*(3 )
    C_w_y[2*nd*i +  3 : 2*nd*i +   7] = [1,]*(4 )
    C_w_y[2*nd*i +  7 : 2*nd*i +  10] = [0.0001,]*(3 )
    C_w_y[2*nd*i + 10 : 2*nd*i +  14] = [0.0001,]*(4 )
    
    i = 1
    C_w_y[2*nd*i      : 2*nd*i +   3] = [10,]*(3 )
    C_w_y[2*nd*i +  3 : 2*nd*i +   7] = [10,]*(4 )
    C_w_y[2*nd*i +  7 : 2*nd*i +  10] = [0.1,]*(3 )
    C_w_y[2*nd*i + 10 : 2*nd*i +  14] = [0.1,]*(4 )

    C_w_y[0+2*nd*nBody :           ] = [0,]*(nMuscle)
    
    # Pass all the variables to the simcore
    C_SimCore(C_h, C_nBody, C_nMuscle, C_nd, C_nY,
              C_body, C_extForce, C_muscle, C_musclePair,
              ct.byref(C_cost), ct.byref(C_cost2), C_ustar, C_Ydesired, C_w_y, C_w_u)
    cost = C_cost.value
    cost2 = C_cost2.value
    print '%4d / %-20s (%s)   %-20s (%s)  ' % ( gFrame, cost, cost.hex(), cost2, cost2.hex() )
    #print gFrame, '/', C_cost.value, '/', C_ustar[:]
    print 'Tension :', [f.T for f in gCon.fibers]
    print 'Actuation :', [a for a in C_ustar]
    if gPlot:
        global gCostHistory, gTensionHistory, gActuationHistory
        gCostHistory.append(C_cost.value)
        for i in xrange(nMuscle):
            gTensionHistory[i].append(gCon.fibers[i].T)
            #gActuationHistory[i].append(gCon.fibers[i].A)
            gActuationHistory[i].append(C_ustar[i])

        if gFrame == 4000:
            pit.figure(1)
            pit.plot(gCostHistory, 'r')
            pit.ylabel('Cost')
            pit.figure(2)
            for i in xrange(nMuscle):
                pit.plot(gTensionHistory[i])
            pit.ylabel('N')
            pit.figure(3)
            for i in xrange(nMuscle):
                pit.plot(gActuationHistory[i])
            pit.ylabel('N')
            # Show the plot and pause the app
            pit.show()

    '''
    cfTotal = 0
    if hasattr(gCon.body[1], 'contactPoints'):
        for cf in gCon.body[1].cf:
            cfTotal += linalg.norm(cf)
    print 'cfTotal =', cfTotal
    '''

    # Retrieve the result from the simcore and update our states
    for i in range(nBody):
        body = gCon.body[i]
        body.q = array(C_body[i][0:7])
        body.qd = array(C_body[i][7:14])
    for i in range(nMuscle):
        muscle = gCon.fibers[i]
        muscle.T = C_muscle[i][4]

def DetermineActiveness(body, alpha0):
    # 'activeCorners' has tuples of the form (body index, corner index)
    activeCorners = []
    activeBodies = set()
    activeCornerPoints = []
    nb = len(body)
    for k in range(nb):
        # Check all eight corners
        bd = body[k]
        mass, size, inertia = bd.mass, bd.boxsize, bd.I
        q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
        sx, sy, sz = size

        for i in range(8):
            A = RotationMatrixFromEulerAngles_xyz(q[3], q[4], q[5])
            c = q[0:3] + dot(A, corners[i])
            if c[2] < alpha0:
                activeCorners.append((k, i))
                activeBodies.add(k)
                activeCornerPoints.append(c)

    # Indices for active/inactive bodies
    inactiveBodies = list(set(range(nb)) - activeBodies)
    activeBodies = list(activeBodies)           
    return activeCorners, activeCornerPoints, activeBodies, inactiveBodies


def BuildEquationsOfMotion(body, bodies, h):
    # 'bodies' contains the indices of rigid body list (body)
    nb   = len(bodies)
    Minv = zeros((6 * nb, 6 * nb))
    Cqd  = zeros((6 * nb))
    Q    = zeros((6 * nb))
    Qd   = zeros((6 * nb))

    for k in bodies:
        bd = gCon.body[k]
        mass, size, inertia = bd.mass, bd.boxsize, bd.I
        q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
        assert len(q) == 6 and len(qd) == 6
        sx, sy, sz = size

        #
        # NOTE: M*qdd + Cqdx      = fg
        #       M*qdd + Cqdx - fg = 0
        #       M*qdd + Cqd_k     = 0
        #
        Minv_k = SymbolicMinv(q + h * qd / 2, inertia)
        Cqd_k = SymbolicCqd(q + h * qd / 2, qd, inertia)
        fg = SymbolicForce(q + h * qd / 2, (0, 0, -9.81 * mass), (0, 0, 0))
        Cqd_k = Cqd_k - fg
        #Cqd_k = Cqd_k - torque[(int)(frame*h)][k]*h

        kk = bodies.index(k)
        Minv[6 * kk:6 * (kk + 1), 6 * kk:6 * (kk + 1)] = Minv_k
        Cqd[6 * kk:6 * (kk + 1)] = Cqd_k
        Q[6 * kk:6 * (kk + 1)] = q
        Qd[6 * kk:6 * (kk + 1)] = qd

    return Minv, Cqd, Q, Qd

def BuildLcpAndSolve(activeBodies, activeCorners, activeCornerPoints, h, mu, Minv_a, Cqd_a, Q_a, Qd_a):
    nba = len(activeBodies)
    p = len(activeCorners) # number of contact points
    err = 0 # Lemke's algorithm return code: 0 means success

    # Basis for contact normal forces (matrix N)
    N = zeros((6 * nba, p))
    # Basis for tangential(frictional) forces (matrix D)
    D = zeros((6 * nba, 8 * p))
    for i in range(p):
        # kp: Body index
        # cp: Corner index
        kp, cp = activeCorners[i]
        bd = gCon.body[kp]
        mass, size, inertia = bd.mass, bd.boxsize, bd.I
        q, qd, corners, dc = bd.q, bd.qd, bd.corners, bd.dc
        sx, sy, sz = size
        k = activeBodies.index(kp)

        # Which one is right?
        #N[6*k:6*(k+1), i] = SymbolicForce(q + h*qd/2, (0, 0, 1), corners[cp])
        N[6 * k:6 * (k + 1), i] = SymbolicPenetration(q + h * qd / 2, corners[cp])


        Di = zeros((6 * nba, 8)) # Eight basis for tangential forces
        for j in range(8):
            Di[6 * k:6 * (k + 1), j] = SymbolicForce(q + h * qd / 2, gCon.di[j], corners[cp])
        D[:, 8 * i:8 * (i + 1)] = Di

    #print '-----------------------------------------------------'
    #print Minv_a, N, D

    M00 = dot(dot(N.T, Minv_a), N)
    M10 = dot(dot(D.T, Minv_a), N)
    M11 = dot(dot(D.T, Minv_a), D)
    Z0 = zeros((p, p))

    # Friction coefficient
    Mu = diag([mu] * p)
    # matrix E
    E = zeros((8 * p, p))
    for i in range(p):
        for j in range(8):
            E[8 * i + j, i] = 1


    LCP_M = vstack([hstack([ M00 , M10.T , Z0 ]),
                    hstack([ M10 , M11   , E  ]),
                    hstack([ Mu  , -E.T  , Z0 ])])

    #
    # NOTE: M*qdd + Cqd = 0
    #
    # in Cqd, '-fg' term included.
    #
    LCP_q0 = h * dot(dot(N.T, Minv_a), -Cqd_a) + dot(N.T, Qd_a)
    LCP_q1 = h * dot(dot(D.T, Minv_a), -Cqd_a) + dot(D.T, Qd_a)
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
    cn = x_opt[0    :p]
    beta = x_opt[p    :p + 8 * p]
    lamb = x_opt[p + 8 * p:p + 8 * p + p]

    return N, cn, D, beta


def FrameMove_Mocap():
    global gCon
    # Total number of rigid bodies
    nb = len(gCon.body)
    h = gCon.h
    mu = gCon.mu


    ### TRAJECTORY INPUT ###
    for k in range(nb):
        gCon.body[k].q = gCon.q_data[gCon.curFrame][k]
        gCon.body[k].qd = gCon.qd_data[gCon.curFrame][k]


    activeCorners, activeCornerPoints, activeBodies, inactiveBodies = DetermineActiveness(gCon.body, gCon.alpha0)
    # Active corner points are used later. Keep them in gCon.
    gCon.activeCornerPoints = activeCornerPoints


    '''
    Process for 'active' bodies
    '''
    # Total number of contact points
    if len(activeBodies) > 0:
        Minv_a, Cqd_a, Q_a, Qd_a = BuildEquationsOfMotion(gCon.body, activeBodies, h)

        N, cn, D, beta = BuildLcpAndSolve(activeBodies, activeCorners, activeCornerPoints, h, mu, Minv_a, Cqd_a, Q_a, Qd_a)

        ground_reaction_force = dot(N, cn) + dot(D, beta)
        Qd_a_next = dot(Minv_a, ground_reaction_force + h * Cqd_a) + Qd_a
        Q_a_next = h * Qd_a_next + Q_a

        '''
        Update the state of active bodies
        '''
        for k in activeBodies:
            kk = activeBodies.index(k)
            gCon.body[k].q = Q_a_next[6 * kk:6 * (kk + 1)]
            gCon.body[k].qd = Qd_a_next[6 * kk:6 * (kk + 1)]
        '''
        Prepare the data for contact force visualization
        '''
        p = len(activeCorners)
        beta_reshaped = beta.reshape(p, 8)
        gCon.contactForces = []
        for i, acp, cn_i, beta_i, ac_i in zip(range(p),
                                              activeCornerPoints,
                                              cn,
                                              beta_reshaped,
                                              activeCorners):
            bodyidx = activeBodies.index(ac_i[0])
            fric = dot(D[:, 8 * i:8 * (i + 1)], beta_i)[6 * bodyidx:6 * (bodyidx + 1)]
            fricdir = array([fric[0] * gCon.cfScaleFactor,
                             fric[1] * gCon.cfScaleFactor,
                             cn_i * gCon.cfScaleFactor])
            friclen = linalg.norm(fricdir)
            if friclen > 0:
                fricdirn = fricdir / friclen
                cf = (fricdirn, friclen)
                gCon.contactForces.append(cf)


    '''
    Process for 'inactive' bodies
    (no effect of external forces)
    '''
    Minv_i, Cqd_i, Q_i, Qd_i = BuildEquationsOfMotion(gCon.body, inactiveBodies, h)
    Qd_i_next = dot(Minv_i, h * Cqd_i) + Qd_i
    Q_i_next = h * Qd_i_next + Q_i
    '''
    Update the state of inactive bodies
    '''
    for k in inactiveBodies:
        kk = inactiveBodies.index(k)
        gCon.body[k].q = Q_i_next[6 * kk:6 * (kk + 1)]
        gCon.body[k].qd = Qd_i_next[6 * kk:6 * (kk + 1)]
        z0 = 0

    '''
    if gRunMode == 'SINGLE':
        print gCon.body[0].q
    '''



    ### TRAJECTORY INPUT ###
    for k in range(nb):
        gCon.body[k].q = gCon.q_data[gCon.curFrame][k]
        gCon.body[k].qd = gCon.qd_data[gCon.curFrame][k]


    if gCon.autoPlay:
        gCon.curFrame = gCon.curFrame + 1
        if hasattr(gCon, 'noFrame'):
            if gCon.curFrame >= gCon.noFrame - 2:
                gCon.curFrame = 0

def MainNoGl(gCon):
    global gFrame
    while gFrame < 2:
        FrameMove()
        gFrame += 1


################################################################################
################################################################################

if __name__ == '__main__':
    gFrame   = 0
    gWorkDir = '/home/johnu/pymuscle/'
    gRunMode = 'CONTROL'
    assert gRunMode in ['MOCAP',            # Motion capture data player with contact force calc
                        'IMPINT',           # Implicit integration tester
                        'CONTROL',          # Stay-still controller
                        'SINGLE'            # Single rigid body tester
                        ]

    libsimcore = ct.CDLL(gWorkDir + 'bin/Release/libsimcore_release.so')
    C_SimCore = libsimcore.SimCore_Python
    libLCP = ct.CDLL(gWorkDir + 'LCP/bin/Release/libLCP_release.so')
    C_StartOctaveEngine = libLCP.StartOctaveEngine
    C_TestOctaveEngine  = libLCP.TestOctaveEngine
    C_StartOctaveEngine()
    C_TestOctaveEngine()
    # Some api in the chain is translating the keystrokes to this octal string
    # so instead of saying: ESCAPE = 27, we use the following.
    ESCAPE = '\033'
    LEFTARROW = 100
    RIGHTARROW = 102

    # Always raise an exception when there is a numerically
    # incorrect value appeared in NumPy module.
    seterr(all='raise')

    #print 'Let\'s go!'

    poiList = ['lowerback', 'trunk', 'soleL', 'thighL', 'calfL']

    if gRunMode == 'MOCAP':
        gCon = GlobalContext(gWorkDir + 'traj_', 'EULER_XYZ')   
    elif gRunMode == 'IMPINT':
        gCon = GlobalContext(gWorkDir + 'traj_', 'QUAT_WFIRST')
        gBipedParam = BipedParameter()
        gCon.body = gBipedParam.buildBody()
        gCon.body0 = copy.deepcopy(gCon.body) #gBipedParam.buildBody() # Guide body
        gCon.fibers = gBipedParam.buildFiber([b.name for b in gCon.body])
        gCon.bodyList = []
        for b in gCon.body:
            gCon.bodyList.append((b.name, None))

            b.q  = array(b.q)
            b.qd = array(b.qd)

        # Write a configuration file containing body and muscle settings.
        # This file can be read at the simcore side.
        WriteSimcoreConfFile('box_lcp5.conf', gCon.body, gCon.fibers, gCon.h)
    elif gRunMode == 'CONTROL':
        gCon = GlobalContext(None, 'QUAT_WFIRST')
        gBipedParam = BipedParameter()
        gCon.body = gBipedParam.buildBody()
        gCon.fibers = gBipedParam.buildFiber([b.name for b in gCon.body])
        gCon.bodyList = []
        for b in gCon.body:
            gCon.bodyList.append((b.name, None))
            b.q  = array(b.q)
            b.qd = array(b.qd)
        gCon.body0   = copy.deepcopy(gCon.body)    # Guide bodies
        gCon.fibers0 = copy.deepcopy(gCon.fibers)  # Guide fibers
        for b, b0 in zip(gCon.body, gCon.body0):
            assert all(b.q == b0.q)
        for f, f0 in zip(gCon.fibers, gCon.fibers0):
            assert all(f.T == f0.T)

        # Write a configuration file containing body and muscle settings.
        # This file can be read at the simcore side.
        WriteSimcoreConfFile('box_lcp5.conf', gCon.body, gCon.fibers, gCon.h)
    elif gRunMode == 'SINGLE':
        gCon = GlobalContext(None, 'EULER_ZXZ')
        '''
        pb = PmBody('lowerback', None, 1, array([0.1,0.2,0.3]),
                    zeros(6), zeros(6), array([0.5,0.2,0.7]), 'EULER_ZXZ')
        '''
        pb = RigidBody('lowerback', None, 1, array([0.1,0.2,0.3]),
                    zeros(6), zeros(6), array([0.5,0.2,0.7]), 'EULER_ZXZ')
        gCon.body = [ pb ]
        gCon.bodyList = [ ('lowerback', None) ]
        gCon.fibers = []
        gCon.body[0].q = array([0,0,10,0,math.pi/2,0])

    C_NCONEBASIS, C_CONEBASIS = BLEM.BuildConeBasisForC()
    di = BLEM.BuildConeBasis()
    gUseOpenGl = False
    gUseC      = True
    gUsePy     = False
    gReprod    = False
    gPlot      = False
    gLoad      = False
    gGravAcc   = 9.81    # Magnitude only

    if len(sys.argv) >= 2 and (sys.argv[1] == '--help' or sys.argv[1] == '-h'):
        print sys.argv[0], ': Muscle fiber simulator'
        print '-----------------------------------------------'
        print '     python', sys.argv[0], '[options]'
        print
        print '   OPTIONS'
        print '      -gl        Use OpenGL window                     (default)'
        print '      -c  | -py  C/Python compiled binary used as core (default: C)'
        print '      -reprod    Reproducibility test'
        print '      -plot'
        print
        sys.exit(0)

    if '-gl' in sys.argv[1:]:
        gUseOpenGl = True
    if '-c' in sys.argv[1:]:
        gUseC      = True
    if '-py' in sys.argv[1:]:
        gUsePy     = False
    if '-reprod' in sys.argv[1:]:
        gReprod    = True
    if '-plot' in sys.argv[1:]:
        gPlot      = True
    if '-load' in sys.argv[1:]:
        idx = sys.argv.index('-load')
        fileName = sys.argv[idx+1]
        try:
            lastStateFile = open(fileName,'r')
            lastState = cPickle.load(lastStateFile)
            gCon.body, gCon.fibers = lastState
            lastStateFile.close()
            print 'Last state loaded successfully.'
        except:
            print 'WARN: No last state file found or exceptional file format.'


    # Plotting
    if gPlot:
        gCostHistory      = []
        gTensionHistory   = []
        gActuationHistory = []
        for i in xrange(len(gCon.fibers)):
            gTensionHistory.append( [] )
            gActuationHistory.append( [] )

    if gUseOpenGl:
        Main(gCon)
    else:
        if gReprod:
            assert gPlot
            gCostHistoryReprod = []
            for i in xrange(3):
                print '    *************    Reproducibility test iter', i, '    *************'
                gFrame = 0
                gCon.body   = copy.deepcopy(gCon.body0)
                gCon.fibers = copy.deepcopy(gCon.fibers0)
                for b, b0 in zip(gCon.body, gCon.body0):
                    assert all(b.q == b0.q)
                for f, f0 in zip(gCon.fibers, gCon.fibers0):
                    assert all(f.T == f0.T)
                MainNoGl(gCon)
                gCostHistoryReprod.append ( copy.deepcopy(gCostHistory) )
                gCostHistory      = []
                gTensionHistory   = []
                gActuationHistory = []
                for i in xrange(len(gCon.fibers)):
                    gTensionHistory.append( [] )
                    gActuationHistory.append( [] )
            for i in xrange(len(gCostHistoryReprod[0])):
                historySet = set([])
                for j in xrange(len(gCostHistoryReprod)):
                    historySet.add( gCostHistoryReprod[j][i] )

                    if len(historySet) != 1:
                        print '   *****************************************'
                        print '      FAILED : Results not reproducible.'
                        print '   *****************************************'
                        sys.exit(-5)

        else:
            MainNoGl(gCon)
