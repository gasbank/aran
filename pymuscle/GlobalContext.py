#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Global context file
"""
from math import cos, sin, pi
from PmMuscle import *
from PmBody import *

class GlobalContext:
	def __init__(self):
		# Friction coefficient
		self.mu = 1.7
		# Simulation Timestep
		self.h = 1.
		# Contact threshold
		self.alpha0 = 0.020
		# Eight basis of friction force
		self.di = [ (1, 0, 0),
		            (cos(pi/4), sin(pi/4), 0),
		            (0, 1, 0),
		            (-cos(pi/4), sin(pi/4), 0),
		            (-1, 0, 0),
		            (-cos(pi/4), -sin(pi/4), 0),
		            (0, -1, 0),
		            (cos(pi/4), -sin(pi/4), 0) ]
		
		
		#              name         pName
		#-------------------------------------------------------------------------------
		bodyList = [ ('Hips',       None),
				     ('lowerback',  'Hips'),
				     ('LHipJoint',  'Hips'),
				     ('RHipJoint',  'Hips'),
				     ('LeftHip',    'LHipJoint'),
				     ('RightHip',   'RHipJoint'),
				     ('LeftKnee',   'LeftHip'),
				     ('RightKnee',  'RightHip'),
				     ('LeftAnkle',  'LeftKnee'),
				     ('RightAnkle', 'RightKnee'),
				     ('LeftToe',    'LeftAnkle'),
				     ('RightToe',   'RightAnkle')  ]
		self.bodyList = bodyList
		
		#             name                  boxsize        mass  drawing color
		#-------------------------------------------------------------------------------
		bodyCfg  = { 'Hips'      : [ (0.115, 0.144, 0.085), 3,  (0.2,0.1,0.2) ], # root
				     'lowerback' : [ (0.427, 0.720, 0.184), 30, (0.2,0.1,0.2) ], # trunk
				     'LHipJoint' : [ (0.054, 0.274, 0.054), 3,  (0.2,0.1,0.2) ],
				     'RHipJoint' : [ (0.054, 0.274, 0.054), 3,  (0.2,0.1,0.2) ],
				     'LeftHip'   : [ (0.145, 0.450, 0.145), 3,  (0.1,0.6,0.0) ],
				     'RightHip'  : [ (0.145, 0.450, 0.145), 3,  (0.0,0.2,0.6) ],
				     'LeftKnee'  : [ (0.145, 0.450, 0.145), 3,  (0.1,0.6,0.0) ],
				     'RightKnee' : [ (0.145, 0.450, 0.145), 3,  (0.0,0.2,0.6) ],
				     'LeftAnkle' : [ (0.184, 0.210, 0.090), 3,  (0.1,0.6,0.0) ],
				     'RightAnkle': [ (0.184, 0.210, 0.090), 3,  (0.0,0.2,0.6) ],
				     'LeftToe'   : [ (0.184, 0.105, 0.090), 3,  (0.1,0.6,0.0) ],
				     'RightToe'  : [ (0.184, 0.105, 0.090), 3,  (0.0,0.2,0.6) ] }
		
		# MUSCLE CONFIGURATION
		#              name         : ( origin    , originPos
		#                               insertion , insertionPos )
		#-------------------------------------------------------------------------------
		muscleCfg = { 'Lhamstring1' : ('lowerback', (1,-1,-1),
				                       'LeftHip',   (0,-0.5,-1)),
				      'Lhamstring2' : ('lowerback', (1,-1,-1),
				                       'LeftKnee',  (0,0,-1)),
				      'Lgastro'     : ('LeftHip',   (0,1,-1),
				                       'LeftAnkle', (0,-1,-1)),
				      'Lfemoris1'   : ('lowerback', (1,-1,1),
				                       'LeftHip',   (0,-0.5,1)),
				      'Lfemoris2'   : ('lowerback', (1,-1,1),
				                       'LeftHip',   (0,1,1))        }
		
		ligaCfg = { 'LhipLiga'      : ('lowerback', (1,-1,0),
				                       'LeftHip',   (0,-1,0)),
				    'LkneeLiga1'    : ('LeftHip',   (0,1,1),
				                       'LeftKnee',  (0,-1,-1)),
				    'LkneeLiga2'    : ('LeftHip',   (0,1,-1),
				                       'LeftKnee',  (0,-1,1)),
				    'LkneeLiga3'    : ('LeftHip',   (1,1,0),
				                       'LeftKnee',  (1,-1,0)),
				    'LkneeLiga4'    : ('LeftHip',   (-1,1,0),
				                       'LeftKnee',  (-1,-1,0)),
				    'LankleLiga1'   : ('LeftKnee',  (0,1,1),
				                       'LeftAnkle', (0,-1,-1)),
				    'LankleLiga2'   : ('LeftKnee',  (0,1,-1),
				                       'LeftAnkle', (0,1,-1)),
				    'LtoeLiga1'     : ('LeftAnkle', (0,1,-1),
				                       'LeftToe',   (0,-1,1)),
				    'LtoeLiga2'     : ('LeftAnkle', (0,1,1),
				                       'LeftToe',   (0,-1,-1))    }
		
		self.fibers = []
		for name in muscleCfg:
			orgBody, orgPos, insBody, insPos = muscleCfg[name]
			pm = PmMuscle(name, orgBody, orgPos, insBody, insPos, 'MUSCLE')
			self.fibers.append(pm)
		for name in ligaCfg:
			orgBody, orgPos, insBody, insPos = ligaCfg[name]
			pm = PmMuscle(name, orgBody, orgPos, insBody, insPos, 'LIGAMENT')
			self.fibers.append(pm)
		
		q_file = open('/media/vm/devel/aran/pymuscle/traj_q.txt', 'r')
		qd_file = open('/media/vm/devel/aran/pymuscle/traj_qd.txt', 'r')
		qdd_file = open('/media/vm/devel/aran/pymuscle/traj_qdd.txt', 'r')
		torque_file = open('/media/vm/devel/aran/pymuscle/torque.txt', 'r')
		# Ignore the first line
		noFrame, nb = map(int, q_file.readline().strip().split())
		self.noFrame = noFrame
		qd_file.readline()
		qdd_file.readline()
		torque_file.readline()
		self.q_data = []
		self.qd_data = []
		self.torque_data = []
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
			self.q_data.append(q_i)
			self.qd_data.append(qd_i)
			self.torque_data.append(t_i)
		
		q_file.seek(0, 0)
		q_file.readline()
		qd_file.seek(0, 0)
		qd_file.readline()
		qdd_file.seek(0, 0)
		qdd_file.readline()
		torque_file.seek(0, 0)
		torque_file.readline()
		# Body specific parameters and state vectors
		self.configured = []
		for bodyName, pBodyName in bodyList:
			boxsize, mass, dc = bodyCfg[bodyName]
			q_ij = array( map(float, q_file.readline().strip().split()) )
			qd_ij = array( map(float, qd_file.readline().strip().split()) )
			
			body = PmBody(bodyName, pBodyName, mass, boxsize, q_ij, qd_ij, dc, 'EULER_XYZ')
			self.configured.append(body)
			
		self.cube = 0               # Cube(display list)
		self.gnd_texture = 0        # Ground texture
		self.quadric = 0            # OpenGL quadric object
		self.curFrame = 0           # Current frame number
		self.autoPlay = False
		self.drawCoupon = 1
		self.winWidth = int(320*2.5)
		self.winHeight = int(240*2.5)
		self.drawOrtho = False
		self.myChar = 0
		
		self.clearColor = (202./255, 207./255, 206./255)
		self.cfScaleFactor = 250 # Contact force arrow length multiplier
		self.perpW = 0.7         # Ratio of perspective viewport width
		self.perpH = 0.75         # Ratio of perspective viewport height
		
		self.planeSize = 40      # Half of ground plane size
		self.gndTexRep = 16      # Ground texture repeatation