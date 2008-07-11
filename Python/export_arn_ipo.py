#!BPY

"""
Name: '00 Aran Exporter IPO Test'
Blender: 246
Group: 'Export'
Tooltip: 'Aran Exporter'
"""

import Blender
from Blender import Ipo
#from Blender import Armature
from Blender.Armature import NLA
#import bpy

def interpModeToString(interp):
	if interp == 0: return 'CONST'
	elif interp == 1: return 'LINEAR'
	elif interp == 2: return 'BEZIER'


print '============ IPOs ============'
for ipo in Ipo.Get():
	print 'Name:', ipo.name
	for curve in ipo.curves:
		print '   ', curve.name, '/ Points:', len(curve.bezierPoints), '/ Interp:', interpModeToString(curve.interpolation)
		for point in curve.bezierPoints:
			if curve.interpolation == 2: # bezier
				print 'Pt:', point.pt, '/ Handle:[', point.vec[0][0], ',', point.vec[0][1], '] / Handle:[', point.vec[2][0], ',', point.vec[2][1], ']'
			else:
				print 'Pt:', point.pt

"""
print '============ ACTIONS ============'
for actionName in NLA.GetActions():
	action = NLA.GetActions()[actionName]
"""
