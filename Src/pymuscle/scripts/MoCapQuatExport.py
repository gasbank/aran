import bpy
from Blender.Mathutils import *

def TransformFromIpoCurves(ipo, frame):
	loc = [0, 0, 0]
	q = Quaternion()
	if ipo == 0 or ipo == None:
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

class Bone:
	def __init__(self, name, parentName, length, restQ):
		self.name = name
		self.parentName = parentName
		self.length = length
		self.restQ  = restQ

if __name__ == '__main__':
	armaName = 'Armature.Walk0'
	arma = bpy.data.objects[armaName]
	bones = [v for v in arma.getData().bones.values()]
	boneNameList = [v.name for v in bones]
	#print [v.name for v in bones]
	#print [v.length for v in bones]
	arma.getData().restPosition = True
	if arma.action:
		header = []
		for c in arma.action.getAllChannelIpos():
			try:
				idx = boneNameList.index(c)
			except:
				idx = None
			if idx is not None:
				if bones[idx].parent:
					parentName = bones[idx].parent.name
				else:
					parentName = '**NONE**'
				restQ = bones[idx].matrix['BONESPACE'].toQuat()
				header.append( Bone(c, parentName, bones[idx].length, restQ) )
		arma.getData().restPosition = False
		oFile = open('mocap.txt', 'w')
		for b in header:
			oFile.write('%-15s %-15s %f %f %f %f %f\n' % (b.name, b.parentName, b.length, b.restQ.w, b.restQ.x, b.restQ.y, b.restQ.z))
		
		for f in xrange(3000):
			for c in [b.name for b in header]:
				ipo = arma.action.getChannelIpo(c)
				assert ipo
				#print c, ipo.name
				L, Q = TransformFromIpoCurves(ipo, f/10.0)
				oFile.write(str(L[0]) + ' ')
				oFile.write(str(L[1]) + ' ')
				oFile.write(str(L[2]) + ' ')
				oFile.write(str(Q.w) + ' ')
				oFile.write(str(Q.x) + ' ')
				oFile.write(str(Q.y) + ' ')
				oFile.write(str(Q.z) + ' ')
			oFile.write('\n')
		oFile.close()
	else:
		print armaName, 'does not have any action.'