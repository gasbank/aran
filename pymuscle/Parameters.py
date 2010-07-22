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
    name       = fields[0]
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
    anotherName= fields[7]
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
                self.nameList.append(rb.name)
                if rb.anotherName != '*':
                    self.anotherNameList.append(rb.anotherName)
                
            # Body name correspondence
            bnCorres = {}
            bnCorres['trunk'] = 'Chest'
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
            
            self.rbConf   = rbConf
            self.bnCorres = bnCorres

        else:
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


        # 2. Body gap (positive) or overlap (negative)
        #    Generally, overlapped one gives stable equilibrium condition
        self.p['trunkThighGap']     = self.p['thighLen']/5
        self.p['thighCalfGap']      = self.p['calfLen']/5
        self.p['calfSoleGap']       = self.p['soleHeight']/5
        self.p['soleToeGap']        = self.p['soleLen']/5

        # 3. Other
        self.p['legsDist']          = self.p['trunkWidth']/1.5


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

    def __getitem__(self, i):
        return self.p[i]

    def getBipedHeight(self):
        return (self.p['trunkLen']
                + self.p['trunkThighGap']
                + self.p['thighLen']
                + self.p['thighCalfGap']
                + self.p['calfLen']
                + self.p['calfSoleGap']
                + self.p['soleHeight'])

    def getTrunkPos(self):
        return array([0,
                      0,
                      self.getBipedHeight() - self.p['trunkLen']/2])
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
    def getHipJointCenter(self):
        return self.getThighPos() + array([0,0,self['thighLen']/2+self['trunkThighGap']/2])
    def getKneeJointCenter(self):
        return self.getCalfPos() + array([0,0,self['calfLen']/2+self['thighCalfGap']/2])
    def getAnkleJointCenter(self):
        return self.getSolePos() + array([0,0,self['soleHeight']/2+self['calfSoleGap']/2])
    def getToeJointCenter(self):
        return self.getToePos() + array([0,self['toeLen']/2+self['soleToeGap']/2,0])

    #
    # LIGAMENTS
    #
    def getHipLigaments(self):
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
        
        
        p3 = c + array([0, -self['trunkLatWidth']/2, self['trunkThighGap']*2])
        p4 = c + array([0, -self['thighLatWidth']/2, -self['trunkThighGap']/2])
        p5 = c + array([0, self['trunkLatWidth']/2, self['trunkThighGap']*2])
        p6 = c + array([0, self['thighLatWidth']/2, -self['trunkThighGap']/2])
        p7 = c + array([self['hipMuscleDist']/2, 0, self['trunkThighGap']/2])
        p8 = c + array([self['hipMuscleDist2']/2, 0, -self['trunkThighGap']/2])
        p9 = c + array([-self['hipMuscleDist']/2, 0, self['trunkThighGap']/2])
        p10 = c + array([-self['hipMuscleDist2']/2, 0, -self['trunkThighGap']/2])
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
    def getKneeLigaments(self):
        c = self.getKneeJointCenter()
        p1 = c + array([0, -self['thighLatWidth']/2, self['thighCalfGap']*2])
        p2 = c + array([0, +self['thighLatWidth']/2, self['thighCalfGap']*2])
        p3 = c + array([0, -self['calfLatWidth']/2, -self['thighCalfGap']*2])
        p4 = c + array([0, +self['calfLatWidth']/2, -self['thighCalfGap']*2])

        p5 = c + array([-self['thighWidth']/2, 0, self['thighCalfGap']/2])
        p6 = c + array([+self['thighWidth']/2, 0, self['thighCalfGap']/2])
        p7 = c + array([-self['calfWidth']/2, 0, -self['thighCalfGap']/2])
        p8 = c + array([+self['calfWidth']/2, 0, -self['thighCalfGap']/2])

        return [ ('kneeLiga1L', p1,p4, 'thighL', 'calfL'),
                 ('kneeLiga2L', p1,p3, 'thighL', 'calfL'),
                 ('kneeLiga3L', p1,p7, 'thighL', 'calfL'),
                 ('kneeLiga4L', p1,p8, 'thighL', 'calfL'),
                 ('kneeLiga1L', p2,p4, 'thighL', 'calfL'),
                 ('kneeLiga2L', p2,p3, 'thighL', 'calfL'),
                 ('kneeLiga3L', p2,p7, 'thighL', 'calfL'),
                 ('kneeLiga4L', p2,p8, 'thighL', 'calfL'),
                 ('kneeLiga1L', p5,p4, 'thighL', 'calfL'),
                 ('kneeLiga2L', p5,p3, 'thighL', 'calfL'),
                 ('kneeLiga3L', p5,p7, 'thighL', 'calfL'),
                 ('kneeLiga4L', p5,p8, 'thighL', 'calfL'),
                 ('kneeLiga1L', p6,p4, 'thighL', 'calfL'),
                 ('kneeLiga2L', p6,p3, 'thighL', 'calfL'),
                 ('kneeLiga3L', p6,p7, 'thighL', 'calfL'),
                 ('kneeLiga4L', p6,p8, 'thighL', 'calfL') ]
        
    def getAnkleLigaments(self):
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
        
    def getToeLigaments(self):
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
        return self.getHipLigaments() + self.getKneeLigaments() + self.getAnkleLigaments() + self.getToeLigaments() 
    
    def getAllMuscles(self):
        return self.getBicepsFemoris() + self.getQuadricepsFemoris() + self.getHamstring() + self.getGastro() + self.getTibialis()

    def buildBody(self, rotParam = 'QUAT_WFIRST'):
        body = []
        
        l = [ ['trunk', [self['trunkWidth'], self['trunkLatWidth'], self['trunkLen']], self.getTrunkPos(), 50. ],
              ['thigh', [self['thighWidth'], self['thighLatWidth'], self['thighLen']], self.getThighPos(), 4. ],
              ['calf',  [self['calfWidth'], self['calfLatWidth'], self['calfLen']],    self.getCalfPos(),  3. ],
              ['sole',  [self['soleWidth'], self['soleLen'], self['soleHeight']],      self.getSolePos(),  1.5  ],
              ['toe',   [self['toeWidth'], self['toeLen'], self['toeHeight']],         self.getToePos(),   1.5  ] ]
        
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
            if name != 'trunk':
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
                body.append(b)
                
                #print '%10s'%newName, pos0, vel0
                
                

        return body
    
    def buildFiber(self, availableBodyNames):
        # Fiber constants for a ligament
        KSE = 1      # Should not be 0
        KPE = 1
        b   = 1         # Should not be 0
        T   = 0.
        A   = 0.
        fiber_liga   = self._buildFiber(availableBodyNames, self.getAllLigaments(), KSE, KPE, b, T, A)
        # Fiber constants for a muscle fiber
        KSE = 10            # Should not be 0
        KPE = 10
        b   = 0.01            # Should not be 0
        T   = 0.
        A   = 0.
        fiber_muscle = self._buildFiber(availableBodyNames, self.getAllMuscles(), KSE, KPE, b, T, A)
        ### DEBUG ###
        #return fiber_liga
        #return []
        return fiber_liga + fiber_muscle
    def _buildFiber(self, availableBodyNames, fiberList, KSE, KPE, b, T, A):
        fiber = []

        for (name, orgPosGlobal, insPosGlobal, orgBody, insBody) in fiberList:
            #orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
            orgPosLocal = self.changeToLocal(orgBody, orgPosGlobal)
            insPosLocal = self.changeToLocal(insBody, insPosGlobal)
            xrest = linalg.norm(orgPosGlobal - insPosGlobal)
            #name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos)
            m = MuscleFiber.MuscleFiber(name, 'LIGAMENT', False, T, A, KSE, KPE, b, xrest, None, None,
                                        orgBody, insBody, None, None, orgPosLocal, insPosLocal)
            if orgBody in availableBodyNames and insBody in availableBodyNames:
                fiber.append(m)
                
            assert name[-1] == 'L' # Muscle fiber's name should be ended with 'L'
            
            # If there are left-side fibers, we also need right counterparts
            newName = name[:-1] + 'R'
            #orgBody, orgPos, insBody, insPos, KSE, KPE, b, xrest, T, A = cfg
            if orgBody[-1] == 'L':
                newOrgBody = orgBody[:-1] + 'R'
            elif orgBody == 'trunk':
                newOrgBody = orgBody
            else:
                assert False
                
            if insBody[-1] == 'L':
                newInsBody = insBody[:-1] + 'R'
            elif insBody == 'trunk':
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
            m = MuscleFiber.MuscleFiber(newName, 'LIGAMENT', False, T, A, KSE, KPE, b, xrest, None, None,
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
    def getColumnMajorTransformMatrix(self, name):
        pos = {'trunk': self.getTrunkPos(),
               'thigh': self.getThighPos(),
               'calf':  self.getCalfPos(),
               'sole':  self.getSolePos(),
               'toe':   self.getToePos()}
        lr = 'L'
        print name,
        if name != 'trunk':
            if name[-1] in ['L', 'R']:
                lr = name[-1]
                name = name[:-1]
            else:
                print 'Error - Body parts other than trunk should have postfix L or R.'
                assert False
                
        comPos = pos[name]
        if lr == 'R':
            comPos[0] *= -1
        
        print comPos
        
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

def DrawBiped():
    trunkPos = gBiped.getTrunkPos()
    glPushMatrix()
    matMultCol = gBiped.getColumnMajorTransformMatrix('trunk')
    glMultMatrixf(matMultCol)
    DrawAxisIndicator('trunk')
    for (k, v) in gBiped.direction.iteritems():
        if k[0:5] == 'trunk' and v == 0: xAxisKey = k
        elif k[0:5] == 'trunk' and v == 1: yAxisKey = k
        elif k[0:5] == 'trunk' and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    glColor(1,0,1)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

    glLineWidth(1)
    # Left leg drawn
    DrawLeg()

    # Right leg drawn
    glPushMatrix()
    #glScale(-1,1,1)
    glTranslate(-gBiped['legsDist'], 0, 0)
    DrawLeg()
    glPopMatrix()

    
    glLineWidth(2)
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
    lineLen = gBiped[name +'Width']/3
    glLineWidth(5)
    glBegin(GL_LINES)
    glColor(1,0,0); glVertex(0,0,0); glVertex(lineLen,0,0)
    glColor(0,1,0); glVertex(0,0,0); glVertex(0,lineLen,0)
    glColor(0,0,1); glVertex(0,0,0); glVertex(0,0,lineLen)
    glEnd()
    glLineWidth(1)

def DrawLeg():
    thighPos = gBiped.getThighPos()
    glPushMatrix()
    matMultCol = gBiped.getColumnMajorTransformMatrix('thighL')
    glMultMatrixf(matMultCol)
    DrawAxisIndicator('thigh')
    for (k, v) in gBiped.direction.iteritems():
        if k[0:5] == 'thigh' and v == 0: xAxisKey = k
        elif k[0:5] == 'thigh' and v == 1: yAxisKey = k
        elif k[0:5] == 'thigh' and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    glColor(1,1,0)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

    calfPos = gBiped.getCalfPos()
    glPushMatrix()
    matMultCol = gBiped.getColumnMajorTransformMatrix('calfL')
    glMultMatrixf(matMultCol)
    DrawAxisIndicator('calf')
    for (k, v) in gBiped.direction.iteritems():
        if k[0:4] == 'calf' and v == 0: xAxisKey = k
        elif k[0:4] == 'calf' and v == 1: yAxisKey = k
        elif k[0:4] == 'calf' and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    glColor(1,0,0)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

    solePos = gBiped.getSolePos()
    glPushMatrix()
    matMultCol = gBiped.getColumnMajorTransformMatrix('soleL')
    glMultMatrixf(matMultCol)
    DrawAxisIndicator('sole')
    for (k, v) in gBiped.direction.iteritems():
        if k[0:4] == 'sole' and v == 0: xAxisKey = k
        elif k[0:4] == 'sole' and v == 1: yAxisKey = k
        elif k[0:4] == 'sole' and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    glColor(1,1,1)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

    toePos = gBiped.getToePos()
    glPushMatrix()
    matMultCol = gBiped.getColumnMajorTransformMatrix('toeL')
    glMultMatrixf(matMultCol)
    DrawAxisIndicator('toe')
    for (k, v) in gBiped.direction.iteritems():
        if k[0:3] == 'toe' and v == 0: xAxisKey = k
        elif k[0:3] == 'toe' and v == 1: yAxisKey = k
        elif k[0:3] == 'toe' and v == 2: zAxisKey = k
    glScale(gBiped[xAxisKey], gBiped[yAxisKey], gBiped[zAxisKey] )
    glColor(0,1,0)
    if gWireframe: glutWireCube(1)
    else: glutSolidCube(1)
    glPopMatrix()
    del xAxisKey, yAxisKey, zAxisKey

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

if __name__ == '__main__':
    if len(sys.argv) == 2:
        fnRbConf = sys.argv[1]
    else:
        fnRbConf = None
    gBiped = BipedParameter(fnRbConf)
    bipHeight = gBiped.getBipedHeight()
    print 'Biped Height =', bipHeight
    
    gPoiList = ( ('Biped', 1.5/bipHeight),
             ('Hip', 5./bipHeight),
             ('Knee', 5./bipHeight),
             ('Ankle', 5./bipHeight),
             ('Toe', 5./bipHeight) )

    print 'Initialize a rigid body with initial conditions...'
    bipedParam = gBiped
    plist = bipedParam.buildBody('EXP')
    flist = bipedParam.buildFiber([b.name for b in plist])
    print 'number of rigid bodies =', len(plist), ' '*20, 'number of muscle fibers =', len(flist)
    h = GetSimTimeStep()
    WriteSimcoreConfFile('MosekMultiRbMultiFibers3D.conf', plist, flist, h, 100.0)
    
    #Main()
