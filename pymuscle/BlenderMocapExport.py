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
from BlenderArmatureStatic import *

def XformFromIpoCurves(ipo, frame):
	loc = [0, 0, 0]
	q = Quaternion()
	for c in ipo:
		v = c[frame]
		if c.name == 'LocX': loc[0] = v
		elif c.name == 'LocY': loc[1] = v
		elif c.name == 'LocZ': loc[2] = v
		elif c.name == 'QuatX': q.x = v
		elif c.name == 'QuatY': q.y = v
		elif c.name == 'QuatZ': q.z = v
		elif c.name == 'QuatW': q.w = v	
	return loc, q


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
"""
interested = [ ['Hips', 'LHipJoint', 'RHipJoint',     # Bone names
                'LeftHip', 'RightHip',
                'LeftKnee', 'RightKnee',
                'LeftAnkle', 'RightAnkle'],
               [0, 0, 0, 0, 0, 0, 0, 0, 0],           # IPOs
               [0, 0, 0, 0, 0, 0, 0, 0, 0],           # Bone objects
               [[0, 0, 0]] * 9,                       # Loc
               [Quaternion()] * 9,                    # Rot (quat)
               [[0, 0, 0]] * 9,                       # Initial global loc
               [Quaternion()] * 9,                    # Initial global rot (quat)
               [[0, 0, 0]] * 9,                       # Initial local loc
               [Quaternion()] * 9                     # Initial local rot (quat)
]
"""


interested = BuildInitialTransform(interestedArmature, interested)



Blender.Redraw()
	
"""	
for i in range(200):
	frame = i+1
	Blender.Set('curframe', frame)

	for j in range(5):
		rb = bpy.data.objects['RB.' + interested[0][j]]
		bone = interested[2][j]
		rb.LocX, rb.LocY, rb.LocZ = 0, 0, 0
		rb.RotX, rb.RotY, rb.RotZ = 0, 0, 0
				
		# Calculate the local transformation
		loc, q = XformFromIpoCurves(interested[1][j], frame)
		
		# Multiply local transformation to initial orientation
		q = QuatMult(q, interested[8][j])
		loc = [a+b for a,b in zip(loc,interested[7][j])]
		
		
		# If the parent exists then get it
		# and combine with the local transformation
		if bone.parent:
			for k in range(len(interested[0])):
				if bone.parent.name == interested[0][k]:
					pLoc = interested[3][k]
					pQ = interested[4][k]
			
			loc = [a + b for a, b in zip(loc, pLoc)]
			q = QuatMult(q, pQ)
		
		# Cache accumulated(global) transformation
		# for the bone
		interested[3][j] = loc
		interested[4][j] = q
		
		#loc = [a + b for a, b in zip(loc, interested[5][j])]
		
		
		rb.LocX, rb.LocY, rb.LocZ = loc
		eul = q.toEuler()
		# Should be stored in radians...
		rb.RotX = eul.x / 180.0 * math.pi
		rb.RotY = eul.y / 180.0 * math.pi
		rb.RotZ = eul.z / 180.0 * math.pi
		

	Blender.Redraw()
"""