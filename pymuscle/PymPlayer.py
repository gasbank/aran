#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Motion data player
"""
import ctypes as ct
from math import *
import sys
import time
import cPickle

from numpy import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import PIL.Image as pil
import pygame
from pygame.font import *

from glprim import *
from MathUtil import *
from GlobalContext import *
from WriteSimcoreConfFile import *
from Parameters import BipedParameter, RigidBodyFromRbConf

ESCAPE = '\033'

def InitializeGl():             # We call this right after our OpenGL window is created.
    glClearColor(0,0,0,1.0)
    glClearDepth(1.0)                                   # Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LEQUAL)                              # The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST)                             # Enables Depth Testing
    glShadeModel (GL_FLAT);                             # Select Flat Shading (Nice Definition Of Objects)
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)  # Really Nice Perspective Calculations
    #glEnable (GL_LIGHT0)
    #glEnable (GL_LIGHTING)
    glEnable (GL_COLOR_MATERIAL)
    #glEnable(GL_TEXTURE_2D)
    #glLineWidth(2)
    #gQuadric = gluNewQuadric(1)
    #glDisable(GL_CULL_FACE);

def ResizeGlScene(winWidth, winHeight):
    if winHeight == 0:
        winHeight = 1
    global gWinWidth, gWinHeight
    gWinWidth  = winWidth
    gWinHeight = winHeight

def KeyPressed(*args):
    global gWireframe, gPoi, gRot, gSlowVideo, gCameraMat, gDrawTraj, gDrawSim
    rotAmount = 0.05
    key = args [0]
    if key == ESCAPE:
        stateFile = open(camdataFn, 'w')
        state = (gCameraMat, gRot, gSlowVideo)
        cPickle.dump(state, stateFile)
        stateFile.close()
        print camdataFn, ': State written.'
        sys.exit ()
    elif key == 'w':
        gWireframe = not gWireframe
    elif key == '1': # +Zaxis rot
        gRot[2] += rotAmount
    elif key == '3': # -Zaxis rot
        gRot[2] -= rotAmount
    elif key == '4': # +Xaxis rot
        gRot[0] += rotAmount
    elif key == '6': # -Xaxis rot
        gRot[0] -= rotAmount
    elif key == '2': # -Yaxis rot
        gRot[1] += rotAmount
    elif key == '8': # +Yaxis rot
        gRot[1] -= rotAmount
    elif key == '+':
        if gSlowVideo > 1:
            gSlowVideo -= 1
    elif key == '-':
        gSlowVideo += 1
    elif key == 'a':
        gCameraMat[0:3,3] += gModelviewCameraMat[0:3,2]/5
    elif key == 'z':
        gCameraMat[0:3,3] -= gModelviewCameraMat[0:3,2]/5
    elif key == 't':
        gDrawTraj = not gDrawTraj
    elif key == 's':
        gDrawSim = not gDrawSim
def SpecialKeyPressed(*args):
    global gWireframe, gPoi, gCameraMat
    key = args [0]
    LEFTKEY  = 100
    UPKEY    = 101
    RIGHTKEY = 102
    DOWNKEY  = 103
    if key == LEFTKEY:
        gCameraMat[0:3,3] += gModelviewCameraMat[0:3,0]/5
    elif key == RIGHTKEY:
        gCameraMat[0:3,3] -= gModelviewCameraMat[0:3,0]/5
    elif key == UPKEY:
        gCameraMat[0:3,3] -= gModelviewCameraMat[0:3,1]/5
    elif key == DOWNKEY:
        gCameraMat[0:3,3] += gModelviewCameraMat[0:3,1]/5
        

def Draw():
    global gCurFrame, gModelviewCameraMat
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    
    glMatrixMode(GL_PROJECTION)         # // Select The Projection Matrix
    glLoadIdentity()                    # // Reset The Projection Matrix
    gluPerspective(45, 1.0, 0.1, 100)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()                    # // Reset The Modelview Matrix
    
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0)
    
    gcm = glGetDouble( GL_MODELVIEW_MATRIX )
    glRotate(gRot[0]/pi*180, gcm[0,0], gcm[1,0], gcm[2,0])
    glRotate(gRot[1]/pi*180, gcm[0,1], gcm[1,1], gcm[2,1])
    glRotate(gRot[2]/pi*180, gcm[0,2], gcm[1,2], gcm[2,2])
    
    glMultMatrixd(gCameraMat.T.flatten())
    
    gModelviewCameraMat = glGetDouble( GL_MODELVIEW_MATRIX )
    
    
    groundSize = 1.5
    glColor(0.3,0.3,0.3)
    glBegin(GL_QUADS)
    glVertex(-groundSize, -groundSize, 0)
    glVertex( groundSize, -groundSize, 0)
    glVertex( groundSize,  groundSize, 0)
    glVertex(-groundSize,  groundSize, 0)
    glEnd()
    
    if gDrawSim:
        glColor(1,1,1)
        nb = simHeader[1]
        for i in xrange(nb):
            qi = simData[gCurFrame*nb + i]
            glPushMatrix()
            glTranslate(qi[0], qi[1], qi[2])
            
            anotherName = gBiped.anotherNameList[i]
            if anotherName != 'trunk':
                anotherName = anotherName[:-1]
            for (k, v) in gBiped.direction.iteritems():
                if   k[ 0:len(anotherName) ] == anotherName and v == 0: xAxisKey = k
                elif k[ 0:len(anotherName) ] == anotherName and v == 1: yAxisKey = k
                elif k[ 0:len(anotherName) ] == anotherName and v == 2: zAxisKey = k
            
            #print gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey]
            v = [qi[3],qi[4],qi[5]]
            quat = VtoQuat(v)
            A = RotationMatrixFromQuaternion(quat[0], quat[1], quat[2], quat[3])
            A_homo = identity(4)
            A_homo[0:3, 0:3] = A
            glMultMatrixd(A_homo.T.flatten())
            
            #print 'SIM'
            #print gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey]
            glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey])
            del xAxisKey, yAxisKey, zAxisKey
            
            glutWireCube(1)
            glPopMatrix()

    if gDrawTraj:
        glColor(1,0,0)
        nb = trajHeader[1]
        for i in xrange(nb):
            name = gBiped.nameList[i]
            anotherName = gBiped.rbConf[name].anotherName
            if anotherName == '*':
                continue
            
            qi = trajData[(gCurFrame + 2)*nb + i]
            glPushMatrix()
            glTranslate(qi[0], qi[1], qi[2])
            
            if anotherName != 'trunk':
                anotherName = anotherName[:-1]
            for (k, v) in gBiped.direction.iteritems():
                if   k[ 0:len(anotherName) ] == anotherName and v == 0: xAxisKey = k
                elif k[ 0:len(anotherName) ] == anotherName and v == 1: yAxisKey = k
                elif k[ 0:len(anotherName) ] == anotherName and v == 2: zAxisKey = k
            
            #print gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey]
            v = [qi[3],qi[4],qi[5]]
            quat = VtoQuat(v)
            A = RotationMatrixFromQuaternion(quat[0], quat[1], quat[2], quat[3])
            A_homo = identity(4)
            A_homo[0:3, 0:3] = A
            glMultMatrixd(A_homo.T.flatten())
            
            #print 'TRAJ'
            #print gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey]
            glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey])
            del xAxisKey, yAxisKey, zAxisKey
            
            glutWireCube(1)
            glPopMatrix()
    
    if gDrawSim or gDrawTraj:
        gCurFrame += 1
        nb = simHeader[1]
        if gCurFrame >= len(simData)/nb:
            gCurFrame = 0
    
    glutSwapBuffers()
    
    # DEBUG
    #sys.exit ()
    ##
    
    time.sleep(0.01*gSlowVideo)

def Main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)
    glutInitWindowSize(gWinWidth, gWinHeight)
    glutInitWindowPosition(0, 0)
    window = glutCreateWindow("PymPlayer")
    glutDisplayFunc(Draw)
    glutIdleFunc(Draw)
    glutReshapeFunc(ResizeGlScene)
    glutKeyboardFunc(KeyPressed)
    glutSpecialFunc(SpecialKeyPressed)
    InitializeGl()
    glutMainLoop()


gQuadric = None
gWinWidth = 700
gWinHeight = 700
gWireframe = False
gCurFrame = 0
gModelviewCameraMat = identity(4)
gCameraMat = identity(4)
gRot = [0,0,0] 
gPoi = 0 # Point of interest
camdataFn = 'PymPlayer.camdata'
gSlowVideo = 1
gDrawSim = True
gDrawTraj = True
 
def loadTrajData(fnTraj):
    traj = open(fnTraj, 'r')
    trajHeader = [int(x) for x in traj.readline().split()]
    trajData = []
    for line in traj:
        q = array( [ float(x) for x in line.split()[0:6] ] )
        assert len(q) == 6
        trajData.append(q)
        
    if trajHeader[0]*trajHeader[1] != len(trajData):
        print 'Warning - trajectory data on', fnTraj, 'may be corrupted.'
        print '    Frame count in header :', trajHeader[0]
        print '    # of bodies in header :', trajHeader[1]
        print '    # of data rows        :', len(trajData), '( should be', trajHeader[0]*trajHeader[1], ')'
    del traj
    return trajHeader, trajData

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print 'PymPlayer: Simulation result player   - 2010 Geoyeob Kim'
        print '  Usage: python PymPlayer.py <traj config> <traj data> <sim data>'
        print
        sys.exit(-1)
    
    fnRbConf = sys.argv[1]
    fnTraj   = sys.argv[2]
    fnSim    = sys.argv[3]
    
    gBiped = BipedParameter(fnRbConf)
    bipHeight = gBiped.getBipedHeight()
    print 'Biped Height =', bipHeight
    
    trajHeader, trajData = loadTrajData(fnTraj)
    simHeader, simData = loadTrajData(fnSim)
    
    nTrajFrame = len(trajData) / trajHeader[1]
    nSimFrame  = len(simData)  / simHeader[1]
    assert nTrajFrame >= nSimFrame

    print 'Initialize a rigid body with initial conditions...'
    bipedParam = gBiped
    plist = bipedParam.buildBody('EXP')
    flist = bipedParam.buildFiber([b.name for b in plist])
    print '# of rigid bodies  =', len(plist)
    print '# of muscle fibers =', len(flist)
    h = GetSimTimeStep()
    
    try:
       
        lastStateFile = open(camdataFn, 'r')
        lastState = cPickle.load(lastStateFile)
        gCameraMat, gRot, gSlowVideo = lastState
        lastStateFile.close()
        print camdataFn, ': Last camera state loaded successfully.'
    except:
        print camdataFn, ': Warning - No last camera state file found or data corrupted.'

    Main()
