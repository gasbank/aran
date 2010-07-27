#!BPY
# Rigid body and muscle fiber model exporter
# 2010 Geoyeob Kim

import zlib
import os
import math
import Blender
from Blender import sys, Window, Ipo, Armature, Modifier
from Blender import Material, Constraint, Mesh, Object
from Blender.Armature import NLA
import bpy
import BPyMessages
#import array
import Blender.Mathutils
from Blender.Mathutils import *
#from xml.dom.minidom import *
#import struct
import MathUtil
import numpy


def XformFromIpoCurves(ipo, frame):
	loc = [0, 0, 0]
	q = Quaternion()
	if ipo == 0:
		return Vector(loc), q
	
	for c in ipo:
		v = c[frame]
		if c.name == 'LocX': loc[0] = v
		elif c.name == 'LocY': loc[1] = v
		elif c.name == 'LocZ': loc[2] = v
		elif c.name == 'QuatX': q.x = v
		elif c.name == 'QuatY': q.y = v
		elif c.name == 'QuatZ': q.z = v
		elif c.name == 'QuatW': q.w = v	
	return Vector(loc), q.normalize()

def QuatMult(q1, q2):
	a1, b1, c1, d1 = q1.w, q1.x, q1.y, q1.z
	a2, b2, c2, d2 = q2.w, q2.x, q2.y, q2.z
	q = Quaternion()
	q.w = a1*a2 - b1*b2 - c1*c2 - d1*d2
	q.x = a1*b2 + b1*a2 + c1*d2 - d1*c2
	q.y = a1*c2 - b1*d2 + c1*a2 + d1*b2
	q.z = a1*d2 + b1*c2 - c1*b2 + d1*a2
	return q

def XtoMat(X):
	#print X
	M = Euler(X[2]).toMatrix()
	M.resize4x4()
	
	headPos = Vector(X[0])
	boneDir = Vector(X[1])
	d = headPos + boneDir
	d.resize4D()
	M[3] = d
	return M

def BuildInitialTransform(interestedArmature, interested):
	sce = bpy.data.scenes.active
	nb = len(interested[0])
	
	for ob in sce.objects:
		if ob.name == interestedArmature:
			if ob.action:
				for c in ob.action.getAllChannelIpos():
					if c in interested[0]:
						ipo = ob.action.getChannelIpo(c)
						idx = interested[0].index(c)
						interested[1][idx] = ipo
			else:
				print '[WARN] NO ACTION ATTACHED TO ', ob
			data = ob.getData()
			
			OXZ = [ [0,0,0], list(ob.loc), [r/math.pi * 180 for r in ob.rot] ]
			M_OXZ = XtoMat(OXZ)
			#print 'OXZ', OXZ
			
			for bone in data.bones.values():
				if bone.name in interested[0]:
					idx = interested[0].index(bone.name)
					interested[2][idx] = bone
	
	for i in range(nb):
		bone = interested[2][i]
		boneDir = bone.tail['BONESPACE'] - bone.head['BONESPACE']
		#print bone, boneDir, bone.tail['BONESPACE'], bone.head['BONESPACE']

		print bone, 'roll', bone.roll
		print bone, 'tail', bone.tail
		print bone, 'head', bone.head

		y = Vector(0,1,0)
		rotAxis = y.cross(boneDir)
		rotAngle = AngleBetweenVecs(y, boneDir)
		if rotAngle == 0:
			q = Quaternion()
		else:
			print 'rotangle', rotAngle
			q = Quaternion(rotAxis, rotAngle)
		
		q0 = Quaternion(Vector(0,1,0), bone.roll['BONESPACE'])
		q = QuatMult(q, q0)
		q.normalize()
		
		X = [ list(bone.head['BONESPACE']),
		      list(boneDir),
		      list(q.toEuler()) ]
		interested[3].append(X)
	
	
	for i in range(nb):
		try:
			rb = bpy.data.objects['RB.' + interested[0][i]]
		except:
			rb = None
		bone = interested[2][i]
		
		#print bone.name, X
		XP = 0
		if bone.parent:
			for k in range(nb):
				if interested[0][k] == bone.parent.name:
					print bone, 'parent is', bone.parent
					XP = interested[4][k]
					break
		else:
			print bone, 'parent is root.'
			XP = M_OXZ
		if XP is 0:
			raise Exception, 'WTF'
		
		MX = XtoMat(interested[3][i])
		
		headPos, boneDir, q = interested[3][i]
		#boneDir = [x/2 for x in boneDir]
		MXHALF = XtoMat( (headPos, boneDir, q) )

		interested[4].append(MX * XP)
		if rb:
			rb.setMatrix(MXHALF * XP)
		
		
	
	return M_OXZ, interested



###########################################################################
# MAIN ENTRY START
###########################################################################
if __name__ == '__main__':
	class TestSet:
		def __init__(self, trajName, nFrame):
			self.trajName = trajName
			self.nFrame   = nFrame
	
	TEST_SET = [ TestSet( 'Walk0',  300  ),
	             TestSet( 'Nav0',  2000  ),
	             TestSet( 'Exer0',  500  ) ]
	#
	# ==================== USER PARAMETERS ========================
	# Select test set first.
	ts = 2
	assert ts < len(TEST_SET)
	#
	#
	# Frame per second determined by the motion capture sequence.
	# Do not change this value unless you are using another
	# motion capture sequence with a different sampling rate.
	mocapFps = 120
	mocapFrameTime = 1./mocapFps
	# The number of frame we want to export
	# from the motion capture sequence.
	# You can determine this value by checking
	# the 'current frame' indicator in Blender.
	# This frame index starts from 1.
	# This is not the same as the number of exported frame.
	mocapFrameCount = TEST_SET[ts].nFrame
	# Export frame per second which we are targeting of.
	# If it is the same as 'mocapFps', the mocap trajectoriy
	# is exactly copied. Otherwise, it is upsampled or downsampled
	# as needed.
	exportFps = 100
	exportFrameTime = 1./exportFps
	# The clock-time length of the export sequence
	# can be calculated by the following equation.
	exportLengthInSeconds = float(mocapFrameCount)/mocapFps
	# Total number of frame to be exported.
	exportFrameCount = int(exportLengthInSeconds * exportFps)
	# Simulation timestep in seconds.
	# It should be the same as 'exportFrameTime'.
	h = exportFrameTime
	# Determine the parameterization of orientation.
	# You can select either 'Euler' or 'Quat'.
	# 'EULER_XYZ' gives you 6 dimensional vector
	# for each ribid body at each instant
	# and 'QUAT_WFIRST' gives you 7 dimensional vector
	# likewise.
	rotParam = 'EXP'
	# Trajectory data taken from this armature
	interestedArmature = 'Armature.' + TEST_SET[ts].trajName
	# ==============================================================
	#
	

    # Bone(body) names should be stored in breadth-first order	
	#               name          local (xAxis and yAxis)    another name
	#                                 in world coord         in simulator
	bodyTable = [ ('Hips',            '+X',     '+Z',         '*'    ),
                  ('lowerback',       '+X',     '+Z',         '*'    ),
	              ('Chest',           '+X',     '+Z',         'trunk'),
	              ('Chest2',          '+X',     '+Z',         '*'),
	              ('lowerneck',       '+X',     '+Z',         '*'),
	              ('Neck',            '+X',     '+Z',         '*'),
	              ('Head',            '+X',     '+Z',         'head'),
	              ('LeftCollar',      '+X',     '+Z',         '*'),
	              ('RightCollar',     '+X',     '+Z',         '*'),
	              ('LeftShoulder',    '-X',     '-Z',         'uarmL'),
	              ('RightShoulder',   '-X',     '-Z',         'uarmR'),
	              ('LeftElbow',       '-X',     '-Z',         'larmL'),
	              ('RightElbow',      '-X',     '-Z',         'larmR'),
	              ('LHipJoint',       '-Y',     '-Z',         '*'    ),
				  ('RHipJoint',       '-Y',     '-Z',         '*'    ),
	              ('LeftHip',         '-X',     '-Z',         'thighL'),
	              ('RightHip',        '-X',     '-Z',         'thighR'),
	              ('LeftKnee',        '-X',     '-Z',         'calfL'),
	              ('RightKnee',       '-X',     '-Z',         'calfR'),
	              ('LeftAnkle',       '+X',     '-Y',         'soleL'),
	              ('RightAnkle',      '+X',     '-Y',         'soleR'),
	              ('LeftToe',         '+X',     '-Y',         'toeL'),
	              ('RightToe',        '+X',     '-Y',         'toeR') ]
	arma = bpy.data.objects[interestedArmature]
	assert arma
	boneNameList = [v.name for v in arma.getData().bones.values()]
	rbMissing = False
	for bt in bodyTable:
		assert bt[0] in boneNameList
		isExist = False
		for obs in bpy.data.objects:
			if obs.name == 'RB.' + bt[0]:
				isExist = True
				break
		if not isExist:
			print 'Warning - RB object \'RB.' + bt[0] + '\' does not exist.'
			rbMissing = True
		
	interestedBodyNames = [ a for a, b, c, d in bodyTable ]


	nb = len(interestedBodyNames) # Number of interested bodies
	
	interested = [ interestedBodyNames,
	               [0] * nb,                              # IPOs
	               [0] * nb,                              # Bone objects
	               [],                                    # Local transform
	               []                                     # Cumulated transform
	             ]
	
	M_OXZ, interested = BuildInitialTransform(interestedArmature, interested)
	#Blender.Redraw()
	
	traj_q = []
	body_dim = []
	for i in range(exportFrameCount):
		frame = i+1
		print 'Frame', frame, 'processed.'
		# Blender thinks Frame 1 as time=0 point.
		# So we need to substract 1 from frame
		# to calculate the exact current time in seconds.
		curTime = (frame-1)*exportFrameTime
		
		if exportFrameCount == mocapFrameCount:
			# No undersampling or downsampling of mocap data.
			# You can see the synchronized result of the armature
			# and rigid bodies in the Blender screen.
			# Otherwise, only the rigid bodies will be updated.
			Blender.Set('curframe', frame)
	
		traj_qi = []
		for j in range(nb):
			try:
				rb = bpy.data.objects['RB.' + interested[0][j]]
			except:
				rb = None
			bone = interested[2][j]
					
			# Calculate the local anim transform
			# Convert the current time (seconds) to Blender frame unit.
			blenderFrame = curTime/mocapFrameTime + 1
			Aloc, Aq = XformFromIpoCurves(interested[1][j], blenderFrame)
			
			# Copy of the local transform (tail of the bone)
			headPos, boneDir, q = interested[3][j]
			Xtail = XtoMat( (headPos, boneDir, q) )
			Xhead = XtoMat( (headPos, [0, 0, 0], q) )
			
			if bone.parent:
				for k in range(nb):
					if interested[0][k] == bone.parent.name:
						PARENT = interested[4][k]
						break
			else:
				PARENT = M_OXZ
			
			A = Aq.toMatrix()
			A.resize4x4()
			A[3] = Aloc.resize4D()
			
			m = A*Xhead*PARENT

			if ts in [0,1]:
				### MOTION TRAJECTORY CORRECTION ###
				if bone.name == 'LeftAnkle':
					cor = Quaternion(Vector(1,0,0),-15)
					m = cor.toMatrix().resize4x4() * m
				elif bone.name == 'RightAnkle':
					cor = Quaternion(Vector(1,0,0),-19)
					m = cor.toMatrix().resize4x4() * m
			
			# Move to middle of head and tail
			T = Matrix()
			T.identity()
			T[3] = Vector(0,bone.length/2,0,1)
			m = T*m

			
			if ts in [0,1]:
				### MOTION TRAJECTORY CORRECTION ###
				if bone.name == 'LeftAnkle':
					cor = Quaternion(Vector(0,1,0),30)
					m = cor.toMatrix().resize4x4() * m
				elif bone.name == 'RightAnkle':
					cor = Quaternion(Vector(0,1,0),-30)
					m = cor.toMatrix().resize4x4() * m
				elif bone.name == 'Head':
					cor = Quaternion(Vector(0,0,1),-8)
					m = cor.toMatrix().resize4x4() * m
					cor = Quaternion(Vector(0,1,0),5)
					m = cor.toMatrix().resize4x4() * m
			
			if rb:		
				# rb.setMatrix(m) -- do not use this one.
				rb.setLocation(m[3][0], m[3][1], m[3][2])
				mteul = m.toEuler()
				rb.setEuler(mteul.x/180*math.pi, mteul.y/180*math.pi, mteul.z/180*math.pi)
			
			# Position(q) for a single body
			trans = m.translationPart()
			if rotParam == 'EULER_XYZ':
				rot = m.toEuler()
				traj_qi.append(array([trans.x,
				                      trans.y,
				                      trans.z,
				                      rot.x / 180 * math.pi,
				                      rot.y / 180 * math.pi,
				                      rot.z / 180 * math.pi]))
			elif rotParam == 'QUAT_WFIRST':
				rot = m.toQuat()
				rot = rot.normalize()
				traj_qi.append(array([trans.x,
				                      trans.y,
				                      trans.z,
				                      rot.w,
				                      rot.x,
				                      rot.y,
				                      rot.z]))
			elif rotParam == 'EXP':
				rotQuat = m.toQuat()
				rotQuat = rotQuat.normalize()
				rot1 = MathUtil.QuatToV([rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z])
				if len(traj_q) > 0:
					th1 = numpy.linalg.norm(rot1)
					rot2 = (1-2*math.pi/th1)*rot1
					prevRot = traj_q[-1][j][3:6]
					rot1Devi = numpy.linalg.norm(prevRot - rot1)
					rot2Devi = numpy.linalg.norm(prevRot - rot2)
					if rot1Devi > rot2Devi:
						rot = rot2
						#print 'alter selected'
					else:
						rot = rot1
				else:
					rot = rot1
				traj_qi.append(numpy.array([trans.x,
				                            trans.y,
				                            trans.z,
				                            rot[0],
				                            rot[1],
				                            rot[2]]))
				
			else:
				raise Exception('unknown rotation parameterization.')
			
			#bb = rb.getBoundBox(0)
			
			#print [2*abs(b) for b in bb[0]]
			"""		
			T[3] = Vector(0,bone.length,0,1)
			mm = T*A*Xhead*PARENT
			"""
			T[3] = Vector(0,bone.length/2,0,1)
			mm = T*m
			interested[4][j] = mm
			
		traj_q.append(traj_qi)
		
		# If you want to check the animation of
		# rigid bodies then remove the comment
		# of the following line
		
		#Blender.Redraw()
	
	
	# Final 'Redraw()' function on blender is crucial
	# to get joint anchors in local coordinates.
	# Don't know why...
	Blender.Redraw()
	traj_qd = []
	print 'Total # of frames:', len(traj_q)
	print 'Total # of bodies:', len(traj_q[0])
	for i in range(len(traj_q)-1):
		traj_qdi = [(q1-q0)/h for q1, q0 in zip(traj_q[i+1], traj_q[i])]
		traj_qd.append(traj_qdi)
	
	traj_qdd = []
	for i in range(len(traj_q)-2):
		traj_qddi = [(qd1-qd0)/h for qd1, qd0 in zip(traj_qd[i+1], traj_qd[i])]
		traj_qdd.append(traj_qddi)
	
	
	metastr = '%d %d\n' % (len(traj_q), len(traj_q[0]))
	
	assert rotParam in ['EULER_XYZ', 'QUAT_WFIRST', 'EXP']
	if rotParam in ['EULER_XYZ', 'EXP']:
		vecLen = 6
	elif rotParam == 'QUAT_WFIRST':
		vecLen = 7
	else:
		raise Exception, 'unknown rotation parameterization'
	
	homeDir = os.getenv("HOME")
	fnBasePrefix        = homeDir + '/pymuscle/trajectories/' + TEST_SET[ts].trajName
	fnPrefix            = fnBasePrefix + '.traj_' + rotParam + '_'
	fnRigidBodyConfig   = fnBasePrefix + '.traj.conf'
	fnJointAnchorConfig = fnBasePrefix + '.jointanchor.conf'
	
	traj_q_file = open(fnPrefix + 'q.txt', 'w')
	traj_q_file.write(metastr)
	for i,iidx in zip(traj_q, xrange(len(traj_q))):
		for j,jidx in zip(i, xrange(len(i))):
			traj_q_file.write('%15e'*vecLen % tuple(j))
			#traj_q_file.write('%15e'*4 % tuple(Euler(j[3:6]).toQuat()))
			traj_q_file.write(' # [' + str(iidx) + '] ' + bodyTable[jidx][0] + ' ' + str(numpy.linalg.norm(j[3:6])))
			traj_q_file.write('\n')
	traj_q_file.close()
	
	traj_qd_file = open(fnPrefix + 'qd.txt', 'w')
	traj_qd_file.write(metastr)
	for i in traj_qd:
		for j in i:
			traj_qd_file.write(('%15e'*vecLen % tuple(j)) + '\n')
	traj_qd_file.close()
	
	traj_qdd_file = open(fnPrefix + 'qdd.txt', 'w')
	traj_qdd_file.write(metastr)
	for i in traj_qdd:
		for j in i:
			traj_qdd_file.write(('%15e'*vecLen % tuple(j)) + '\n')
	traj_qdd_file.close()
	
	if not rbMissing:
		rbconf_file = open(fnRigidBodyConfig, 'w')
		for (name, xAxis, yAxis, corresName), bone in zip(bodyTable, interested[2]):
			rb = bpy.data.objects['RB.' + name]
			bb = rb.getBoundBox(0)[0]
			boundbox = [ abs(bbi)*2 for bbi in bb ]
			rbconf_file.write('%-16s'%name + ' ')
			rbconf_file.write('%15e'*3 % tuple(boundbox) + '   ')
			rbconf_file.write(xAxis + '   ' + yAxis + '   ')
			rbconf_file.write('%15e'%bone.length + '    ')
			rbconf_file.write(corresName)
			rbconf_file.write('\n')
		rbconf_file.close()
	
	
	jaconf_file = open(fnJointAnchorConfig, 'w')
	for (name, xAxis, yAxis, corresName), bone in zip(bodyTable, interested[2]):
		try:
			rb = bpy.data.objects['RB.' + name]
		except:
			continue
		jaList = []
		for obj in bpy.data.objects:
			if obj.getParent() == rb:
				jaList.append(obj)
		#print name,
		for ja in jaList:
			jaLocalPos = ja.getMatrix('localspace')[3]
			#print ja, jaLocalPos,
			
			# Left-side anchors
			jaconf_file.write('%15s ' % ja.name)
			jaconf_file.write('%15s ' % corresName)
			jaconf_file.write('%15e '*3 % (jaLocalPos[0],jaLocalPos[1],jaLocalPos[2]))
			jaconf_file.write('\n')
			# Right-side anchors
			rightJaName = ja.name.replace('L.', 'R.')
			if ja.name != rightJaName:
				jaconf_file.write('%15s ' % rightJaName)
				if corresName != 'trunk':
					jaconf_file.write('%15s ' % (corresName[:-1] + 'R'))
				else:
					jaconf_file.write('%15s ' % corresName)
				jaconf_file.write('%15e '*3 % (-jaLocalPos[0],jaLocalPos[1],jaLocalPos[2]))
				jaconf_file.write('\n')
		#print
	jaconf_file.close()