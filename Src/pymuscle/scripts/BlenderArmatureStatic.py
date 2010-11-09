#!BPY

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
import array
import Blender.Mathutils
from Blender.Mathutils import *
from xml.dom.minidom import *
import struct

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
	M = Matrix()
	M.resize4x4()
	M = Euler(X[1]).toMatrix()
	M.resize4x4()
	M[3] = Vector(X[0][0],X[0][1],X[0][2],1)
	return M

def BuildInitialTransform(interestedArmature, interested):
	sce = bpy.data.scenes.active
	for ob in sce.objects:
		if ob.name == interestedArmature:
			for c in ob.action.getAllChannelIpos():
				if c in interested[0]:
					ipo = ob.action.getChannelIpo(c)
					idx = interested[0].index(c)
					interested[1][idx] = ipo
	
	
			data = ob.getData()
			
			OXZ = [ list(ob.loc), [r/math.pi * 180 for r in ob.rot] ]
			M_OXZ = XtoMat(OXZ)
			#print 'OXZ', OXZ
			
			for bone in data.bones.values():
				if bone.name in interested[0]:
					idx = interested[0].index(bone.name)
					interested[2][idx] = bone
	
	for i in range(4):
		bone = interested[2][i]
		boneDir = bone.tail['BONESPACE'] - bone.head['BONESPACE']
		#print 'boneDir', boneDir
		y = Vector(0,1,0)
		rotAxis = y.cross(boneDir)
		rotAngle = AngleBetweenVecs(y, boneDir)
		if rotAngle == 0:
			q = Quaternion()
		else:
			q = Quaternion(rotAxis, rotAngle)
		
		X = [ list(boneDir), list(q.toEuler()) ]
		interested[3].append(XtoMat(X))
	
	
	for i in range(4):
		rb = bpy.data.objects['RB.' + interested[0][i]]
		bone = interested[2][i]
		
		X = interested[3][i].copy()
		X[3] = Vector(X[3][0]/2,X[3][1]/2,X[3][2]/2, 1)
		if bone.parent:
			for k in range(9):
				if interested[0][k] == bone.parent.name:
					XP = interested[4][k]
					break
		else:
			XP = M_OXZ
		
		rb.setMatrix(X * XP)
		interested[4].append(interested[3][i] * XP)
	
	return interested



###########################################################################

interestedArmature = 'Armature.007'

interested = [ ['Hips', 'LHipJoint', 'RHipJoint',     # Bone names
                'LeftHip', 'RightHip',
                'LeftKnee', 'RightKnee',
                'LeftAnkle', 'RightAnkle'],
               [0, 0, 0, 0, 0, 0, 0, 0, 0],           # IPOs
               [0, 0, 0, 0, 0, 0, 0, 0, 0],           # Bone objects
               [],                                    # Local transform
               []                                     # Cumulated transform
]



interested = BuildInitialTransform(interestedArmature, interested)



Blender.Redraw()
