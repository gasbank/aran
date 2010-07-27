#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Trajectory player
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
from BipedParameter import BipedParameter, RigidBodyFromRbConf

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
    #glLineWidth(0.1)
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

def DrawRigidBodies(color, nb, data, offset, rbNamingConvention):
    glColor(color[0], color[1], color[2])
    for i in xrange(nb):
        # Since the Naming convention
        # between Blender and the simulator(tracker)
        # we need to distinguish them using a flag variable. 
        if rbNamingConvention == 'BLENDER':
            name = gBiped.nameList[i]
            anotherName = gBiped.rbConf[name].anotherName
            if anotherName == '*':
                continue
        elif rbNamingConvention == 'SIMULATOR':
            anotherName = gBiped.anotherNameList[i]
            if anotherName not in ['trunk', 'head']:
                # Remove side suffix. ('L' or 'R')
                anotherName = anotherName[:-1]
        else:
            assert False
        
        qi = data[(gCurFrame + offset)*nb + i]
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
    
    if gDrawTraj:
        color  = (1, 0, 0)
        nb     = trajHeader[1]
        DrawRigidBodies(color, nb, trajData, trajFrameOffset, 'BLENDER')
    if gDrawSim and not cmdopt.nosim:
        color  = (1, 1, 1)
        nb     = simHeader[1]
        DrawRigidBodies(color, nb, simData, simFrameOffset, 'SIMULATOR')
            
    if gDrawSim or gDrawTraj:
        gCurFrame += 1
        if gCurFrame >= maxFrame:
            print 'rewind'
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

class PlayerOptions:
    def __init__(self):
        self.trajconf = None
        self.trajdata = None
        self.simdata  = None
        self.nosim    = False

def ParseCommandLine(argv):
    mvo = PlayerOptions()
    mvo.trajconf = argv[1]
    for av in argv[2:]:
        avparts = av.partition('=')
        if avparts[0] == '--trajdata':
            mvo.trajdata  = avparts[2]
        elif avparts[0] == '--simdata':
            mvo.output = avparts[2]
        elif avparts[0] == '--nosim':
            mvo.nosim = True
        else:
            print 'Error - command line argument', av, 'unknown.'
            raise Exception
        
    if not mvo.nosim:
        if mvo.trajdata is None and mvo.simdata is None:
            # We use both of traj and sim data and the file names
            # determined by traj conf file name automatically.
            fnRbConf = mvo.trajconf
            assert fnRbConf[fnRbConf.find('.'):] == '.traj.conf'
            mvo.trajdata = fnRbConf[:fnRbConf.find('.')] + '.traj_EXP_q.txt'
            mvo.simdata  = fnRbConf[:fnRbConf.find('.')] + '.sim_EXP_q.txt'
        elif (mvo.trajdata is None and mvo.simdata is not None) or (mvo.trajdata is not None and mvo.simdata is None):
            print 'Error - trajdata and simdata should be provided in a pair.'
            return None
    else:
        if mvo.trajdata is None:
            # trajdata can be determined by traj conf file name
            fnRbConf = mvo.trajconf
            assert fnRbConf[fnRbConf.find('.'):] == '.traj.conf'
            mvo.trajdata = fnRbConf[:fnRbConf.find('.')] + '.traj_EXP_q.txt'
        if mvo.simdata is not None:
            print 'Warn - nosim flag used. simdata will be ignored.'
    return mvo

if __name__ == '__main__':
    print 'PymPlayer: Trajectory player      -- 2010 Geoyeob Kim'
    if len(sys.argv) < 2 or '--help' in sys.argv[1:]:
        print '  Usage: python PymPlayer.py <traj conf> [Options]'
        print
        print '  Options:'
        print '    --trajdata=<path>    : trajectory data file (.txt)'
        print '    --simdata=<path>     : simulation data file (.txt)'
        print '    --nosim              : only use trajectory data file.'
        print '                           --simdata will be ignored.'
        print
        sys.exit(-1)
    cmdopt = ParseCommandLine(sys.argv)
    
    fnRbConf = cmdopt.trajconf
    fnTraj   = cmdopt.trajdata
    fnSim    = cmdopt.simdata
    print 'trajconf :', fnRbConf
    print 'trajdata :', fnTraj
    if fnSim:
        print 'simdata  :', fnSim
    
    gBiped = BipedParameter(fnRbConf)
    bipHeight = gBiped.getBipedHeight()
    print 'Biped Height =', bipHeight
    
    trajHeader, trajData = loadTrajData(fnTraj)
    nTrajFrame = len(trajData) / trajHeader[1]
    if nTrajFrame <= 2:
        print "Error - trajectory data should have three or more frames"
        print '        since the first two frames are used as an initial condition'
        print '        of simulated biped.'
        print 'Aborted.'
        sys.exit(-5)
    print '# of trajectory frames :', nTrajFrame
    if fnSim:
        simHeader, simData = loadTrajData(fnSim)
        nSimFrame  = len(simData)  / simHeader[1]
        assert nSimFrame > 0
        print '# of simulation frames :', nSimFrame

    print 'Initialize a rigid body with initial conditions...'
    bipedParam = gBiped
    plist = bipedParam.buildBody('EXP')
    flist = bipedParam.buildFiber([b.name for b in plist])
    print '# of rigid bodies  :', len(plist)
    print '# of muscle fibers :', len(flist)
    h = GetSimTimeStep()
    
    try:
        lastStateFile = open(camdataFn, 'r')
        lastState = cPickle.load(lastStateFile)
        gCameraMat, gRot, gSlowVideo = lastState
        lastStateFile.close()
        print camdataFn, ': Last camera state loaded successfully.'
    except:
        print camdataFn, ': Warning - No last camera state file found or data corrupted.'

    # We need 2 offset frames in trajectory
    # since the tracker uses first two frames as
    # initial conditions.
    trajFrameOffset = 2
    simFrameOffset  = 0

    maxFrame = nTrajFrame - trajFrameOffset
    if fnSim:
        maxFrame = min(maxFrame, nSimFrame - simFrameOffset)
    Main()
