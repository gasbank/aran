#!BPY

"""
Name: '000 Aran Exporter (.arn)'
Blender: 246
Group: 'Export'
Tooltip: 'Aran Exporter (Blender 2.43)'
"""

import math
import Blender
from Blender import sys
from Blender import Ipo
import bpy
import BPyMessages
import array
import Blender.Mathutils
from Blender.Mathutils import Euler
from Blender.Mathutils import Matrix
#from Blender import Ipo

"""
'c' char character 1 
'b' signed char int 1 
'B' unsigned char int 1 
'u' Py_UNICODE Unicode character 2 
'h' signed short int 2 
'H' unsigned short int 2 
'i' signed int int 2 
'I' unsigned int long 2 
'l' signed long int 4 
'L' unsigned long long 4 
'f' float float 4 
'd' double float 8 

"""

def attach_int(val):
	ary = array.array('L')
	ary.append(val)
	binstream.append(ary)

def attach_ints(val):
	ary = array.array('L')
	ary.fromlist(val)
	binstream.append(ary)

def attach_floats(val):
	ary = array.array('f')
	ary.fromlist(val)
	binstream.append(ary)

def attach_str(val):
	ary = array.array('c')
	ary.fromstring(val)
	binstream.append(ary)
	
def attach_strz(val):
	ary = array.array('c')
	ary.fromstring(val)
	ary.append('\0')
	binstream.append(ary)

def attach_zeros(count):
	ary = array.array('B')
	ary.fromlist([0x00] * count)
	binstream.append(ary)

def attach_matrix(mat):
	ary = array.array('f')
	ary.fromlist([mat[0][0], mat[0][1], mat[0][2], mat[0][3]])
	ary.fromlist([mat[1][0], mat[1][1], mat[1][2], mat[1][3]])
	ary.fromlist([mat[2][0], mat[2][1], mat[2][2], mat[2][3]])
	ary.fromlist([mat[3][0], mat[3][1], mat[3][2], mat[3][3]])
	binstream.append(ary)
	
def WriteHLine():
	out.write('*****************************************************\n')

def ColorStr(col, alpha):
	return '[%.2f %.2f %.2f ; %.2f]' % (col[0], col[1], col[2], alpha);

def EulerToAxisDegString(eul):
	quat = eul.toQuat()
	return 'Axis %.2f %.2f %.2f ; Deg %.2f' % (quat.axis.x, quat.axis.y, quat.axis.z, quat.angle)

def XformMatrixWrite(mat):
	out.write('%s\n' % mat)
	scaleVec = mat.scalePart()
	transVec = mat.translationPart()
	rotatEuler = mat.toEuler()
	out.write('Scale: %s / Trans: %s / Rotate %s\n' % (scaleVec, transVec, rotatEuler))

def VectorToString(v):
	return '%.2f %.2f %.2f' % (v.x, v.y, v.z)

def EulerRadToDeg(eul):
	return Euler([math.degrees(eul.x), math.degrees(eul.y), math.degrees(eul.z)])

def EulerDegToRad(eul):
	return Euler([math.radians(eul.x), math.radians(eul.y), math.radians(eul.z)])

def MatrixToDetailString(mat):
	scaleVec = mat.scalePart()
	transVec = mat.translationPart()
	rotatEulerDeg = mat.toEuler()
	rotatEulerRad = EulerDegToRad(rotatEulerDeg)
	ret = ''
	ret = ret + 'Loc        : %s\n' % VectorToString(transVec)
	ret = ret + 'Size       : %s\n' % VectorToString(scaleVec)
	ret = ret + 'Rot (Rad)  : %s\n' % VectorToString(rotatEulerRad)
	ret = ret + 'Rot (Deg)  : %s\n' % VectorToString(rotatEulerDeg)
	ret = ret + 'Rot        : %s\n' % rotatEulerDeg.toQuat()
	ret = ret + 'Rot        : %s\n' % EulerToAxisDegString(rotatEulerDeg)
	return ret

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


def InterpModeToString(interp):
	if interp == 0: return 'CONST'
	elif interp == 1: return 'LINEAR'
	elif interp == 2: return 'BEZIER'


#############################################################################################################

def export_node_mesh(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	if ob.parent: parName = ob.parent.name
	
	out.write('<ArnType 0x00002002> %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write(MatrixToDetailString(matLocal))
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
		out.write('v %.2f %.2f %.2f n %.2f %.2f %.2f\n' % (vert.co.x, vert.co.y, vert.co.z, vert.no.x, vert.no.y, vert.no.z))
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
		
		
	# *** Binary Writing Phase ***
	attach_int(0x00002002)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_strz(parName)
	attach_matrix(matLocal)
	attach_floats([0.0] * 16) # reserved space
	attach_ints([len(mesh.materials), len(mesh.verts), len(mesh.faces)])
	attach_ints(localGlobalMatMap)
	
	binstream.append(vertary)
	binstream.append(faceary)
	binstream.append(attrary)

def export_node_camera(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	if ob.parent: parName = ob.parent.name
	
	out.write('<ArnType 0x00007001> %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write(MatrixToDetailString(matLocal))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	
	cam = ob.getData(mesh=0)
	if (cam.type == 'ortho'): camType = 1
	else: camType = 0
	out.write('Type: %s\n' % cam.type)
	out.write('Angle: %.2f\n' % cam.angle)
	out.write('Clip: %.2f ~ %.2f\n' % (cam.clipStart, cam.clipEnd))
	out.write('Scale: %.2f\n' % cam.scale)

	# *** Binary Writing Phase ***
	attach_int(0x00007001)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_strz(parName)
	attach_matrix(matLocal)
	attach_int(camType)
	attach_floats([cam.angle, cam.clipStart, cam.clipEnd, cam.scale])

def export_node_lamp(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	if ob.parent: parName = ob.parent.name
	
	out.write('<ArnType 0x00003001> %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write(MatrixToDetailString(matLocal))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	
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
	
	out.write('<ArnType 0x00000030> %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write(MatrixToDetailString(matLocal))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	
	out.write('Type  : %s\n' % lampType)
	out.write('RGB   : %.2f %.2f %.2f\n' % (lamp.R, lamp.G, lamp.B))
	out.write('Clip  : %.2f ~ %.2f\n' % (lamp.clipStart, lamp.clipEnd))
	
	
	# *** Binary Writing Phase ***
	attach_int(0x00003001)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_strz(parName)
	attach_matrix(matLocal)
	attach_int(d3dType)
	#             _________Diffuse_____________   __________Specular___________   ____________Ambient__________
	attach_floats([lamp.R, lamp.G, lamp.B, 1.0] + [lamp.R, lamp.G, lamp.B, 1.0] + [lamp.R, lamp.G, lamp.B, 1.0])
	#             ________Position___________   ______Direction____________
	attach_floats([ob.LocX, ob.LocY, ob.LocZ] + [ob.RotX, ob.RotY, ob.RotZ])
	#              Cutoff range       Falloff         Att0       Att1        Att2
	attach_floats([    0.5,            0.5,            0.3,   lamp.quad1, lamp.quad2])
	#              Inner       Outer angle of spotlight cone
	attach_floats([1.0,         2.0])

def export_materials():
	obName = 'Global Materials Node'
	materialCount = len(bpy.data.materials)
	matNodeChunkSize = 4 * (4*4 + 1)
	out.write('-----------------------------------------------------------------\n')
	out.write('|                   GLOBAL MATERIALS NODE                       |\n')
	out.write('-----------------------------------------------------------------\n')
	out.write('<ArnType 0x00009000> %s\n' % obName)
	out.write('Node Chunk Size : %d\n' % (matNodeChunkSize * materialCount))
	out.write('Material Count  : %d\n' % materialCount)
	
	# *** Binary Writing Phase *** ; Global Materials Node Start
	attach_int(0x00009000)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_int(materialCount)
	# *** Binary Writing Phase *** ; Global Materials Node End
	
	matGlobalIndex = 0 # 0 is reserved for default material
	parName = obName # parent name
	for mat in bpy.data.materials:
		WriteHLine()
		matName = mat.name
		out.write('MatGlobalIndex: %d\n' % matGlobalIndex)
		out.write('<ArnType 0x00009001> %s\n' % matName)
		out.write('Node Chunk Size : %d\n' % matNodeChunkSize)
		out.write('Parent Name     : %s\n' % parName)
		out.write(' - Diffuse  (Col) : %s\n' % ColorStr(mat.rgbCol,  mat.alpha))
		out.write(' - Ambient  (Mir) : %s\n' % ColorStr(mat.mirCol,  mat.alpha))
		out.write(' - Specular (Spe) : %s\n' % ColorStr(mat.specCol, mat.alpha))
		out.write(' - Emissive (???) : [0.00 0.00 0.00 ; 0.00]\n')
		out.write(' - Power    (???) : 1.00\n')
		
		matDic[mat.name] = matGlobalIndex
		matGlobalIndex = matGlobalIndex + 1
		
		texs = mat.getTextures()
		for tex in texs:
			if tex != None:
				if tex.tex:
					#print tex.tex.image
					pass
					
		# *** Binary Writing Phase *** ; Individual material data node
		attach_int(0x00009001)
		attach_strz(matName)
		attach_int(matNodeChunkSize)                      # Node Chunk SIze
		# ------------------------------------------------
		attach_strz(parName)
		#            _______Diffuse___________   ________Ambient___________   ________Specular_________   ______Emissive______   Power
		attach_floats(mat.rgbCol  + [mat.alpha] + mat.mirCol  + [mat.alpha] + mat.specCol + [mat.alpha] + [0.0, 0.0, 0.0, 0.0] + [1.0])
		# *** Binary Writing Phase *** ; Individual material data End


def BezPointToString(bez):
	return 'Knot : (%.2f,%.2f) / Handles: (%.2f,%.2f) (%.2f,%.2f)\n' % (bez.pt[0], bez.pt[1], bez.vec[0][0], bez.vec[0][1], bez.vec[2][0], bez.vec[2][1])

def NonbezPointToString(bez):
	return 'Point: (%.2f,%.2f)\n' % (bez.pt[0], bez.pt[1])

def export_ipos():
	obName = 'Global IPOs Node'
	ipoCount = len(Ipo.Get())
	out.write('-----------------------------------------------------------------\n')
	out.write('|                      GLOBAL IPOS NODE                         |\n')
	out.write('-----------------------------------------------------------------\n')
	out.write('<ArnType 0x0000A000> %s\n' % obName)
	out.write('Node Chunk Size : { not calculated }\n')
	out.write('IPO Count  : %d\n' % ipoCount)
	
	# *** Binary Writing Phase *** ; Global IPOs Node Start
	attach_int(0x0000A000)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_int(ipoCount)
	# *** Binary Writing Phase *** ; Global IPOs Node End
	
	parName = obName # parent name
	for ipo in Ipo.Get():
		WriteHLine()
		ipoName = ipo.name
		ipoCurveCount = len(ipo.curves)
		out.write('<ArnType 0x0000A001> %s\n' % ipoName)
		out.write('Node Chunk Size : { not calculated }\n')
		out.write('Parent Name     : %s\n' % parName)
		out.write('Curve Count     : %d\n' % ipoCurveCount)
		
		attach_int(0x0000A001)
		attach_strz(ipoName)
		attach_int(0)                      # Node Chunk SIze
		# ------------------------------------------------
		attach_strz(parName)
		attach_int(ipoCurveCount)
		
		
		for curve in ipo.curves:
			curveCount = len(curve.bezierPoints)
			out.write(' - Curve Name   : %s\n' % curve.name)
			out.write(' - Point count  : %d\n' % curveCount)
			out.write(' - Interp Type  : %s\n' % InterpModeToString(curve.interpolation))
			
			pointary = array.array('f')
			for point in curve.bezierPoints:
				if curve.interpolation == 2: # bezier
					out.write(BezPointToString(point))
				else:
					out.write(NonbezPointToString(point))
				pointary.fromlist([point.pt[0], point.pt[1], point.vec[0][0], point.vec[0][1], point.vec[2][0], point.vec[2][1]])
		
			attach_strz(curve.name)
			attach_int(curveCount)
			attach_int(curve.interpolation)
			binstream.append(pointary)


def start_export(filename):
	attach_strz('ARN25')          # File Descriptor
	attach_int(0)                 # Node Count (unused)
	
	sce = bpy.data.scenes.active
	export_materials()
	export_ipos()
	for ob in sce.objects:
		WriteHLine()
		"""
		if ob.ipo is not None:
			out.write('IPO: %s\n' % ob.ipo.name)
		"""
		
		if ob.type == 'Mesh':
			export_node_mesh(ob)
		elif ob.type == 'Camera':
			export_node_camera(ob)
		elif ob.type == 'Lamp':
			export_node_lamp(ob)
		else:
			print ob.name, 'is not supported type; skipping'
	
	attach_strz('TERM')           # Terminal Descriptor
	
	for bi in binstream: # write array collection to file
		bi.tofile(outbin)
		
	out.close()
	outbin.close()
	print '== FINISHED =='






#############################################################################################################

global matDic        # material dictionary
global outbin        # out binary ARN
global out           # out text ARN (for debug)
global binstream     # array collection

#Blender.Window.FileSelector(write_obj, 'Aran Export', sys.makename(ext='.txt'))
fileName = 'e:/devel/aran_svn/working/models/gus2.arn'

matDic       = {}
outbin       = file(fileName, 'wb')
out          = file(fileName + '.txt', 'w')
binstream    = []

start_export(fileName)

