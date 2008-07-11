#!BPY

"""
Name: 'Aran Exporter (.arn)'
Blender: 246
Group: 'Export'
Tooltip: 'Aran Exporter'
"""

import math
import Blender
from Blender import sys
import bpy
import BPyMessages
import array
import Blender.Mathutils
from Blender.Mathutils import Euler
from Blender.Mathutils import Matrix
#from Blender import Ipo

def XformMatrixWrite(out, mat):
	out.write('%s\n' % mat)
	scaleVec = mat.scalePart()
	transVec = mat.translationPart()
	rotatEuler = mat.toEuler()
	out.write('Scale: %s / Trans: %s / Rotate %s\n' % (scaleVec, transVec, rotatEuler))

def MatrixAxisTransform(mat):
	matLocal = Matrix(mat)
	scaleVec = matLocal.scalePart()
	transVec = matLocal.translationPart()
	rotatEuler = matLocal.toEuler()
	"""
	print 'Before Axis Transform'
	print matLocal
	print 'DECOMPOSITION'
	print 'Scale: ', scaleVec
	print 'Trans: ', transVec
	print 'Rotat: ', rotatEuler
	print
	"""
	transVec.z = -transVec.z
	rotatEuler.x = -rotatEuler.x
	rotatEuler.y = -rotatEuler.y
	scaleMat = Matrix([scaleVec.x, 0, 0, 0], [0, scaleVec.y, 0, 0], [0, 0, scaleVec.z, 0], [0, 0, 0, 1])
	transMat = Matrix([1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [transVec.x, transVec.y, transVec.z, 1])
	rotatMat = rotatEuler.toMatrix().resize4x4()
	matLocal = scaleMat * rotatMat * transMat
	"""
	print matLocal
	print 'DECOMPOSITION'
	print 'Scale: ', matLocal.scalePart()
	print 'Trans: ', matLocal.translationPart()
	print 'Rotat: ', matLocal.toEuler()
	"""
	return matLocal


def export_mesh(out, outbin, ob, matDic):
	degRot = Euler([math.degrees(-ob.rot.x), math.degrees(-ob.rot.y), math.degrees(ob.rot.z)])
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	out.write('%s <ArnType 0x10>\n' % obName)
	out.write('Loc        : %f %f %f\n' % (ob.LocX, ob.LocY, ob.LocZ))
	out.write('Size       : %f %f %f\n' % (ob.SizeX, ob.SizeY, ob.SizeZ))
	out.write('Rot (Rad)  : %f %f %f\n' % (ob.rot.x, ob.rot.y, ob.rot.z))
	out.write('Rot (Deg)  : %f %f %f\n' % (degRot.x, degRot.y, degRot.z))
	out.write('RotQuat    :\n')
	out.write('%s\n' % degRot.toQuat())
	out.write('Axis: (%f, %f, %f) Deg: %f\n' % (degRot.toQuat().axis.x, degRot.toQuat().axis.y, degRot.toQuat().axis.z, degRot.toQuat().angle))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	mesh = ob.getData(mesh=1)
	out.write('Materials  : %d\n' % len(mesh.materials))
	out.write('Verts      : %d\n' % len(mesh.verts))
	out.write('Faces      : %d\n' % len(mesh.faces))
	
	out.write('--Materials--\n');
	localGlobalMatMap = []
	localMatIndex = 0
	for material in mesh.materials:
		globalMatIndex = 0
		matNameShort = ''
		if material is None:
			globalMatIndex = 0
			matNameShort = '<NoneType material>'
		else:
			globalMatIndex = matDic[material.name]
			matNameShort = material.name
		out.write('[%d -> %d] %s\n' % (localMatIndex, globalMatIndex, matNameShort))
		localGlobalMatMap.append(globalMatIndex)
		localMatIndex = localMatIndex + 1;
	
	out.write('--Verts--\n');
	vertary = array.array('f')
	for vert in mesh.verts:
		out.write('v %f %f %f n %f %f %f\n' % (vert.co.x, vert.co.y, vert.co.z, vert.no.x, vert.no.y, vert.no.z))
		vertary.fromlist([vert.co.x, vert.co.y, -vert.co.z, vert.no.x, vert.no.y, -vert.no.z])
	
	out.write('--Faces with material index--\n');
	faceary = array.array('H')
	attrary = array.array('L')
	for face in mesh.faces:
		out.write('f [%d]' % face.mat)
		attrary.append(face.mat)
		if len(face.v) is not 3:
			print '### error: all faces should consist of three vertices ###'
			return;
		for vert in face.v:
			out.write(' %i' % vert.index)
			faceary.append(vert.index)
		out.write('\n')
		
	bin = []
	ary = array.array('L')
	ary.append(0x10)
	bin.append(ary)
	ary = array.array('c')
	ary.fromstring(obName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(obName)))
	bin.append(ary)
	parName = ''
	if ob.parent: parName = ob.parent.name
	ary = array.array('c')
	ary.fromstring(parName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(parName)))
	bin.append(ary)
	ary = array.array('f')

	ary.fromlist([matLocal[0][0], matLocal[0][1], matLocal[0][2], matLocal[0][3]])
	ary.fromlist([matLocal[1][0], matLocal[1][1], matLocal[1][2], matLocal[1][3]])
	ary.fromlist([matLocal[2][0], matLocal[2][1], matLocal[2][2], matLocal[2][3]])
	ary.fromlist([matLocal[3][0], matLocal[3][1], matLocal[3][2], matLocal[3][3]])
	
	
	ary.fromlist([ob.LocX, ob.LocY, -ob.LocZ])
	ary.fromlist([ob.SizeX, ob.SizeY, ob.SizeZ])
	ary.fromlist([ob.rot.x, ob.rot.y, ob.rot.z])
	ary.fromlist([degRot.toQuat().x, degRot.toQuat().y, degRot.toQuat().z, degRot.toQuat().w])
	bin.append(ary)
	ary = array.array('L')
	ary.fromlist([len(mesh.materials), len(mesh.verts), len(mesh.faces)])
	ary.fromlist(localGlobalMatMap)
	bin.append(ary)
	bin.append(vertary)
	bin.append(faceary)
	bin.append(attrary)
	for bi in bin:
		bi.tofile(outbin)

def export_camera(out, outbin, ob):
	obName = ob.name
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	out.write('%s <ArnType 0x20>\n' % obName)
	XformMatrixWrite(out, matLocal)
	cam = ob.getData(mesh=0)
	out.write('Type: %s\n' % cam.type)
	out.write('Angle: %f\n' % cam.angle)
	out.write('Clip: %f ~ %f\n' % (cam.clipStart, cam.clipEnd))
	out.write('Scale: %f\n' % cam.scale)

	bin = []
	ary = array.array('L')
	ary.append(0x20)
	bin.append(ary)
	ary = array.array('c')
	ary.fromstring(obName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(obName)))
	bin.append(ary)
	parName = ''
	if ob.parent: parName = ob.parent.name
	ary = array.array('c')
	ary.fromstring(parName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(parName)))
	bin.append(ary)
	ary = array.array('f')
	ary.fromlist([matLocal[0][0],matLocal[0][1],matLocal[0][2],matLocal[0][3]])
	ary.fromlist([matLocal[1][0],matLocal[1][1],matLocal[1][2],matLocal[1][3]])
	ary.fromlist([matLocal[2][0],matLocal[2][1],matLocal[2][2],matLocal[2][3]])
	ary.fromlist([matLocal[3][0],matLocal[3][1],matLocal[3][2],matLocal[3][3]])
	ary.fromlist([ob.LocX, ob.LocY, ob.LocZ])
	ary.fromlist([ob.RotX, ob.RotY, ob.RotZ])
	bin.append(ary)
	ary = array.array('L')
	if cam.type == 'ortho': ary.append(1)
	else: ary.append(0)
	bin.append(ary)
	ary = array.array('f')
	ary.fromlist([cam.angle, cam.clipStart, cam.clipEnd, cam.scale])
	bin.append(ary)
	for bi in bin:
		bi.tofile(outbin)

def export_lamp(out, outbin, ob):
	lamp = ob.getData(mesh=0)
	if lamp.type == 0:
		lampType = 'Lamp'
		d3dType = 1 # point light
	elif lamp.type == 1:
		lampType = 'Sun'
		d3dType = 3 # directional light
	elif lamp.type == 2:
		lampType = 'Spot'
		d3dType = 2 # spot light
	else:
		print 'Unsupported lamp type.'
		return
	
	obName = ob.name
	out.write('%s <ArnType 0x30>\n' % obName)
	out.write('Loc: %f %f %f\n' % (ob.LocX, ob.LocY, ob.LocZ))
	out.write('Rot: %f %f %f\n' % (ob.RotX, ob.RotY, ob.RotZ))
	out.write('Type: %s\n' % lampType)
	out.write('RGB: %f %f %f\n' % (lamp.R, lamp.G, lamp.B))
	out.write('Clip: %f ~ %f\n' % (lamp.clipStart, lamp.clipEnd))
	
	bin = []
	ary = array.array('L')
	ary.append(0x30)
	bin.append(ary)
	ary = array.array('c')
	ary.fromstring(obName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(obName)))
	bin.append(ary)
	parName = ''
	if ob.parent: parName = ob.parent.name
	ary = array.array('c')
	ary.fromstring(parName)
	bin.append(ary)
	ary = array.array('B')
	ary.fromlist([0x00]*(64-len(parName)))
	bin.append(ary)
	ary = array.array('f')
	ary.fromlist([ob.matrixLocal[0][0],ob.matrixLocal[0][1],ob.matrixLocal[0][2],ob.matrixLocal[0][3]])
	ary.fromlist([ob.matrixLocal[1][0],ob.matrixLocal[1][1],ob.matrixLocal[1][2],ob.matrixLocal[1][3]])
	ary.fromlist([ob.matrixLocal[2][0],ob.matrixLocal[2][1],ob.matrixLocal[2][2],ob.matrixLocal[2][3]])
	ary.fromlist([ob.matrixLocal[3][0],ob.matrixLocal[3][1],ob.matrixLocal[3][2],ob.matrixLocal[3][3]])
	bin.append(ary)
	ary = array.array('L')
	ary.append(d3dType) # light type
	bin.append(ary)
	ary = array.array('f')
	ary.fromlist([lamp.R, lamp.G, lamp.B, 1.0]) # diffuse
	ary.fromlist([lamp.R, lamp.G, lamp.B, 1.0]) # specular
	ary.fromlist([lamp.R, lamp.G, lamp.B, 1.0]) # ambient
	ary.fromlist([ob.LocX, ob.LocY, ob.LocZ]) # position
	ary.fromlist([ob.RotX, ob.RotY, ob.RotZ]) # direction
	ary.fromlist([0.5, 0.5, 0.3, lamp.quad1, lamp.quad2]) # cutoff range, falloff, att0, att1, att2
	ary.fromlist([1.0, 2.0]) # inner/outer angle of spotlight cone
	bin.append(ary)
	for bi in bin:
		bi.tofile(outbin)
		
def export_material(out, outbin, matDic):
	matGlobalIndex = 0 # 0 is reserved for default material
	for mat in bpy.data.materials:
		matName = mat.name
		out.write('********************************************************************************\n')
		out.write('MatGlobalIndex: %d\n' % matGlobalIndex)
		matDic[mat.name] = matGlobalIndex
		matGlobalIndex = matGlobalIndex + 1
		out.write('%s <ArnType 0x40>\n' % matName)
		out.write('Diffuse (Col): %s %f\n' % (mat.rgbCol.__str__(),  mat.alpha))
		out.write('Ambient (Mir): %s %f\n' % (mat.mirCol.__str__(),  mat.alpha))
		out.write('Specular(Spe): %s %f\n' % (mat.specCol.__str__(), mat.alpha))
		out.write('Emissive(???): [0.0, 0.0, 0.0] 0.0\n')
		out.write('Power   (???): 1.0\n')
		texs = mat.getTextures()
		for tex in texs:
			if tex != None:
				if tex.tex:
					#print tex.tex.image
					pass
		bin = []
		ary = array.array('L')
		ary.append(0x40)
		bin.append(ary)
		ary = array.array('c')
		ary.fromstring(matName)
		bin.append(ary)
		ary = array.array('B')
		ary.fromlist([0x00]*(64-len(matName)))
		bin.append(ary)
		
		ary = array.array('f')
		# diffuse, ambient, specular, emissive
		ary.fromlist(mat.rgbCol  + [mat.alpha])
		ary.fromlist(mat.mirCol  + [mat.alpha])
		ary.fromlist(mat.specCol + [mat.alpha])
		ary.fromlist([0.0, 0.0, 0.0, 0.0])
		# power
		ary.append(1.0)
		bin.append(ary)
		for bi in bin:
			bi.tofile(outbin)		

def write_obj(filename):
	outbin = file(filename, 'wb')
	out = file(filename+'.txt', 'w')
	sce = bpy.data.scenes.active
	export_material(out, outbin, matDic)
	for ob in sce.objects:
		out.write('********************************************************************************\n')
		if ob.ipo is not None: out.write('IPO: %s\n' % ob.ipo.name)
		if ob.type == 'Mesh': export_mesh(out, outbin, ob, matDic)
		elif ob.type == 'Camera': export_camera(out, outbin, ob)
		elif ob.type == 'Lamp': export_lamp(out, outbin, ob)
		else: print ob.name, 'is not supported type.'
	out.close()
	outbin.close()
	
	print '== FINISHED =='

matDic = {}
write_obj('e:/devel/aran_svn/working/models/gus2.arn')
#Blender.Window.FileSelector(write_obj, 'Aran Export', sys.makename(ext='.txt'))