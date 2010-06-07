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
	def __init__(self, fnPrefix, rotParam):
		assert rotParam in ['EULER_XYZ', 'EULER_ZXZ', 'QUAT_WFIRST']
		
		# Friction coefficient
		self.mu = 1.5
		# Simulation Timestep
		self.h = 0.0025
		# Contact threshold
		self.alpha0 = 0.
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
		#                               insertion , insertionPos, 
		#                               KSE, KPE, b, xrest, T, A )
		#-------------------------------------------------------------------------------
		muscleCfg = { 'Lhamstring1' : ('lowerback', (1,-1,-1),
				                       'LeftHip',   (0,-0.5,-1),
		                               5000, 4500, 500, 0.1, 0, 0),
				      'Lhamstring2' : ('lowerback', (1,-1,-1),
				                       'LeftKnee',  (0,0,-1),
		                               5000, 4500, 500, 0.1, 0, 0),
				      'Lgastro'     : ('LeftHip',   (0,1,-1),
				                       'LeftAnkle', (0,-1,-1),
		                               5000, 4500, 500, 0.1, 0, 0),
				      'Lfemoris1'   : ('lowerback', (1,-1,1),
				                       'LeftHip',   (0,-0.5,1),
		                               5000, 4500, 500, 0.1, 0, 0),
				      'Lfemoris2'   : ('lowerback', (1,-1,1),
				                       'LeftHip',   (0,1,1),
		                               5000, 4500, 500, 0.1, 0, 0)        }
		
		ligaCfg = { 'LhipLiga'      : ('lowerback', (1,-1,0),
				                       'LeftHip',   (0,-1,0),
		                               50000, 45000, 500, 0.05, 0, 0),
				    'LkneeLiga1'    : ('LeftHip',   (0,1,1),
				                       'LeftKnee',  (0,-1,-1),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LkneeLiga2'    : ('LeftHip',   (0,1,-1),
				                       'LeftKnee',  (0,-1,1),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LkneeLiga3'    : ('LeftHip',   (1,1,0),
				                       'LeftKnee',  (1,-1,0),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LkneeLiga4'    : ('LeftHip',   (-1,1,0),
				                       'LeftKnee',  (-1,-1,0),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LankleLiga1'   : ('LeftKnee',  (0,1,1),
				                       'LeftAnkle', (0,-1,-1),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LankleLiga2'   : ('LeftKnee',  (0,1,-1),
				                       'LeftAnkle', (0,1,-1),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LtoeLiga1'     : ('LeftAnkle', (0,1,-1),
				                       'LeftToe',   (0,-1,1),
		                               5000, 4500, 500, 0.05, 0, 0),
				    'LtoeLiga2'     : ('LeftAnkle', (0,1,1),
				                       'LeftToe',   (0,-1,-1),
		                               5000, 4500, 500, 0.05, 0, 0)    }


		
		self.fibers = []
		"""
		for name in muscleCfg:
			pm = PmMuscle(name, muscleCfg[name], 'MUSCLE', True)
			self.fibers.append(pm)
		"""
		for name in ligaCfg:
			pm = PmMuscle(name, ligaCfg[name], 'LIGAMENT', True)
			self.fibers.append(pm)
		
		if fnPrefix is not None:
			fnPrefix += rotParam + '_'
			q_file = open(fnPrefix + 'q.txt', 'r')
			qd_file = open(fnPrefix + 'qd.txt', 'r')
			qdd_file = open(fnPrefix + 'qdd.txt', 'r')
			#torque_file = open('/media/vm/devel/aran/pymuscle/torque.txt', 'r')
			
			# Ignore the first line
			noFrame, nb = map(int, q_file.readline().strip().split())
			self.noFrame = noFrame
			qd_file.readline()
			qdd_file.readline()
			#torque_file.readline()
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
					#t_ij = array( map(float, torque_file.readline().strip().split()) )
					#t_i.append(t_ij)
				self.q_data.append(q_i)
				self.qd_data.append(qd_i)
				#self.torque_data.append(t_i)
			
			q_file.seek(0, 0)
			q_file.readline()
			qd_file.seek(0, 0)
			qd_file.readline()
			qdd_file.seek(0, 0)
			qdd_file.readline()
			#torque_file.seek(0, 0)
			#torque_file.readline()
			
			# Body specific parameters and state vectors
			self.body = []
			for bodyName, pBodyName in bodyList:
				boxsize, mass, dc = bodyCfg[bodyName]
				q_ij = array( map(float, q_file.readline().strip().split()) )
				qd_ij = array( map(float, qd_file.readline().strip().split()) )
				
				body = PmBody(bodyName, pBodyName, mass, boxsize, q_ij, qd_ij, dc, rotParam)
				self.body.append(body)
		
		self.rotParam = rotParam
		self.cube = 0               # Cube(display list)
		self.gnd_texture = 0        # Ground texture
		self.quadric = 0            # OpenGL quadric object
		self.curFrame = 0           # Current frame number
		self.autoPlay = True
		self.drawCoupon = 1
		self.winWidth = 900
		self.winHeight = 750
		self.drawOrtho = False
		self.myChar = 0
		
		self.clearColor = (202./255, 207./255, 206./255)
		self.cfScaleFactor = 250 # Contact force arrow length multiplier
		self.perpW = 0.5         # Ratio of perspective viewport width
		self.perpH = 0.6         # Ratio of perspective viewport height
		
		self.planeSize = 40      # Half of ground plane size
		self.gndTexRep = 16      # Ground texture repeatation

	def findBodyIndex(self, name):
		nb = len(self.bodyList)
		for i, n in zip(range(nb), self.bodyList):
			bodyName, pBodyName = n
			if bodyName == name:
				return i
		raise Exception('Wrong body name!')
	def findBodyIndex2(self, nameList):
		for nl in nameList:
			try:
				return self.findBodyIndex(nl)
			except Exception as inst:
				if inst[0] == 'Wrong body name!':
					pass
		raise Exception('Wrong body name!')
	
def WriteConfigurationFile(gCon):
	assert gCon.rotParam == 'QUAT_WFIRST'
	
	nb = len(gCon.body)
	bodyConf = open('/home/johnu/pymuscle/body.conf', 'w')
	bodyConf.write('body=\n')
	bodyConf.write('(\n');
	for k in range(nb):
		p  = tuple(gCon.body[k].q[0:3])     # Linear position
		q  = tuple(gCon.body[k].q[3:7])     # Quaternion orientation
		pd = tuple(gCon.body[k].qd[0:3])    # Linear velocity
		qd = tuple(gCon.body[k].qd[3:7])    # Time rate of quaternion orientation
		
		bodyConf.write('\t{\n')
		bodyConf.write('\t\tname = \"' + gCon.bodyList[k][0] + '\";\n')
		bodyConf.write('\t\tp = [%f,%f,%f];\n' % p)
		bodyConf.write('\t\tq = [%f,%f,%f,%f];\n' % q)
		bodyConf.write('\t\tpd = [%f,%f,%f];\n' % pd)
		bodyConf.write('\t\tqd = [%f,%f,%f,%f];\n' % qd)
		bodyConf.write('\t\tmass = %lf;\n' % gCon.body[k].mass)
		bodyConf.write('\t\tsize = [%f,%f,%f];\n' % tuple(gCon.body[k].boxsize))
		#bodyConf.write('\t\tgrav = true;\n')
		bodyConf.write('\t}%s\n' % (',' if k<nb-1 else ''))
	bodyConf.write(');')
	bodyConf.close()

	nMuscle = len(gCon.fibers)
	muscleConf = open('/home/johnu/pymuscle/muscle.conf', 'w')
	muscleConf.write('muscle=\n')
	muscleConf.write('(\n');
	for k in range(nMuscle):
		mus = gCon.fibers[k]
		
		if mus.bAttachedPosNormalized:
			orgBodyIdx = gCon.findBodyIndex(mus.orgBody)
			insBodyIdx = gCon.findBodyIndex(mus.insBody)
			borg = gCon.body[orgBodyIdx]
			bins = gCon.body[insBodyIdx]
			
			localorg = tuple([b/2. * p for b,p in zip(borg.boxsize, mus.orgPos)])
			localins = tuple([b/2. * p for b,p in zip(bins.boxsize, mus.insPos)])
		else:
			localorg = tuple(mus.orgPos)
			localins = tuple(mus.insPos)
	
		muscleConf.write('\t{\n')
		muscleConf.write('\t\tname = \"' + mus.name + '\";\n')
		muscleConf.write('\t\torigin = \"' + mus.orgBody + '\";\n')
		muscleConf.write('\t\tinsertion = \"' + mus.insBody + '\";\n')
		muscleConf.write('\t\tKSE = %lf;\n' % mus.KSE)
		muscleConf.write('\t\tKPE = %lf;\n' % mus.KPE)
		muscleConf.write('\t\tb = %lf;\n' % mus.b)
		muscleConf.write('\t\txrest = %lf;\n' % mus.xrest)
		muscleConf.write('\t\tT = %lf;\n' % mus.T)
		muscleConf.write('\t\tA = %lf;\n' % mus.A)
		muscleConf.write('\t\toriginPos = [%f,%f,%f];\n' % localorg)
		muscleConf.write('\t\tinsertionPos = [%f,%f,%f];\n' % localins)
		muscleConf.write('\t}%s\n' % (',' if k<nMuscle-1 else ''))
	muscleConf.write(');')
	muscleConf.close()
		

if __name__ == '__main__':
	gCon = GlobalContext('/home/johnu/pymuscle/traj_', 'QUAT_WFIRST')
	WriteConfigurationFile(gCon)
	
	