#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Biped model builder
"""
from numpy import *
from math import *
import OpenGL
OpenGL.ERROR_CHECKING = False
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from glprim import *
from MathUtil import *
import sys
import PIL.Image as pil
import pygame
from pygame.font import *
import RigidBody, MuscleFiber
from GlobalContext import *
import ctypes as ct
from WriteSimcoreConfFile import *
import BipedParameter

ESCAPE = '\033'

def InitializeGl():                # We call this right after our OpenGL window is created.
    glClearColor(0,0,0,1.0)
    glClearDepth(1.0)                                    # Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LEQUAL)                                # The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST)                                # Enables Depth Testing
    glShadeModel (GL_FLAT);                                # Select Flat Shading (Nice Definition Of Objects)
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)     # Really Nice Perspective Calculations
    #glEnable (GL_LIGHT0)
    #glEnable (GL_LIGHTING)
    glEnable (GL_COLOR_MATERIAL)
    #glEnable(GL_TEXTURE_2D)
    #glLineWidth(2)
    #gQuadric = gluNewQuadric(1)
    #glDisable(GL_CULL_FACE);
    glEnable(GL_LINE_SMOOTH);

def ResizeGlScene(winWidth, winHeight):
    if winHeight == 0:
        winHeight = 1
    global gWinWidth, gWinHeight
    gWinWidth  = winWidth
    gWinHeight = winHeight

def KeyPressed(*args):
    global gWireframe, gPoi, gDrawLiga, gDrawMuscle
    key = args [0]
    if key == ESCAPE: sys.exit ()
    elif key == 'z': gWireframe = not gWireframe
    elif key >= '1' and key <= '5': gPoi = int(key)-1
    elif key == 'm': gDrawMuscle = not gDrawMuscle
    elif key == 'l': gDrawLiga = not gDrawLiga

def Draw():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    # Point Of Interest
    global gPoi
    poiName, zoomLevel = gPoiList[gPoi]
    if poiName == 'Biped': poi = gBiped.getBipedCenter()
    elif poiName == 'Hip': poi = gBiped.getHipJointCenter()
    elif poiName == 'Knee': poi = gBiped.getKneeJointCenter()
    elif poiName == 'Ankle': poi = gBiped.getAnkleJointCenter()
    elif poiName == 'Toe': poi = gBiped.getToeJointCenter()
    else: raise Exception('What the...')

    SetViewport(0, 0, gWinWidth/2, gWinHeight/2, zoomLevel)
    gluLookAt(poi[0], -100, poi[2],
              poi[0], 0, poi[2],
              0, 0, 1);
    DrawBiped()

    SetViewport(gWinWidth/2, 0, gWinWidth/2, gWinHeight/2, zoomLevel)
    gluLookAt(100, poi[1], poi[2],
              0, poi[1], poi[2],
              0, 0, 1);
    DrawBiped()

    SetViewport(0, gWinHeight/2, gWinWidth/2, gWinHeight/2, zoomLevel)
    gluLookAt(poi[0], poi[1], 100,
              poi[0], poi[1], 0,
              0, 1, 0);
    DrawBiped()

    SetViewport(gWinWidth/2, gWinHeight/2, gWinWidth/2, gWinHeight/2, zoomLevel)
    gluLookAt(poi[0]+1, poi[1]-1.5, poi[2]+1,
              poi[0], poi[1], poi[2],
              0, 0, 1);
    DrawBiped()

    SetViewport(0,0, gWinWidth,gWinHeight, 0)
    glDisable(GL_DEPTH_TEST)
    glColor(0.8,0.8,0.8)
    glBegin(GL_LINES)
    glVertex(-1,0,0)
    glVertex(1,0,0)
    glVertex(0,-1,0)
    glVertex(0,1,0)
    glEnd()
    glEnable(GL_DEPTH_TEST)

    glutSwapBuffers()


def SetViewport(x, y, w, h, zoomLevel):
    glViewport(x, y, w, h)
    aspectRatio = float(gWinWidth) / gWinHeight
    glMatrixMode(GL_PROJECTION)            # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    if zoomLevel != 0:
        glOrtho(-aspectRatio/zoomLevel, aspectRatio/zoomLevel,
                -1./zoomLevel, 1./zoomLevel,
                1, 1000)
    glMatrixMode(GL_MODELVIEW)        # // Select The Modelview Matrix
    glLoadIdentity()                    # // Reset The Modelview Matrix

def DrawRigidBody(rbName, side = None):
    assert side in [None, 'L', 'R']
    rbPos = gBiped.getPos(rbName)
    glPushMatrix()
    if side is not None:
        rbSidedName = rbName + side
    else:
        rbSidedName = rbName
    matMultCol = gBiped.getColumnMajorTransformMatrix(rbSidedName)
    glMultMatrixf(matMultCol)
    DrawAxisIndicator(rbName)
    rbNameLen = len(rbName)
    for (k, v) in gBiped.direction.iteritems():
        if   k[0:rbNameLen] == rbName and v == 0: xAxisKey = k
        elif k[0:rbNameLen] == rbName and v == 1: yAxisKey = k
        elif k[0:rbNameLen] == rbName and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    if side is None:
        glColor(1,0,1)
    elif side == 'L':
        glColor(0,1,1)
    elif side == 'R':
        glColor(1,0,0)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

def DrawBiped():
    DrawRigidBody('trunk')
    DrawRigidBody('head')
    DrawLeg('L')
    DrawLeg('R')
    DrawArm('L')
    DrawArm('R')
    
    glLineWidth(1)
    glBegin(GL_LINES)
    if gDrawLiga:
        glColor(1,1,1) # Ligament is white
        for (muscleName, orgPos, insPos, orgName, insName) in gBiped.getAllLigaments():
            glVertex(orgPos[0], orgPos[1], orgPos[2])
            glVertex(insPos[0], insPos[1], insPos[2])
    if gDrawMuscle:
        glColor(1,0,0) # Muscle is red
        for (muscleName, orgPos, insPos, orgName, insName) in gBiped.getAllMuscles():
            glVertex(orgPos[0], orgPos[1], orgPos[2])
            glVertex(insPos[0], insPos[1], insPos[2])
    glEnd()

def DrawAxisIndicator(name):
    lineLen = gBiped[name +'Width']/2
    glLineWidth(5)
    glBegin(GL_LINES)
    glColor(1,0,0); glVertex(0,0,0); glVertex(lineLen,0,0)
    glColor(0,1,0); glVertex(0,0,0); glVertex(0,lineLen,0)
    glColor(0,0,1); glVertex(0,0,0); glVertex(0,0,lineLen)
    glEnd()
    glLineWidth(1)

def DrawLeg(side):
    DrawRigidBody('thigh', side)
    DrawRigidBody('calf', side)
    DrawRigidBody('sole', side)
    DrawRigidBody('toe', side)

def DrawArm(side):
    DrawRigidBody('uarm', side)
    DrawRigidBody('larm', side)
    
def Main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
    glutInitWindowSize(gWinWidth, gWinHeight)
    glutInitWindowPosition(0, 0)
    window = glutCreateWindow("Muscle")
    glutDisplayFunc(Draw)
    glutIdleFunc(Draw)
    glutReshapeFunc(ResizeGlScene)
    glutKeyboardFunc(KeyPressed)
    InitializeGl()
    glutMainLoop()


gQuadric    = None
gWinWidth   = 700
gWinHeight  = 700
gWireframe  = False
gDrawLiga   = True
gDrawMuscle = True
gPoi        = 0 # Point of interest

class ModelViewerOptions:
    def __init__(self):
        # Fill this with default options
        self.trajconf = None
        self.output   = None
        self.nogl     = False
        self.mu       = 100.0

def ParseCommandLine(argv):
    mvo = ModelViewerOptions()
    for av in argv[1:]:
        avparts = av.partition('=')
        if avparts[0] == '--trajconf':
            mvo.trajconf = avparts[2]
        elif avparts[0] == '--output':
            mvo.output = avparts[2]
        elif avparts[0] == '--nogl':
            mvo.nogl = True
        elif avparts[0] == '--mu':
            mvo.mu = float(avparts[2])                        
        else:
            print 'Error - command line argument', av, 'unknown.'
            raise Exception
        
    if mvo.output is None and mvo.trajconf is None:
        mvo.output = 'sample.sim.conf'
    elif mvo.output is None and mvo.trajconf:
        mvo.output = mvo.trajconf[:mvo.trajconf.find('.')] + '.sim.conf'
    return mvo

if __name__ == '__main__':
    print 'PymModelBuilder: Biped model builder      -- 2010 Geoyeob Kim'
    if len(sys.argv) > 4 or '--help' in sys.argv[1:]:
        print '  Usage:'
        print '    python PymModelBuilder.py [Options]'
        print
        print '  Options:'
        print '    --trajconf=<path>  : input trajectory conf'
        print '    --output=<path>    : output simulation conf'
        print '    --nogl             : only writing simulation conf'
        print '    --mu=<real number> : friction coefficient'
        print
        sys.exit(-1)
    try:
        mvo = ParseCommandLine(sys.argv)
    except:
        print 'Exception occurred during processing command line arguments.'
        print 'See the help by using the \'--help\' argument.'
        print 'Aborted.'
        sys.exit(-2)
    
    fnRbConf = mvo.trajconf
    gBiped = BipedParameter.BipedParameter(fnRbConf)
    bipHeight = gBiped.getBipedHeight()
    
    gPoiList = ( ('Biped', 1.5/bipHeight),
                 ('Hip', 5./bipHeight),
                 ('Knee', 5./bipHeight),
                 ('Ankle', 5./bipHeight),
                 ('Toe', 5./bipHeight) )

    print 'Initialize a rigid body with initial conditions...'
    if fnRbConf:
        print '    using', fnRbConf
    bipedParam = gBiped
    plist = bipedParam.buildBody('EXP')
    flist = bipedParam.buildFiber([b.name for b in plist])
    h = GetSimTimeStep()
    print 'Biped Height          :', bipHeight, 'm'
    print '# of rigid bodies     :', len(plist)
    print '# of muscle fibers    :', len(flist)
    print 'Simulation time step  :', h, 'seconds'
    
    fnOutput = mvo.output
    WriteSimcoreConfFile(fnOutput, plist, flist, h, mvo.mu)
    print 'Simulation conf file written to', fnOutput
    
    if hasattr(mvo, 'nogl') and mvo.nogl == True:
        pass
    else:
        Main()
        