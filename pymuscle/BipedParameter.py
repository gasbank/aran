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
import OpenGL
OpenGL.ERROR_CHECKING = False
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
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
import RigidBody, MuscleFiber
from GlobalContext import *
import ctypes as ct
from WriteSimcoreConfFile import *

ESCAPE = '\033'


def RigidBodyFromRbConf(fields):
    name       = fields[0]            # Blender side mesh name
    dimX       = float(fields[1])
    dimY       = float(fields[2])
    dimZ       = float(fields[3])
    xAxis      = fields[4]
    yAxis      = fields[5]
    boneLength = float(fields[6])
    mass       = 1.0
    boxsize    = (dimX,dimY,dimZ)
    q          = array([0,0,0,0,0,0])
    qd         = array([0,0,0,0,0,0])
    dc         = array([0.1,0.2,0.3])
    rotParam   = 'EXP'
    anotherName= fields[7]            # Simulator side rigid body name
    rb         = RigidBody.RigidBody(name, None, mass, boxsize, q, qd, dc, rotParam)
    # Two additional attributes added
    rb.xAxis   = xAxis
    rb.yAxis   = yAxis
    rb.anotherName = anotherName
    return rb

class BipedParameter:
    def __init__(self, fnRbConf = None):
        self.p = {}
        self.direction = {}
        
        self.direction['soleLen']        = 1
        self.direction['soleHeight']     = 2
        self.direction['soleWidth']      = 0
        self.direction['toeLen']         = 1
        self.direction['toeHeight']      = 2
        self.direction['toeWidth']       = 0
        self.direction['calfLen']        = 2
        self.direction['calfWidth']      = 0
        self.direction['calfLatWidth']   = 1
        self.direction['thighLen']       = 2
        self.direction['thighWidth']     = 0
        self.direction['thighLatWidth']  = 1
        self.direction['trunkLen']       = 2
        self.direction['trunkWidth']     = 0
        self.direction['trunkLatWidth']  = 1
        self.direction['headLen']        = 2
        self.direction['headWidth']      = 0
        self.direction['headLatWidth']   = 1
        self.direction['uarmLen']        = 2 # uarm : Upper arm
        self.direction['uarmWidth']      = 0
        self.direction['uarmLatWidth']   = 1
        self.direction['larmLen']        = 2 # larm : Lower arm
        self.direction['larmWidth']      = 0
        self.direction['larmLatWidth']   = 1
                    
        # NOTE: set this in the unit of centimeters.
        # 1. Body dimension

        self.p['soleLen']           = 20.
        self.p['soleHeight']        = 8.
        self.p['soleWidth']         = 15.

        self.p['toeLen']            = 15.
        self.p['toeHeight']         = 5.
        self.p['toeWidth']          = 12.

        self.p['calfLen']           = 50.
        self.p['calfWidth']         = 8.
        self.p['calfLatWidth']      = 8.

        self.p['thighLen']          = 55.
        self.p['thighWidth']        = 12.
        self.p['thighLatWidth']     = 12.

        self.p['trunkLen']          = 70.
        self.p['trunkWidth']        = 45.
        self.p['trunkLatWidth']     = 25.
        
        self.p['headLen']           = 19.
        self.p['headWidth']         = 22.
        self.p['headLatWidth']      = 18.
        
        self.p['uarmLen']           = 45.
        self.p['uarmWidth']         = 8.
        self.p['uarmLatWidth']      = 8.
        
        self.p['larmLen']           = 43.
        self.p['larmWidth']         = 7.
        self.p['larmLatWidth']      = 7.
        
        if fnRbConf:
            rbConf = {}
            rbConf_file = open(fnRbConf, 'r')
            self.anotherNameList = []
            self.nameList = []
            for line in rbConf_file:
                fields = line.split()
                #print fields
                rb = RigidBodyFromRbConf(fields)
                rbConf[rb.name] = rb
                self.nameList.append(rb.name) # Blender name list
                if rb.anotherName != '*':
                    self.anotherNameList.append(rb.anotherName)
            print '# of RB from', fnRbConf, ':', len(rbConf)
            assert len(rbConf) > 0
            # Body name correspondence
            bnCorres = {}
            bnCorres['trunk'] = 'Chest'
            bnCorres['head' ] = 'Head'
            bnCorres['uarm' ] = 'LeftShoulder'
            bnCorres['larm' ] = 'LeftElbow'
            bnCorres['thigh'] = 'LeftHip'
            bnCorres['calf' ] = 'LeftKnee'
            bnCorres['sole' ] = 'LeftAnkle'
            bnCorres['toe'  ] = 'LeftToe'
            
            self.direction['soleLen']        = 1
            self.direction['soleHeight']     = 2
            self.direction['soleWidth']      = 0
            self.direction['toeLen']         = 1
            self.direction['toeHeight']      = 2
            self.direction['toeWidth']       = 0
            self.direction['calfLen']        = 1
            self.direction['calfWidth']      = 0
            self.direction['calfLatWidth']   = 2
            self.direction['thighLen']       = 1
            self.direction['thighWidth']     = 0
            self.direction['thighLatWidth']  = 2
            self.direction['trunkLen']       = 1
            self.direction['trunkWidth']     = 0
            self.direction['trunkLatWidth']  = 2
            self.direction['headLen']        = 1
            self.direction['headWidth']      = 0
            self.direction['headLatWidth']   = 2
            self.direction['uarmLen']        = 1 # uarm : Upper arm
            self.direction['uarmWidth']      = 2
            self.direction['uarmLatWidth']   = 0
            self.direction['larmLen']        = 1 # larm : Lower arm
            self.direction['larmWidth']      = 2
            self.direction['larmLatWidth']   = 0
            
            
            self.p[ 'soleLen']       = rbConf[ bnCorres['sole'] ].boxsize[ self.direction['soleLen']    ]
            self.p[ 'soleHeight']    = rbConf[ bnCorres['sole'] ].boxsize[ self.direction['soleHeight'] ]
            self.p[ 'soleWidth']     = rbConf[ bnCorres['sole'] ].boxsize[ self.direction['soleWidth']  ]
            
            self.p[ 'toeLen']       = rbConf[ bnCorres['toe'] ].boxsize[ self.direction['toeLen']    ]
            self.p[ 'toeHeight']    = rbConf[ bnCorres['toe'] ].boxsize[ self.direction['toeHeight'] ]
            self.p[ 'toeWidth']     = rbConf[ bnCorres['toe'] ].boxsize[ self.direction['toeWidth']  ]
            
            self.p[ 'calfLen']       = rbConf[ bnCorres['calf'] ].boxsize[ self.direction['calfLen']      ]
            self.p[ 'calfWidth']     = rbConf[ bnCorres['calf'] ].boxsize[ self.direction['calfWidth']    ]
            self.p[ 'calfLatWidth']  = rbConf[ bnCorres['calf'] ].boxsize[ self.direction['calfLatWidth'] ]
            
            self.p[ 'thighLen']       = rbConf[ bnCorres['thigh'] ].boxsize[ self.direction['thighLen']      ]
            self.p[ 'thighWidth']     = rbConf[ bnCorres['thigh'] ].boxsize[ self.direction['thighWidth']    ]
            self.p[ 'thighLatWidth']  = rbConf[ bnCorres['thigh'] ].boxsize[ self.direction['thighLatWidth'] ]
            
            self.p[ 'trunkLen']       = rbConf[ bnCorres['trunk'] ].boxsize[ self.direction['trunkLen']      ]
            self.p[ 'trunkWidth']     = rbConf[ bnCorres['trunk'] ].boxsize[ self.direction['trunkWidth']    ]
            self.p[ 'trunkLatWidth']  = rbConf[ bnCorres['trunk'] ].boxsize[ self.direction['trunkLatWidth'] ]
            
            self.p[ 'headLen']       = rbConf[ bnCorres['head'] ].boxsize[ self.direction['headLen']      ]
            self.p[ 'headWidth']     = rbConf[ bnCorres['head'] ].boxsize[ self.direction['headWidth']    ]
            self.p[ 'headLatWidth']  = rbConf[ bnCorres['head'] ].boxsize[ self.direction['headLatWidth'] ]
            
            self.p[ 'uarmLen']       = rbConf[ bnCorres['uarm'] ].boxsize[ self.direction['uarmLen']      ]
            self.p[ 'uarmWidth']     = rbConf[ bnCorres['uarm'] ].boxsize[ self.direction['uarmWidth']    ]
            self.p[ 'uarmLatWidth']  = rbConf[ bnCorres['uarm'] ].boxsize[ self.direction['uarmLatWidth'] ]
            
            self.p[ 'larmLen']       = rbConf[ bnCorres['larm'] ].boxsize[ self.direction['larmLen']      ]
            self.p[ 'larmWidth']     = rbConf[ bnCorres['larm'] ].boxsize[ self.direction['larmWidth']    ]
            self.p[ 'larmLatWidth']  = rbConf[ bnCorres['larm'] ].boxsize[ self.direction['larmLatWidth'] ]
            
            self.rbConf   = rbConf
            self.bnCorres = bnCorres



        # 2. Body gap (positive) or overlap (negative)
        #    Generally, overlapped one gives stable equilibrium condition
        self.p['trunkThighGap']     = self.p['thighLen']/5
        self.p['trunkHeadGap']      = self.p['headLen']/2
        self.p['thighCalfGap']      = self.p['calfLen']/5
        self.p['calfSoleGap']       = self.p['soleHeight']/5
        self.p['soleToeGap']        = self.p['soleLen']/5
        self.p['trunkUarmGap']      = self.p['trunkWidth']/10
        self.p['uarmLarmGap']       = self.p['uarmLen']/10
        

        # 3. Other
        self.p['legsDist']          = self.p['trunkWidth']/1.5
        self.p['armDist']           = self.p['trunkLen']/2 * 0.8


        # 4. Muscle Attachment Parameters
        self.p['hipMuscleDist']       = self.p['trunkWidth']/2
        self.p['hipMuscleLatDist']    = 14.
        self.p['hipMuscleDist2']      = self.p['thighWidth']
        self.p['hipMuscleLatDist2']   = self.p['thighLatWidth']
        self.p['hipMuscleInsLen']     = 4.

        self.p['ankleMuscleDist']     = self.p['calfWidth']
        self.p['ankleMuscleLatDist']  = self.p['calfLatWidth']
        self.p['ankleMuscleDist2']    = self.p['soleWidth']
        self.p['ankleMuscleLatDist2'] = self.p['soleWidth']
        self.p['ankleMuscleInsLen']   = 0

        # Convert the unit from centimeters to meters
        if fnRbConf is None:
            for (k, v) in self.p.iteritems():
                self.p[k] = v/100.

        self._pos = { 'trunk' : self.getTrunkPos,
                      'thigh' : self.getThighPos,
                      'calf'  : self.getCalfPos,
                      'sole'  : self.getSolePos,
                      'toe'   : self.getToePos,
                      'uarm'  : self.getUarmPos,
                      'larm'  : self.getLarmPos,
                      'head'  : self.getHeadPos  }
        self._liga = { 'neck' : self.getNeckLigaments,
                       'sho'  : self.getShoulderLigaments,
                       'elb'  : self.getElbowLigaments,
                       'hip'  : self.getHipLigaments,
                       'knee' : self.getKneeLigaments,
                       'ankle': self.getAnkleLigaments,
                       'toe'  : self.getToeLigaments      }
    def __getitem__(self, i):
        return self.p[i]
    
    def reportLigaments(self):
        for (k,v) in self._liga.iteritems():
            print '%10s - %3d' % (k, len(v()))

    def getBipedHeight(self):
        return   self.p['headLen']       \
               + self.p['trunkHeadGap']  \
               + self.p['trunkLen']      \
               + self.p['trunkThighGap'] \
               + self.p['thighLen']      \
               + self.p['thighCalfGap']  \
               + self.p['calfLen']       \
               + self.p['calfSoleGap']   \
               + self.p['soleHeight']    \

    def getTrunkPos(self):
        return array([0,
                      0,
                      self.getBipedHeight() - self.p['headLen'] - self.p['trunkHeadGap'] - self.p['trunkLen']/2])
    def getHeadPos(self):
        return array([0,
                      0,
                      self.getBipedHeight() - self.p['headLen']/2])
    def getUarmPos(self):
        trunkPos = self.getTrunkPos()
        return array([trunkPos[0] + self.p['trunkWidth']/2 + self.p['trunkUarmGap'] + self.p['uarmWidth']/2,
                      trunkPos[1] + 0,
                      trunkPos[2] + self.p['armDist'] - self.p['uarmLen']/2])
    def getLarmPos(self):
        pPos = self.getUarmPos()
        return array([pPos[0] + 0,
                      pPos[1] + 0,
                      pPos[2] - self.p['uarmLen']/2 - self.p['uarmLarmGap'] - self.p['larmLen']/2 ])    
    def getThighPos(self):
        return array([self.p['legsDist']/2,
                      0,
                      self.p['thighLen']/2
                      + self.p['thighCalfGap']
                      + self.p['calfLen']
                      + self.p['calfSoleGap']
                      + self.p['soleHeight']])
    def getCalfPos(self):
        return array([self.p['legsDist']/2,
                      0,
                      self.p['calfLen']/2
                      + self.p['calfSoleGap']
                      + self.p['soleHeight']])
    def getSolePos(self):
        return array([self.p['legsDist']/2,
                      0,
                      self.p['soleHeight']/2])
    def getToePos(self):
        return array([self.p['legsDist']/2,
                      -self.p['soleLen']/2-self.p['soleToeGap']-self.p['toeLen']/2,
                      self.p['toeHeight']/2])
    def getBipedCenter(self):
        return array([0,0,self.getBipedHeight()/2])
    def getNeckJointCenter(self):
        return self.getTrunkPos() + array([0,0,self['trunkLen']/2 + self['trunkHeadGap']/2])
    def getHipJointCenter(self):
        return self.getThighPos() + array([0,0,self['thighLen']/2+self['trunkThighGap']/2])
    def getShoulderJointCenter(self):
        return self.getTrunkPos() + array([self['trunkWidth']/2 + self['trunkUarmGap']/2,
                                           0,
                                           self['armDist'] ])
    def getElbowJointCenter(self):
        return self.getUarmPos() + array([0,
                                          0,
                                          -self['uarmLen']/2 - self['uarmLarmGap']/2 ])
    def getKneeJointCenter(self):
        return self.getCalfPos() + array([0,0,self['calfLen']/2+self['thighCalfGap']/2])
    def getAnkleJointCenter(self):
        return self.getSolePos() + array([0,0,self['soleHeight']/2+self['calfSoleGap']/2])
    def getToeJointCenter(self):
        return self.getToePos() + array([0,self['toeLen']/2+self['soleToeGap']/2,0])


    def getXyPoints(self, c, wx, wy):
        ret = []
        for i in range(3):
            for j in range(3):
                ret.append( c + array([(j-1)*(wx/2), (i-1)*(wy/2), 0]) )
        return ret
    def getXzPoints(self, c, wx, wz):
        ret = []
        for i in range(3):
            for j in range(3):
                ret.append( c + array([(j-1)*(wx/2), 0, (i-1)*(wz/2)]) )
        return ret
    def getYzPoints(self, c, wy, wz):
        ret = []
        for i in range(3):
            for j in range(3):
                ret.append( c + array([0, (j-1)*(wy/2), (i-1)*(wz/2)]) )
        return ret
    def getFrontalLigaments(self,
                             ligamentPrefix,
                             c, gap, 
                             b1, b1w, b1lw, b1zoff,
                             b2, b2w, b2lw, b2zoff,
                             subset
                             ):
        # B1 attached points
        xyp = [self.getXzPoints(c, b1w, b1lw)[i] for i in subset]
        for xypi in xyp: xypi[1] += gap/2 + b1zoff
        p = xyp
        p_cen = c + array([0, gap/2, 0])
        # B2 attached points
        xyp = [self.getXzPoints(c, b2w, b2lw)[i] for i in subset]
        for xypi in xyp: xypi[1] += -gap/2 + b2zoff
        v = xyp
        v_cen = c + array([0, -gap/2, 0])
        ret = []
        idx = 1
        for pi in p:
            for vi in v:
                ret.append( (ligamentPrefix + str(idx) + 'L',
                             pi, vi, b1, b2) )
                idx += 1
        ret.append( (ligamentPrefix + str(idx) + 'CenL',
                     p_cen, v_cen, b1, b2) )
        return ret
    def getLateralLigaments(self,
                             ligamentPrefix,
                             c, gap, 
                             b1, b1w, b1lw, b1zoff,
                             b2, b2w, b2lw, b2zoff,
                             subset
                             ):
        # B1 attached points
        xyp = [self.getYzPoints(c, b1w, b1lw)[i] for i in subset]
        for xypi in xyp: xypi[0] += gap/2 + b1zoff
        p = xyp
        p_cen = c + array([gap/2, 0, 0])
        # B2 attached points
        xyp = [self.getYzPoints(c, b2w, b2lw)[i] for i in subset]
        for xypi in xyp: xypi[0] += -gap/2 + b2zoff
        v = xyp
        v_cen = c + array([gap/2, 0, 0])
        ret = []
        idx = 1
        for pi in p:
            for vi in v:
                ret.append( (ligamentPrefix + str(idx) + 'L',
                             pi, vi, b1, b2) )
                idx += 1
        ret.append( (ligamentPrefix + str(idx) + 'CenL',
                     p_cen, v_cen, b1, b2) )
        return ret
    def getVerticalLigaments(self,
                             ligamentPrefix,
                             c, gap, 
                             b1, b1w, b1lw, b1zoff,
                             b2, b2w, b2lw, b2zoff,
                             subset
                             ):
        # B1 (body 1) should be posed in higher place than B2 (body 2)
        # e.g. Thigh as B1, Calf as B2 ---> OK
        #      Larm as B1, Uarm as B2  ---> Error
        # B1 attached points
        xyp = [self.getXyPoints(c, b1w, b1lw)[i] for i in subset]
        for xypi in xyp: xypi[2] += gap/2 + b1zoff
        p = xyp
        p_cen = c + array([0, 0, gap/2])
        # B2 attached points
        xyp = [self.getXyPoints(c, b2w, b2lw)[i] for i in subset]
        for xypi in xyp: xypi[2] += -gap/2 + b2zoff
        v = xyp
        v_cen = c + array([0, 0, -gap/2])
        ret = []
        idx = 1
        for pi in p:
            for vi in v:
                ret.append( (ligamentPrefix + str(idx) + 'L',
                             pi, vi, b1, b2) )
                idx += 1
        ret.append( (ligamentPrefix + str(idx) + 'CenL',
                     p_cen, v_cen, b1, b2) )
        return ret
    #
    # LIGAMENTS
    #
    def getNeckLigaments_Subset(self, subset):
        c = self.getNeckJointCenter()
        trhg = self['trunkHeadGap']
        # neck attached points
        hw   = self['headWidth']
        hlw  = self['headLatWidth']
        # trunk attached points
        trw  = self['trunkWidth']
        trlw = self['trunkLatWidth']
        
        return self.getVerticalLigaments('neckLiga', c, trhg,
                                         'head', hw, hlw, 0,
                                         'trunk', trw, trlw, 0,
                                         subset)
    def getNeckLigaments_FullyConnected(self):
        return self.getNeckLigaments_Subset([1,3,5,7])
    def getNeckLigaments_Triangular(self):
        return self.getNeckLigaments_Subset([1,6,8])
    def getNeckLigaments(self):
        return self.getNeckLigaments_Triangular()
    def getNeckLigaments_Original(self):
        c = self.getNeckJointCenter()
        p1 = c + array([0, 0, -self['trunkHeadGap']/2])
        p2 = c + array([0, 0,  self['trunkHeadGap']/2])
        
        p = [ c + array([  self['trunkWidth']/2,  self['trunkLatWidth']/2, -self['trunkHeadGap']/2]),
              c + array([  self['trunkWidth']/2, -self['trunkLatWidth']/2, -self['trunkHeadGap']/2]),
              c + array([ -self['trunkWidth']/2, -self['trunkLatWidth']/2, -self['trunkHeadGap']/2]),
              c + array([ -self['trunkWidth']/2,  self['trunkLatWidth']/2, -self['trunkHeadGap']/2]), ]
              
        v = [ c + array([  self['headWidth']/2,  self['headLatWidth']/2,  self['trunkHeadGap']/2]),
              c + array([  self['headWidth']/2, -self['headLatWidth']/2,  self['trunkHeadGap']/2]),
              c + array([ -self['headWidth']/2, -self['headLatWidth']/2,  self['trunkHeadGap']/2]),
              c + array([ -self['headWidth']/2,  self['headLatWidth']/2,  self['trunkHeadGap']/2]), ]
        
        ret = []
        ret.append( ('neckLiga1L', p1, p2, 'trunk', 'head') )
        idx = 2
        for pi in p:
            for vi in v:
                ret.append( ('neckLiga'+str(idx)+'L', pi, vi, 'trunk', 'head') )
                idx += 1
        return ret
    def getShoulderLigaments(self):
        c = self.getShoulderJointCenter()
        trug = self['trunkUarmGap']
        # uarm attached points
        uw   = self['uarmWidth']
        ulw  = self['uarmLatWidth']
        # trunk attached points
        trw  = self['uarmWidth']*2
        trlw = self['uarmLatWidth']*2
        
        return self.getLateralLigaments('shoLiga', c, trug,
                                         'uarmL', uw, ulw, 0,
                                         'trunk', trw, trlw, 0,
                                         [1,6,8])
    def getShoulderLigaments_Original(self):
        c = self.getShoulderJointCenter()
        p1 = c + array([-self['trunkUarmGap']/2, 0, 0])
        p2 = c + array([ self['trunkUarmGap']/2, 0, 0])
        
        p = [ c + array([-self['trunkUarmGap']/2,  self['uarmLatWidth'],  self['uarmWidth']]),
              c + array([-self['trunkUarmGap']/2,  self['uarmLatWidth'], -self['uarmWidth']]),
              c + array([-self['trunkUarmGap']/2, -self['uarmLatWidth'], -self['uarmWidth']]),
              c + array([-self['trunkUarmGap']/2, -self['uarmLatWidth'],  self['uarmWidth']])  ]
        v = [ c + array([ self['trunkUarmGap']/2,  self['uarmLatWidth']/2,  self['uarmWidth']/2]),
              c + array([ self['trunkUarmGap']/2,  self['uarmLatWidth']/2, -self['uarmWidth']/2]),
              c + array([ self['trunkUarmGap']/2, -self['uarmLatWidth']/2, -self['uarmWidth']/2]),
              c + array([ self['trunkUarmGap']/2, -self['uarmLatWidth']/2,  self['uarmWidth']/2])  ]
        
        ret = []
        ret.append( ('shoLiga1L', p1, p2, 'trunk', 'uarmL') )
        idx = 2
        for pi in p:
            for vi in v:
                ret.append( ('shoLiga'+str(idx)+'L', pi, vi, 'trunk', 'uarmL') )
                idx += 1
        return ret
        
    def getElbowLigaments(self):
        c = self.getElbowJointCenter()
        ulg = self['uarmLarmGap']
        # uarm attached points
        uw   = self['uarmWidth']
        ulw  = self['uarmLatWidth']
        # larm attached points
        lw  = self['larmWidth']
        llw = self['larmLatWidth']
        
        return self.getVerticalLigaments('elbLiga', c, ulg,
                                         'uarmL', uw, ulw, 0,
                                         'larmL', lw, llw, 0,
                                         [2,3,8])
    def getElbowLigaments_Original(self):
        c = self.getElbowJointCenter()
        p1 = c + array([0, 0,  self['uarmLarmGap']/2])
        p2 = c + array([0, 0, -self['uarmLarmGap']/2])
        
        p = [ c + array([ self['uarmWidth']/2,  self['uarmLatWidth']/2,  self['uarmLarmGap']/2]),
              c + array([ self['uarmWidth']/2, -self['uarmLatWidth']/2,  self['uarmLarmGap']/2]),
              c + array([-self['uarmWidth']/2, -self['uarmLatWidth']/2,  self['uarmLarmGap']/2]),
              c + array([-self['uarmWidth']/2,  self['uarmLatWidth']/2,  self['uarmLarmGap']/2])  ]
        v = [ c + array([ self['larmWidth']/2,  self['larmLatWidth']/2, -self['uarmLarmGap']/2]),
              c + array([ self['larmWidth']/2, -self['larmLatWidth']/2, -self['uarmLarmGap']/2]),
              c + array([-self['larmWidth']/2, -self['larmLatWidth']/2, -self['uarmLarmGap']/2]),
              c + array([-self['larmWidth']/2,  self['larmLatWidth']/2, -self['uarmLarmGap']/2])  ]
        ret = []
        ret.append( ('elbLiga1L', p1, p2, 'uarmL', 'larmL') )
        idx = 2
        for pi in p:
            for vi in v:
                ret.append( ('elbLiga'+str(idx)+'L', pi, vi, 'uarmL', 'larmL') )
                idx += 1
        return ret


    def getHipLigaments_Subset(self, subset):
        c = self.getHipJointCenter()
        trtg = self['trunkThighGap']
        # trunk attached points
        trw  = self['hipMuscleDist']
        trlw = self['trunkLatWidth']
        # thighL attached points
        tw  = self['thighWidth']
        tlw = self['thighLatWidth']
        
        return self.getVerticalLigaments('hipLiga', c, trtg,
                                         'trunk', trw, trlw, 0,
                                         'thighL', tw, tlw, 0,
                                         subset)
    def getHipLigaments_FullyConnected(self):
        return self.getHipLigaments_Subset([1,3,5,7])
    def getHipLigaments_Triangular(self):
        return self.getHipLigaments_Subset([1,6,8])
    def getHipLigaments(self):
        return self.getHipLigaments_Triangular()
    def getHipLigaments_Original(self):
        c = self.getHipJointCenter()
        p1 = c + array([0, 0, self['trunkThighGap']/2])
        p2 = c + array([0, 0, -self['trunkThighGap']/2])
        
        c1 = c + array([1.01, 1.01, self['trunkThighGap']/2])
        c2 = c + array([1.01, 1.01, -self['trunkThighGap']/2])
        
        c3 = c + array([1.01,-1.01, self['trunkThighGap']/2])
        c4 = c + array([1.01,-1.01, -self['trunkThighGap']/2])
        
        c5 = c + array([-1.01, 1.01, self['trunkThighGap']/2])
        c6 = c + array([-1.01, 1.01, -self['trunkThighGap']/2])
        
        c7 = c + array([-1.01, -1.01, self['trunkThighGap']/2])
        c8 = c + array([-1.01, -1.01, -self['trunkThighGap']/2])
        
        
        p3  = c + array([0,                          -self['trunkLatWidth']/2,  self['trunkThighGap']/2])
        p4  = c + array([0,                          -self['thighLatWidth']/2, -self['trunkThighGap']/2])
        p5  = c + array([0,                           self['trunkLatWidth']/2,  self['trunkThighGap']/2])
        p6  = c + array([0,                           self['thighLatWidth']/2, -self['trunkThighGap']/2])
        p7  = c + array([self['hipMuscleDist']/4,                           0,  self['trunkThighGap']/2])
        p8  = c + array([self['hipMuscleDist2']/2,                          0, -self['trunkThighGap']/2])
        p9  = c + array([-self['hipMuscleDist']/4,                          0,  self['trunkThighGap']/2])
        p10 = c + array([-self['hipMuscleDist2']/2,                         0, -self['trunkThighGap']/2])
        return [ ('hipLiga1L', p1,p2, 'trunk', 'thighL'),
			     #('hipLiga1L', c1,c2, 'trunk', 'thighL'),
			     #('hipLiga1L', c3,c4, 'trunk', 'thighL'),
			     #('hipLiga1L', c5,c6, 'trunk', 'thighL'),
			     #('hipLiga1L', c7,c8, 'trunk', 'thighL'),
                 
                 ('hipLiga2L', p3,p4, 'trunk', 'thighL'),
                 ('hipLiga3L', p3,p6, 'trunk', 'thighL'),
                 ('hipLiga4L', p3,p8, 'trunk', 'thighL'),
                 ('hipLiga5L', p3,p10, 'trunk', 'thighL'),
                 ('hipLiga2L', p5,p4, 'trunk', 'thighL'),
                 ('hipLiga3L', p5,p6, 'trunk', 'thighL'),
                 ('hipLiga4L', p5,p8, 'trunk', 'thighL'),
                 ('hipLiga5L', p5,p10, 'trunk', 'thighL'),
                 ('hipLiga2L', p7,p4, 'trunk', 'thighL'),
                 ('hipLiga3L', p7,p6, 'trunk', 'thighL'),
                 ('hipLiga4L', p7,p8, 'trunk', 'thighL'),
                 ('hipLiga5L', p7,p10, 'trunk', 'thighL'),
                 ('hipLiga2L', p9,p4, 'trunk', 'thighL'),
                 ('hipLiga3L', p9,p6, 'trunk', 'thighL'),
                 ('hipLiga4L', p9,p8, 'trunk', 'thighL'),
                 ('hipLiga5L', p9,p10, 'trunk', 'thighL') ]
    
    def getKneeLigaments_Subset(self, subset):
        c = self.getKneeJointCenter()
        tcg = self['thighCalfGap']
        # thighL attached points
        tw  = self['thighWidth']
        tlw = self['thighLatWidth']
        # calfL attached points
        cw  = self['calfWidth']
        clw = self['calfLatWidth']
        return self.getVerticalLigaments('kneeLiga', c, tcg,
                                         'thighL', tw, tlw, 0,
                                         'calfL', cw, clw, 0,
                                         subset)
    def getKneeLigaments_FullyConnected(self):
        return self.getKneeLigaments_Subset([1,3,5,7])
    def getKneeLigaments_Triangular(self):
        return self.getKneeLigaments_Subset([1,6,8])
    def getKneeLigaments(self):
        return self.getKneeLigaments_Triangular()
    def getAnkleLigaments_Subset(self, subset):
        c = self.getAnkleJointCenter()
        csg = self['calfSoleGap']
        # calfL attached points
        cw  = self['calfWidth']
        clw = self['calfLatWidth']
        # soleL attached points
        sw   = self['soleWidth']
        slen = self['soleLen']
        return self.getVerticalLigaments('ankleLiga', c, csg,
                                         'calfL', cw, clw, 0,
                                         'soleL', sw, slen, 0,
                                         subset)
    def getAnkleLigaments_FullyConnected(self):
        return self.getAnkleLigaments_Subset([1,3,5,7])
    def getAnkleLigaments_Triangular(self):
        return self.getAnkleLigaments_Subset([1,6,8])
    def getAnkleLigaments(self):
        return self.getAnkleLigaments_Triangular()
    def getAnkleLigaments_Original(self):
        # Inclined (/ \)
        c = self.getAnkleJointCenter()
        p1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
        p2 = c + array([ self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
        p3 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
        p4 = c + array([ self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])

        p5 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
        p6 = c + array([-self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
        p7 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2])
        p8 = c + array([-self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])

        return [ ('ankleLiga1L', p1,p2, 'calfL', 'soleL'),
                 ('ankleLiga2L', p1,p4, 'calfL', 'soleL'),
                 ('ankleLiga2L', p1,p6, 'calfL', 'soleL'),
                 ('ankleLiga2L', p1,p8, 'calfL', 'soleL'),
                 ('ankleLiga1L', p3,p2, 'calfL', 'soleL'),
                 ('ankleLiga2L', p3,p4, 'calfL', 'soleL'),
                 ('ankleLiga2L', p3,p6, 'calfL', 'soleL'),
                 ('ankleLiga2L', p3,p8, 'calfL', 'soleL'),
                 ('ankleLiga1L', p5,p2, 'calfL', 'soleL'),
                 ('ankleLiga2L', p5,p4, 'calfL', 'soleL'),
                 ('ankleLiga2L', p5,p6, 'calfL', 'soleL'),
                 ('ankleLiga2L', p5,p8, 'calfL', 'soleL'),
                 ('ankleLiga1L', p7,p2, 'calfL', 'soleL'),
                 ('ankleLiga2L', p7,p4, 'calfL', 'soleL'),
                 ('ankleLiga2L', p7,p6, 'calfL', 'soleL'),
                 ('ankleLiga2L', p7,p8, 'calfL', 'soleL')
                  ]
    
    def getToeLigaments_Subset(self, subset):
        c = self.getToeJointCenter()
        stg = self['soleToeGap']
        # soleL attached points
        sw = self['soleWidth']
        sh = self['soleHeight']
        # toeL attached points
        tw = self['toeWidth']
        th = self['toeHeight']
        return self.getFrontalLigaments('toeLiga', c, stg,
                                         'soleL', sw, sh, 0,
                                         'toeL', tw, th, 0,
                                         subset)
    def getToeLigaments_FullyConnected(self):
        return self.getToeLigaments_Subset([1,3,5,7])
    def getToeLigaments_Triangular(self):
        return self.getToeLigaments_Subset([0,2,7])
    def getToeLigaments(self):
        return self.getToeLigaments_Triangular()
    def getToeLigaments_Original(self):
            c = self.getToeJointCenter()
            p1 = c + array([0, self['soleToeGap']/2, self['toeHeight']/2])
            p2 = c + array([0, self['soleToeGap']/2, -self['toeHeight']/2])
            p3 = c + array([0, -self['soleToeGap']/2, self['toeHeight']/2])
            p4 = c + array([0, -self['soleToeGap']/2, -self['toeHeight']/2])
    
            p5 = c + array([ self['toeWidth']/2, self['soleToeGap']/2, 0])
            p6 = c + array([-self['toeWidth']/2, self['soleToeGap']/2, 0])
            p7 = c + array([ self['toeWidth']/2, -self['soleToeGap']/2, 0])
            p8 = c + array([-self['toeWidth']/2, -self['soleToeGap']/2, 0])
    
            return [ ('toeLiga1L', p1,p4, 'soleL', 'toeL'),
                     ('toeLiga2L', p1,p3, 'soleL', 'toeL'),
                     ('toeLiga3L', p1,p7, 'soleL', 'toeL'),
                     ('toeLiga4L', p1,p8, 'soleL', 'toeL'),
                     ('toeLiga1L', p2,p4, 'soleL', 'toeL'),
                     ('toeLiga2L', p2,p3, 'soleL', 'toeL'),
                     ('toeLiga3L', p2,p7, 'soleL', 'toeL'),
                     ('toeLiga4L', p2,p8, 'soleL', 'toeL'),
                     ('toeLiga1L', p5,p4, 'soleL', 'toeL'),
                     ('toeLiga2L', p5,p3, 'soleL', 'toeL'),
                     ('toeLiga3L', p5,p7, 'soleL', 'toeL'),
                     ('toeLiga4L', p5,p8, 'soleL', 'toeL'),
                     ('toeLiga1L', p6,p4, 'soleL', 'toeL'),
                     ('toeLiga2L', p6,p3, 'soleL', 'toeL'),
                     ('toeLiga3L', p6,p7, 'soleL', 'toeL'),
                     ('toeLiga4L', p6,p8, 'soleL', 'toeL') ]
    
    #
    # Muscle fibers between trunk and thigh
    #
    def getBicepsFemoris(self):
        c1 = self.getHipJointCenter()
        c2 = self.getThighPos()
        p1 = c1 + array([0, -self['trunkLatWidth']/2, self['trunkThighGap']/2 + self['trunkLen']/2])
        p2 = c2 + array([0, -self['thighLatWidth']/2, -self['thighLen']/2])
        p3 = c1 + array([0, +self['trunkLatWidth']/2, self['trunkThighGap']/2 + self['trunkLen']/2])
        p4 = c2 + array([0, +self['thighLatWidth']/2, -self['thighLen']/2])
        p5 = c1 + array([-self['legsDist'], 0, self['trunkThighGap']/2 + self['trunkLen']/2])
        p6 = c2 + array([-self['thighWidth']/2, 0, -self['thighLen']/2])
        p7 = c1 + array([+self['legsDist']/2, 0, self['trunkThighGap']/2 + self['trunkLen']/2])
        p8 = c2 + array([+self['thighWidth']/2, 0, -self['thighLen']/2])
        return [ ('BicepsFemoris1L', p1,p2, 'trunk', 'thighL'),
                 ('BicepsFemoris2L', p1,p4, 'trunk', 'thighL'),
                 ('BicepsFemoris3L', p1,p6, 'trunk', 'thighL'),
                 ('BicepsFemoris4L', p1,p8, 'trunk', 'thighL'),
                 ('BicepsFemoris1L', p3,p2, 'trunk', 'thighL'),
                 ('BicepsFemoris2L', p3,p4, 'trunk', 'thighL'),
                 ('BicepsFemoris3L', p3,p6, 'trunk', 'thighL'),
                 ('BicepsFemoris4L', p3,p8, 'trunk', 'thighL'),
                 ('BicepsFemoris1L', p5,p2, 'trunk', 'thighL'),
                 ('BicepsFemoris2L', p5,p4, 'trunk', 'thighL'),
                 ('BicepsFemoris3L', p5,p6, 'trunk', 'thighL'),
                 ('BicepsFemoris4L', p5,p8, 'trunk', 'thighL'),
                 ('BicepsFemoris1L', p7,p2, 'trunk', 'thighL'),
                 ('BicepsFemoris2L', p7,p4, 'trunk', 'thighL'),
                 ('BicepsFemoris3L', p7,p6, 'trunk', 'thighL'),
                 ('BicepsFemoris4L', p7,p8, 'trunk', 'thighL')  ]
    #
    # Muscle fibers between trunk and calf
    #
    def getQuadricepsFemoris(self):
        c1 = self.getHipJointCenter()
        p = []
        p.append( c1 + array([0, -self['trunkLatWidth']/2, self['trunkThighGap']/2 + self['trunkLen']/2]) )
        p.append( c1 + array([0, +self['trunkLatWidth']/2, self['trunkThighGap']/2 + self['trunkLen']/2]) )
        p.append( c1 + array([-self['legsDist'], 0, self['trunkThighGap']/2 + self['trunkLen']/4]) )
        p.append( c1 + array([+self['legsDist']/2, 0, self['trunkThighGap']/2 + self['trunkLen']/4]) )
        c2 = self.getCalfPos()
        v = []
        v.append( c2 + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  0]) )
        v.append( c2 + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  0]) )
        v.append( c2 + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  0]) )
        v.append( c2 + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  0]) )
        ret = []
        idx = 0
        for pi in p:
        	for vi in v:
        		ret.append( ('QuadricepsFemoris'+str(idx)+'L', pi, vi, 'trunk', 'calfL') )
                idx += 1
        return ret
    #
    # Muscle fibers between thigh and sole
    #
    def getHamstring(self):
        c1 = self.getSolePos()
        c2 = self.getThighPos()
        p1 = c1 + array([ 0,  self['soleLen']/2, 0])
        p2 = c2 + array([ 0,  self['thighLatWidth']/2, -self['thighLen']/4])
        p3 = c1 + array([ 0,  -self['soleLen']/2, 0])
        p4 = c2 + array([ 0,  -self['thighLatWidth']/2, -self['thighLen']/4])
        
        p5 = c1 + array([ self['soleWidth']/2,  0, 0])
        p6 = c2 + array([ self['thighWidth']/2,  0, -self['thighLen']/4])
        p7 = c1 + array([ -self['soleWidth']/2,  0, 0])
        p8 = c2 + array([ -self['thighWidth']/2,  0, -self['thighLen']/4])
        return [ ('hamstring1L', p1,p2, 'soleL', 'thighL'),
                 ('hamstring2L', p1,p4, 'soleL', 'thighL'),
                 ('hamstring3L', p1,p6, 'soleL', 'thighL'),
                 ('hamstring4L', p1,p8, 'soleL', 'thighL'),
                 ('hamstring1L', p3,p2, 'soleL', 'thighL'),
                 ('hamstring2L', p3,p4, 'soleL', 'thighL'),
                 ('hamstring3L', p3,p6, 'soleL', 'thighL'),
                 ('hamstring4L', p3,p8, 'soleL', 'thighL'),
                 ('hamstring1L', p5,p2, 'soleL', 'thighL'),
                 ('hamstring2L', p5,p4, 'soleL', 'thighL'),
                 ('hamstring3L', p5,p6, 'soleL', 'thighL'),
                 ('hamstring4L', p5,p8, 'soleL', 'thighL'),
                 ('hamstring1L', p7,p2, 'soleL', 'thighL'),
                 ('hamstring2L', p7,p4, 'soleL', 'thighL'),
                 ('hamstring3L', p7,p6, 'soleL', 'thighL'),
                 ('hamstring4L', p7,p8, 'soleL', 'thighL') ]
    #
    # Muscle fibers between calf and sole
    #
    def getGastro(self):
        c = self.getAnkleJointCenter()
        
        v1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
        v2 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
        v3 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2])
        v4 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2])
        
        r1 = c + array([ self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
        r2 = c + array([ self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
        r3 = c + array([-self['calfWidth']/2, -self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
        r4 = c + array([-self['calfWidth']/2,  self['calfLatWidth']/2,  self['calfSoleGap']/2 + self['calfLen']/2 ])
        
        p1 = c + array([ self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
        p2 = c + array([ self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
        p3 = c + array([-self['soleWidth']/2, -self['soleLen']/2, -self['calfSoleGap']/2])
        p4 = c + array([-self['soleWidth']/2,  self['soleLen']/2, -self['calfSoleGap']/2])
        
        return [
                 ('gastro1L', v1,p1, 'calfL', 'soleL'),
                 ('gastro2L', v1,p2, 'calfL', 'soleL'),
                 ('gastro3L', v1,p3, 'calfL', 'soleL'),
                 ('gastro4L', v1,p4, 'calfL', 'soleL'),
                 
                 ('gastro5L', v2,p1, 'calfL', 'soleL'),
                 ('gastro6L', v2,p2, 'calfL', 'soleL'),
                 ('gastro7L', v2,p3, 'calfL', 'soleL'),
                 ('gastro8L', v2,p4, 'calfL', 'soleL'),
                 
                 ('gastro9L', v3,p1, 'calfL', 'soleL'),
                 ('gastroaL', v3,p2, 'calfL', 'soleL'),
                 ('gastrobL', v3,p3, 'calfL', 'soleL'),
                 ('gastrocL', v3,p4, 'calfL', 'soleL'),
                 
                 ('gastrodL', v4,p1, 'calfL', 'soleL'),
                 ('gastroeL', v4,p2, 'calfL', 'soleL'),
                 ('gastrofL', v4,p3, 'calfL', 'soleL'),
                 ('gastrogL', v4,p4, 'calfL', 'soleL'),
                 
                 ######################################
                 
                 ('gastro1L', r1,p1, 'calfL', 'soleL'),
                 ('gastro2L', r1,p2, 'calfL', 'soleL'),
                 ('gastro3L', r1,p3, 'calfL', 'soleL'),
                 ('gastro4L', r1,p4, 'calfL', 'soleL'),
                 
                 ('gastro5L', r2,p1, 'calfL', 'soleL'),
                 ('gastro6L', r2,p2, 'calfL', 'soleL'),
                 ('gastro7L', r2,p3, 'calfL', 'soleL'),
                 ('gastro8L', r2,p4, 'calfL', 'soleL'),
                 
                 ('gastro9L', r3,p1, 'calfL', 'soleL'),
                 ('gastroaL', r3,p2, 'calfL', 'soleL'),
                 ('gastrobL', r3,p3, 'calfL', 'soleL'),
                 ('gastrocL', r3,p4, 'calfL', 'soleL'),
                 
                 ('gastrodL', r4,p1, 'calfL', 'soleL'),
                 ('gastroeL', r4,p2, 'calfL', 'soleL'),
                 ('gastrofL', r4,p3, 'calfL', 'soleL'),
                 ('gastrogL', r4,p4, 'calfL', 'soleL')
        ]

    #
    # Muscle fibers between calf and toe
    #        
    def getTibialis(self):
        c1 = self.getToePos()
        c2 = self.getCalfPos()
        p1 = c1 + array([0,                    -self['toeLen']/2,        self['toeHeight']/2])
        p2 = c2 + array([0,                    -self['calfLatWidth']/2, -self['calfLen']/4])
        p3 = c1 + array([self['soleWidth']/2,  0,                        self['toeHeight']/2])
        p4 = c2 + array([self['soleWidth']/2,  -self['calfLatWidth']/2, -self['calfLen']/4])
        p5 = c1 + array([-self['soleWidth']/2, 0,                        self['toeHeight']/2])
        p6 = c2 + array([-self['soleWidth']/2, -self['calfLatWidth']/2, -self['calfLen']/4])
        return [ ('TibialisL', p1,p2, 'toeL', 'calfL'),
                 ('TibialisL', p1,p4, 'toeL', 'calfL'),
                 ('TibialisL', p1,p6, 'toeL', 'calfL'),
                 ('TibialisL', p3,p2, 'toeL', 'calfL'),
                 ('TibialisL', p3,p4, 'toeL', 'calfL'),
                 ('TibialisL', p3,p6, 'toeL', 'calfL'),
                 ('TibialisL', p5,p2, 'toeL', 'calfL'),
                 ('TibialisL', p5,p4, 'toeL', 'calfL'),
                 ('TibialisL', p5,p6, 'toeL', 'calfL') ]
    
    
    def getAllLigaments(self):
        return   self.getHipLigaments() + self.getKneeLigaments() \
               + self.getAnkleLigaments() + self.getToeLigaments() \
               + self.getShoulderLigaments() + self.getElbowLigaments() \
               + self.getNeckLigaments()
    
    def getAllMuscles(self):
        return self.getBicepsFemoris() + self.getQuadricepsFemoris() + \
               self.getHamstring() + self.getGastro() + \
               self.getTibialis()

    def buildBody(self, rotParam = 'QUAT_WFIRST'):
        body = []
        
        l = [ ['trunk', [self['trunkWidth'], self['trunkLatWidth'], self['trunkLen']],   self.getTrunkPos(), 50.   ],
              ['head',  [self['headWidth'],  self['headLatWidth'],  self['headLen']],    self.getHeadPos(),   5.   ],
              ['uarm',  [self['uarmWidth'],  self['uarmLatWidth'],  self['uarmLen']],    self.getUarmPos(),   4.   ],
              ['larm',  [self['larmWidth'],  self['larmLatWidth'],  self['larmLen']],    self.getLarmPos(),   3.   ],
              ['thigh', [self['thighWidth'], self['thighLatWidth'], self['thighLen']],   self.getThighPos(),  4.   ],
              ['calf',  [self['calfWidth'],  self['calfLatWidth'],  self['calfLen']],    self.getCalfPos(),   3.   ],
              ['sole',  [self['soleWidth'],  self['soleLen'],       self['soleHeight']], self.getSolePos(),   1.5  ],
              ['toe',   [self['toeWidth'],   self['toeLen'],        self['toeHeight']],  self.getToePos(),    1.5  ],
               ]
        
        for i in xrange(len(l)):
            for (k, v) in self.direction.iteritems():
                if   k[ 0:len(l[i][0]) ] == l[i][0] and v == 0: xAxisKey = k
                elif k[ 0:len(l[i][0]) ] == l[i][0] and v == 1: yAxisKey = k
                elif k[ 0:len(l[i][0]) ] == l[i][0] and v == 2: zAxisKey = k
            l[i][1] = [ self[xAxisKey], self[yAxisKey], self[zAxisKey] ]
            del xAxisKey, yAxisKey, zAxisKey

        # DEBUGGING START
        #l = l[1:4]
        # DEBUGGING END


        for i, ll in zip(range(len(l)), l):
            #pos0[2] += 0.5 # Start from the sky
            
            name = ll[0]
            if name not in ['trunk', 'head']:
                name += 'L'
            
            xMat = self.getColumnMajorTransformMatrix(name)
            xMat = array(xMat).reshape(4,4).T
            linPos = xMat[0:3,3]
            #print xMat, linalg.det(xMat[0:3,0:3])
            rotQuat = MatrixToQuaternion(xMat)
            #print 'rotQuat:', rotQuat
            if rotParam == 'QUAT_WFIRST':
                pos0 = hstack([ linPos, rotQuat ])
                vel0 = zeros(7)
            else:
                v = QuatToV(rotQuat)
                pos0 = hstack([ linPos, v ])
                vel0 = zeros(6)
            drawingColor = (0.2,0.1,0.2)
            
            b = RigidBody.RigidBody(name, None, ll[3], ll[1], pos0, vel0, drawingColor, rotParam)
            if hasattr(self, 'rbConf'):
                for (k,v) in self.rbConf.iteritems():
                    if v.anotherName == name:
                        b.blenderName = v.name
            else:
                b.blenderName = ''
            body.append(b)
            
            #print '%10s'%name, pos0, vel0
            
            if name[-1] == 'L':
                # If there are left body segments, we also need right counterparts
                newName = name[:-1] + 'R'
                
                xMat = self.getColumnMajorTransformMatrix(newName)
                xMat = array(xMat).reshape(4,4).T
                linPos = xMat[0:3,3]
                rotQuat = MatrixToQuaternion(xMat)
                if rotParam == 'QUAT_WFIRST':
                    pos0 = hstack([ linPos, rotQuat ])
                    vel0 = zeros(7)
                else:
                    v = QuatToV(rotQuat)
                    pos0 = hstack([ linPos, v ])
                    vel0 = zeros(6)
                drawingColor = (0.2,0.1,0.2)

                b = RigidBody.RigidBody(newName, None, ll[3], ll[1], pos0, vel0, drawingColor, rotParam)
                if hasattr(self, 'rbConf'):
                    for (k,v) in self.rbConf.iteritems():
                        if v.anotherName == newName:
                            b.blenderName = v.name
                else:
                    b.blenderName = ''
                body.append(b)
                
                #print '%10s'%newName, pos0, vel0
                
                

        return body
    
    def buildFiber(self, availableBodyNames, noliga=False, noact=False):
        # Fiber constants for a ligament
        KSE = 1      # Should not be 0
        KPE = 0
        b   = 1         # Should not be 0
        T   = 0.
        A   = 0.
        fiber_liga   = self._buildFiber(availableBodyNames, self.getAllLigaments(), KSE, KPE, b, T, A, 'LIGAMENT')
        # Fiber constants for a muscle fiber
        KSE = 1            # Should not be 0
        KPE = 0
        b   = 1            # Should not be 0
        T   = 0.
        A   = 0.
        fiber_muscle = self._buildFiber(availableBodyNames, self.getAllMuscles(), KSE, KPE, b, T, A, 'MUSCLE')
        
        ret = []
        if not noliga:
            ret += fiber_liga
        if not noact:
            ret += fiber_muscle
        return ret
    def _buildFiber(self, availableBodyNames, fiberList, KSE, KPE, b, T, A, typeStr):
        fiber = []

        for (name, orgPosGlobal, insPosGlobal, orgBody, insBody) in fiberList:
            #orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
            orgPosLocal = self.changeToLocal(orgBody, orgPosGlobal)
            insPosLocal = self.changeToLocal(insBody, insPosGlobal)
            xrest = linalg.norm(orgPosGlobal - insPosGlobal)
            #name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos)
            m = MuscleFiber.MuscleFiber(name, typeStr, False, T, A, KSE, KPE, b, xrest, None, None,
                                        orgBody, insBody, None, None, orgPosLocal, insPosLocal)
            if orgBody in availableBodyNames and insBody in availableBodyNames:
                fiber.append(m)
            
            # Ligaments related to the neck joint do not have
            # right side counterparts. Skip it.
            if name[:4] == 'neck':
                continue
            # If there are left-side fibers, we also need right counterparts
            assert name[-1] == 'L' # Muscle fiber's name should be ended with 'L'
            
            
            newName = name[:-1] + 'R'
            #orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
            if orgBody[-1] == 'L':
                newOrgBody = orgBody[:-1] + 'R'
            elif orgBody in ['trunk', 'head']:
                newOrgBody = orgBody
            else:
                assert False
                
            if insBody[-1] == 'L':
                newInsBody = insBody[:-1] + 'R'
            elif insBody in ['trunk', 'head']:
                newInsBody = insBody
            else:
                assert False

            newOrgPosGlobal = copy(orgPosGlobal)
            newOrgPosGlobal[0] *= -1
            newInsPosGlobal = copy(insPosGlobal)
            newInsPosGlobal[0] *= -1
            orgPosLocal = self.changeToLocal(newOrgBody, newOrgPosGlobal)
            insPosLocal = self.changeToLocal(newInsBody, newInsPosGlobal)
            xrest = linalg.norm(newOrgPosGlobal - newInsPosGlobal)
            #name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos)
            m = MuscleFiber.MuscleFiber(newName, typeStr, False, T, A, KSE, KPE, b, xrest, None, None,
                                        newOrgBody, newInsBody, None, None, orgPosLocal, insPosLocal)
            if newOrgBody in availableBodyNames and newInsBody in availableBodyNames:
                fiber.append(m)

        return fiber

    def changeToLocal(self, bodyName, globalPos):
        assert len(globalPos) == 3
        xMat = self.getColumnMajorTransformMatrix(bodyName)
        xMat = array(xMat).reshape(4,4).T
        xMatInv = linalg.inv(xMat)
        localPos = dot(xMatInv, array( list(globalPos) + [1.0] ))
        return localPos[0:3]

    def getBipedBB(self):
        """
        A bounding box for the whole biped (for adjusting the camera)
        """
        maxX = max(self['trunkWidth']/2,
                   self['legsDist']/2 + self['thighWidth']/2,
                   self['legsDist']/2 + self['calfWidth']/2,
                   self['legsDist']/2 + self['soleWidth']/2,
                   self['legsDist']/2 + self['toeWidth']/2)
        minX = -maxX

        maxY = max(self['trunkLatWidth']/2,
                   self['thighLatWidth']/2,
                   self['calfLatWidth']/2,
                   self['soleLen']/2)
        minY = min(-maxY, -self['soleLen']/2-self['soleToeGap']-self['toeLen'])

        maxZ = self.getBipedHeight()
        minZ = 0.

        return (minX, minY, minZ, maxX, maxY, maxZ)
    
    def getPos(self, name):
        return self._pos[name]()
    
    def getColumnMajorTransformMatrix(self, name):
        lr = 'L'
        #print name,
        if name not in ['trunk', 'head']:
            if name[-1] in ['L', 'R']:
                lr = name[-1]
                name = name[:-1]
            else:
                print 'Error - Body parts other than trunk/head should have postfix L or R.'
                assert False
                
        comPos = self.getPos(name)
        if lr == 'R':
            comPos[0] *= -1
        
        #print comPos
        
        if hasattr(self, 'rbConf'):
            xAxis = self.rbConf[ self.bnCorres[ name ] ].getAxisVector('X')
            yAxis = self.rbConf[ self.bnCorres[ name ] ].getAxisVector('Y')
            zAxis = self.rbConf[ self.bnCorres[ name ] ].getAxisVector('Z')
            M = identity(4)
            M[0:3,0] = xAxis
            M[0:3,1] = yAxis
            M[0:3,2] = zAxis
            M[0:3,3] = comPos
            return list(M.T.flatten())
        else:
            M = identity(4)
            M[0:3,3] = comPos
            return list(M.T.flatten())
