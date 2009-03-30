#!BPY

"""
Name: '000 Aran Exporter (.arn)'
Blender: 246
Group: 'Export'
Tooltip: 'Aran Exporter (Blender 2.43)'
"""

import math
import Blender
from Blender import sys, Window, Ipo, Armature, Modifier
from Blender.Armature import NLA
import bpy
import BPyMessages
import array
import Blender.Mathutils
from Blender.Mathutils import *

"""
'c' char character 1 
'b' signed char int 1 
'B' unsigned char int 1 
'u' Py_UNICODE Unicode character 2 
'h' signed short int 2 
'H' unsigned short int 2 
'i' signed int int 2 
'I' unsigned int long 2 
'I' signed long int 4 
'I' unsigned long long 4 
'f' float float 4 
'd' double float 8 

"""

def attach_int(val):
	ary = array.array('I')
	ary.append(val)
	binstream.append(ary)

def attach_ints(val):
	ary = array.array('I')
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

def attach_strzs(val):
	for v in val:
		ary = array.array('c')
		ary.fromstring(v)
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

def MatrixToDetailString(mat, precedingSpace = 0):
	scaleVec = mat.scalePart()
	transVec = mat.translationPart()
	rotatEulerDeg = mat.toEuler()
	rotatEulerRad = EulerDegToRad(rotatEulerDeg)
	ret = ''
	ret = ret + ' ' * precedingSpace + 'Loc        : %s\n' % VectorToString(transVec)
	ret = ret + ' ' * precedingSpace + 'Size       : %s\n' % VectorToString(scaleVec)
	ret = ret + ' ' * precedingSpace + 'Rot (Rad)  : %s\n' % VectorToString(rotatEulerRad)
	ret = ret + ' ' * precedingSpace + 'Rot (Deg)  : %s\n' % VectorToString(rotatEulerDeg)
	ret = ret + ' ' * precedingSpace + 'Rot        : %s\n' % rotatEulerDeg.toQuat()
	ret = ret + ' ' * precedingSpace + 'Rot        : %s\n' % EulerToAxisDegString(rotatEulerDeg)
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

def BoneWeightCompareDesc(w1, w2):
	if w1[1] > w2[1]:
		return -1
	elif w1[1] == w2[1]:
		return 0
	else:
		return 1
		
def export_node_mesh(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	ipoName = ''
	if ob.parent: parName = ob.parent.name
	if ob.ipo is not None: ipoName = ob.ipo.name
	
	"""
	if ob.ipo is not None:
		out.write('(*)matrixLocal will be ignored by IPO data\n')
		matLocal.identity()
	"""
	
	mesh = ob.getData(mesh=1)
	if len(mesh.verts) is 0:
		print ob.name, ': No vertices; skipping'
		return
	for face in mesh.faces: face.sel = 1
	mesh.quadToTriangle()
	
	out.write('{ ArnType 0x00002002   NDT_MESH3 } %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write('IPO        : %s\n' % ipoName)
	out.write(MatrixToDetailString(matLocal))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	
	
	out.write('Materials         : %d\n' % len(mesh.materials))
	out.write('Original Vertices : %d\n' % len(mesh.verts))
	out.write('Faces             : %d\n' % len(mesh.faces))
	
	out.write('--Materials--\n');
	localGlobalMatMap = []
	matNameList = []
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
		
		matNameList.append(material.name)

	# Print original vertices
	"""
	out.write('--Orignal Vertices--\n');
	vertary = array.array('f')
	for ii in range(len(mesh.verts)):
		vert = mesh.verts[ii]
		out.write('v %4i: %10.2f %10.2f %10.2f / n %10.2f %10.2f %10.2f\n' % (ii, vert.co.x, vert.co.y, vert.co.z, vert.no.x, vert.no.y, vert.no.z))
		vertary.fromlist([vert.co.x, vert.co.y, -vert.co.z, vert.no.x, vert.no.y, -vert.no.z])
	"""
	
	out.write('--Faces with material/original vertex index--\n');
	faceary = array.array('H')
	attrary = array.array('I')
	vertUVMap = {}
	addedVertMap = []
	vertDupCount = [0] * len(mesh.verts)
	for face in mesh.faces:
		out.write('f [%d]' % face.mat)
		attrary.append(face.mat)
		faceVertCount = len(face.v)
		if faceVertCount is not 3: # Mesh faces should consist of triangles only!
			print '### error: all faces should consist of three vertices ###'
			BPyMessages.Error_NoMeshFaces()
			return;
		for ii in range(faceVertCount):
			vert = face.v[ii]
			dupCount = 0
			if mesh.faceUV: # if UV texturing is applied
				faceVertIdx = vert.index
				if vertUVMap.has_key(faceVertIdx):
					if face.uv[ii] not in vertUVMap[faceVertIdx]:
						vertUVMap[faceVertIdx].append(face.uv[ii])
						vertDupCount[faceVertIdx] = vertDupCount[faceVertIdx] + 1
						addedVertMap.append( ( faceVertIdx, vertDupCount[faceVertIdx] ) )
						dupCount = vertDupCount[faceVertIdx]
					else:
						dupCount = vertUVMap[faceVertIdx].index(face.uv[ii])
				else:
					vertUVMap[faceVertIdx] = [ face.uv[ii] ]
			realVertIdx = vert.index
			outStr = ''
			outStr = ' %i:%i' % (vert.index, dupCount)
			if ( vert.index, dupCount ) in addedVertMap:
				realVertIdx = addedVertMap.index(( vert.index, dupCount))+len(mesh.verts)
				outStr = '%s{%i}' % (outStr, realVertIdx)
			else:
				realVertIdx = vert.index
			out.write('%10s' % outStr)
			faceary.append(realVertIdx)
		
		if mesh.faceUV: out.write(' / UV: %s' % face.uv.__str__())
		
		out.write('\n')
		
	finalVertCount = len(mesh.verts) + len(addedVertMap)
	out.write('--- Final vertices with added vertices ---\n')
	out.write('  Final Vertices Count: %i\n' % finalVertCount)
	
	# Print added vertices and whole UV coordinates
	vertary = array.array('f')
	if mesh.faceUV:
		out.write('%s\n' % addedVertMap)
		for ii in range(len(mesh.verts)):
				out.write('v %d: %s\n' % (ii, vertUVMap[ii]))
	
	# Print original vertices
	for ii in range(len(mesh.verts)):
		vert = mesh.verts[ii]
		if mesh.faceUV:
			texU = vertUVMap[ii][0].x
			texV = vertUVMap[ii][0].y
			
		else:
			texU = 0
			texV = 0
		out.write('v %4i: %10s %10.2f %10.2f %10.2f / n %10.2f %10.2f %10.2f' % (ii, '', vert.co.x, vert.co.y, vert.co.z, vert.no.x, vert.no.y, vert.no.z))
		out.write(' / uv %5.2f %5.2f\n' % (texU, texV))
		vertary.fromlist([vert.co.x, vert.co.y, -vert.co.z])
		vertary.fromlist([vert.no.x, vert.no.y, -vert.no.z])
		vertary.fromlist([texU, abs(texV-1)])

	# Print added vertices
	for ii in range(len(addedVertMap)):
		vert = mesh.verts[addedVertMap[ii][0]]
		faceIdxLong = '%i:%i' % (addedVertMap[ii][0], addedVertMap[ii][1])
		texU = vertUVMap[addedVertMap[ii][0]][addedVertMap[ii][1]].x
		texV = vertUVMap[addedVertMap[ii][0]][addedVertMap[ii][1]].y
		out.write('v %4i: %10s ' % (ii+len(mesh.verts), faceIdxLong))
		out.write('%10.2f %10.2f %10.2f / n %10.2f %10.2f %10.2f' % (vert.co.x, vert.co.y, vert.co.z, vert.no.x, vert.no.y, vert.no.z))
		out.write(' / uv %5.2f %5.2f\n' % (texU, texV))
		vertary.fromlist([vert.co.x, vert.co.y, -vert.co.z])
		vertary.fromlist([vert.no.x, vert.no.y, -vert.no.z])
		vertary.fromlist([texU, abs(texV-1)])
	
	
	
	# Check for modifiers, especially for 'Armature'
	armature_modifier_exist = False
	armature_mod = 0
	if len(ob.modifiers) is not 0:
		for modifier in ob.modifiers:
			if modifier.type == Modifier.Types['ARMATURE']:
				armature_modifier_exist = True
				armature_mod = modifier
				out.write(' @ Armature Modifier exist on this mesh (Name: %s)@\n' % armature_mod[Modifier.Settings.OBJECT].name)
	
	vertWeightArray = array.array('f')
	vertMatIdxArray = array.array('B')
	boneWeightList = []
	if armature_modifier_exist:
		out.write('-= Bone Influences against each vertex =-\n')
		
		vertGroup = mesh.getVertGroupNames()
		vertGroupMap = {}
		for vg in vertGroup:
			vertIndList = mesh.getVertsFromGroup(vg, 1)
			weightList = []
			out.write(' ~ %s [Start] ~\n' % vg)
			for ind in vertIndList:
				out.write('   (%4i, %.2f)\n' % (ind[0], ind[1]))
				weightList.append( (ind[0], ind[1]) )
				for addedVertInd in range(len(addedVertMap)):
					if addedVertMap[addedVertInd][0] == ind[0]:
						out.write('   (%4i, %.2f)*\n' % (addedVertInd + len(mesh.verts), ind[1]))
						weightList.append( (addedVertInd + len(mesh.verts), ind[1]) )
			out.write(' ~ %s [End / Total: %i] ~\n\n' % (vg, len(weightList)))
			boneWeightList.append( (vg, weightList) )
			if not vertGroupMap.has_key(vg):
				vertGroupMap[vg] = len(vertGroupMap)
		
		#out.write(vertGroupMap.__str__())
		boneInfPerVert = []
		
		# Print bone weights related to original verts
		for i in range(len(mesh.verts)):
			out.write('v %4i: %10.2f %10.2f %10.2f / Bones:' % (i, mesh.verts[i].co.x, mesh.verts[i].co.y, mesh.verts[i].co.z))
			boneinf_list = mesh.getVertexInfluences(i)
			
			if len(boneinf_list) > 4:
				print '### error: there should be no more than 4 influences per vertex ###'
				BPyMessages.Error_NoMeshFaces()
			
			total_bone_inf = 0
			for boneinf in boneinf_list:
				total_bone_inf = total_bone_inf + boneinf[1]
				
			boneinf_list = sorted(boneinf_list, BoneWeightCompareDesc)
			
			boneinf_list_normalized = []
			for boneinf in boneinf_list:
				out.write(' %s(%.2f)' % (boneinf[0], boneinf[1]/total_bone_inf))
				boneinf_list_normalized.append( (boneinf[0], boneinf[1]/total_bone_inf) )
			
			out.write('\n')
			boneInfPerVert.append( boneinf_list_normalized )
		
		# Print bone weights related to added vertices
		for ii in range(len(addedVertMap)):
			i = addedVertMap[ii][0]
			
			out.write('v %4i(%4i): %10.2f %10.2f %10.2f / Bones:' % (ii+len(mesh.verts), i, mesh.verts[i].co.x, mesh.verts[i].co.y, mesh.verts[i].co.z))
			boneinf_list = mesh.getVertexInfluences(i)
			
			if len(boneinf_list) > 4:
				print '### error: there should be no more than 4 influences per vertex ###'
				BPyMessages.Error_NoMeshFaces()
			
			total_bone_inf = 0
			for boneinf in boneinf_list:
				total_bone_inf = total_bone_inf + boneinf[1]
				
			boneinf_list = sorted(boneinf_list, BoneWeightCompareDesc)
			
			boneinf_list_normalized = []
			for boneinf in boneinf_list:
				out.write(' %s(%.2f)' % (boneinf[0], boneinf[1]/total_bone_inf))
				boneinf_list_normalized.append( (boneinf[0], boneinf[1]/total_bone_inf) )
			
			out.write('\n')
			boneInfPerVert.append( boneinf_list_normalized )
		
		
		# --------------------------
		# C/C++ Vertex data code
		#
		# For each vertex,
		#
		#   -------------------+-------------------------------------------------
		#                      |  3 floats for vertex positions
		#   stored in vertary  |  3 floats for vertex normals
		#                      |  2 floats for uv texture coords
		#   -------------------+-------------------------------------------------
		#                      |  3 floats for three blending weights, and fourth one can be induced
		#                      |  4 ints for bone matrix indices
		#   -------------------+-------------------------------------------------
		#
		# --------------------------
		#out.write(boneInfPerVert.__str__())
		out.write('/* C/C++ Vertex data code */\n')
		out.write('/* x y z    nx ny nz   u v     w0 w1 w2    m0 m1 m2 m3 */\n')
		for i in range(finalVertCount):
			out.write('vert[%3d] = VERTEX( ' % i)
			out.write('%10.6ff, %10.6ff, %10.6ff, ' % (vertary[8*i + 0], vertary[8*i + 1], vertary[8*i + 2])) # Position
			out.write('%10.6ff, %10.6ff, %10.6ff, ' % (vertary[8*i + 3], vertary[8*i + 4], vertary[8*i + 5])) # Normal
			out.write('%10.6ff, %10.6ff, ' % (vertary[8*i + 6], vertary[8*i + 7])) # UV
			weightSum = 0.0

			for j in range(min(range(3), len(boneInfPerVert[i]))):
				out.write('%10.6ff, ' % (boneInfPerVert[i][j][1]))
				weightSum = weightSum + boneInfPerVert[i][j][1]
				vertWeightArray.append(boneInfPerVert[i][j][1])
			for j in range(max(0, 3 - len(boneInfPerVert[i]))):
				out.write('%10.6ff, ' % (1-weightSum))
				vertWeightArray.append(1-weightSum)
			
			boneMatIdx = [255, 255, 255, 255]
			for j in range(len(boneInfPerVert[i])):
				boneMatIdx[j] = 0
				if vertGroupMap.has_key( boneInfPerVert[i][j][0] ):
					boneMatIdx[j] = vertGroupMap[ boneInfPerVert[i][j][0] ]
			out.write('FOUR_BYTES_INTO_DWORD(%d, %d, %d, %d) );\n' % (boneMatIdx[0], boneMatIdx[1], boneMatIdx[2], boneMatIdx[3] ))
			vertMatIdxArray.fromlist(boneMatIdx)
		out.write('\n\n')
		for i in range(len(mesh.faces)*3):
			out.write('f[%3d] = %d;\n' % (i, faceary[i]))
	
	
	# *** Binary Writing Phase ***
	attach_int(0x00002002)
	attach_strz(obName)
	attach_int(0x17622680)                      # Node Chunk Size
	# ---------------------------------------------------
	attach_strz(parName)
	attach_strz(ipoName)
	attach_matrix(matLocal)
	attach_ints([armature_modifier_exist]) # Indicates whether skeleton animation applied to this mesh
	attach_floats([0.0] * 15) # reserved space
	attach_ints([len(mesh.materials), finalVertCount, len(mesh.faces)])
	attach_strzs(matNameList)
	
	binstream.append(vertary)
	binstream.append(faceary)
	binstream.append(attrary)
	
	# Write
	# BoneCount
	#    [Bone Name, vertCount, (vert index, weight), (vert index, weight), ...]
	#    [Bone Name, vertCount, (vert index, weight), (vert index, weight), ...]
	#     ...
	if armature_modifier_exist:
		attach_strz(armature_mod[Modifier.Settings.OBJECT].name)
		attach_int(len(vertGroupMap)) # Should be the same as bone count!
		for vgm in vertGroupMap:
			attach_strz(vgm)
		binstream.append(vertWeightArray)
		binstream.append(vertMatIdxArray)
		attach_int( len(boneWeightList) )
		for bwl in boneWeightList:
			attach_strz(bwl[0])
			attach_int( len(bwl[1]) )
			#out.write('xxxxxxxxxxx;; %i\n' % len(bwl[1]))
			for bw in bwl[1]:
				attach_int(bw[0])
				attach_floats([bw[1]])
	
	mesh.triangleToQuad()

def export_node_camera(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	ipoName = ''
	if ob.parent: parName = ob.parent.name
	if ob.ipo is not None: ipoName = ob.ipo.name
	
	out.write('{ ArnType 0x00007001   NDT_CAMERA2 } %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write('IPO        : %s\n' % ipoName)
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
	attach_strz(ipoName)
	attach_matrix(matLocal)
	attach_int(camType)
	attach_floats([cam.angle, cam.clipStart, cam.clipEnd, cam.scale])

def export_node_lamp(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	ipoName = ''
	if ob.parent: parName = ob.parent.name
	if ob.ipo is not None: ipoName = ob.ipo.name
	
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
		print ob.name, ': Unsupported lamp type; skipping'
		return
	
	out.write('{ ArnType 0x00003001   NDT_LIGHT2 } %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write('IPO        : %s\n' % ipoName)
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
	attach_strz(ipoName)
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
	out.write('{ ArnType 0x00009000   NDT_MATERIAL1 } %s\n' % obName)
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
		out.write('{ ArnType 0x00009001   NDT_MATERIAL2 } %s\n' % matName)
		out.write('Node Chunk Size : %d\n' % matNodeChunkSize)
		out.write('Parent Name     : %s\n' % parName)
		out.write(' - Diffuse  (Col) : %s\n' % ColorStr(mat.rgbCol,  mat.alpha))
		out.write(' - Ambient  (Mir) : %s\n' % ColorStr(mat.mirCol,  mat.alpha))
		out.write(' - Specular (Spe) : %s\n' % ColorStr(mat.specCol, mat.alpha))
		out.write(' - Emissive (???) : [0.00 0.00 0.00 ; 0.00]\n')
		out.write(' - Power    (???) : 1.00\n')
		texs = mat.getTextures()
		texImgs = []
		for tex in texs:
			if tex != None and tex.tex != None and tex.tex.image != None:
				texImgs.append(tex.tex.image.name)
				out.write(' - Textures: %s\n' % tex.tex.image.name)
				
		matDic[mat.name] = matGlobalIndex
		matGlobalIndex = matGlobalIndex + 1
			
		# *** Binary Writing Phase *** ; Individual material data node
		attach_int(0x00009001)
		attach_strz(matName)
		attach_int(matNodeChunkSize)                      # Node Chunk SIze
		# ------------------------------------------------
		attach_strz(parName)
		#            _______Diffuse___________   ________Ambient___________   ________Specular_________   ______Emissive______   Power
		attach_floats(mat.rgbCol  + [mat.alpha] + mat.mirCol  + [mat.alpha] + mat.specCol + [mat.alpha] + [0.0, 0.0, 0.0, 0.0] + [1.0])
		attach_int(len(texImgs))
		attach_strzs(texImgs)
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
	out.write('{ ArnType 0x0000A000   NDT_IPO1 } %s\n' % obName)
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
		out.write('{ ArnType 0x0000A001   NDT_IPO2 } %s\n' % ipoName)
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
				handle1x = point.vec[0][0]
				handle1y = point.vec[0][1]
				knotx    = point.vec[1][0] #point.pt[0]
				knoty    = point.vec[1][1] #point.pt[1]
				handle2x = point.vec[2][0]
				handle2y = point.vec[2][1]
				if curve.name in ['LocZ', 'RotZ']:
					handle1y = -handle1y
					knoty    = -knoty
					handle2y = -handle2y
				if curve.name in ['RotX', 'RotY', 'RotZ']: # Blender stores its Rot IPO's Y values with divided by 10.
					handle1y = handle1y * 10
					knoty    = knoty    * 10
					handle2y = handle2y * 10
				pointary.fromlist([handle1x, handle1y, knotx, knoty, handle2x, handle2y])
		
			attach_strz(curve.name)
			attach_int(curveCount)
			attach_int(curve.interpolation)
			binstream.append(pointary)

def export_particle_system(ob):
	psys_list = ob.getParticleSystems()
	if len(psys_list) == 0:
		return
	print len(psys_list), "particle systems applied on", ob.name
	psys = psys_list[0]
	if psys.type is not 0:
		print "Only EMITTER type of particle system is supported."
		return
	print " - amount:", psys.amount
	print " - startFrame:", psys.startFrame
	print " - endFrame:", psys.endFrame
	print " - lifetime:", psys.lifetime
	print " - randlife:", psys.randlife
	print " - particleDistribution:", psys.particleDistribution

def MakeBoneListRecursive(bone, boneList):
	bone_mat = bone.matrix['ARMATURESPACE']
	#bone_mat = bone.matrix['BONESPACE']
	#bone_mat_world = bone_mat * ar_mat
	out.write('-- Bone:%s' % bone.name)
	boneParentName = ''
	if bone.parent:
		boneParentName = bone.parent.name
		out.write(' (Parent:%s)' % boneParentName)
	out.write('\n')
	out.write('---- Bone World Mat:\n')
	out.write(MatrixToDetailString(bone_mat, 7))
	boneList.append( (bone.name, boneParentName, bone_mat) )	
	
	for boneChild in bone.children:
		MakeBoneListRecursive(boneChild, boneList)

def export_actions():
	obName = 'Global Actions Node'
	actionCount = len(NLA.GetActions())
	out.write('-----------------------------------------------------------------\n')
	out.write('|                    GLOBAL ACTIONS NODE                        |\n')
	out.write('-----------------------------------------------------------------\n')
	out.write('{ ArnType 0x0000C000   NDT_ACTION1 } %s\n' % obName)
	out.write('Node Chunk Size : { not calculated }\n')
	out.write('Action Count  : %d\n' % actionCount)
	
	actList = []
	for actName in NLA.GetActions():
		act = NLA.GetActions()[actName]
		out.write('  - Action Name: %s\n' % act.name)
		channelList = []
		for actIpoName in act.getAllChannelIpos():
			actIpo = act.getAllChannelIpos()[actIpoName]
			out.write('    - Ipo: %s - %s\n' % (actIpoName, actIpo.name))
			channelList.append( (actIpoName, actIpo.name) )
		actList.append( (act.name, channelList) )
	
	# *** Binary Writing Phase *** ; Global IPOs Node Start
	attach_int(0x0000C000)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_int(actionCount)
	for act in actList:
		attach_strz(act[0])
		attach_int(len(act[1])) # Number of channels(bone--ipo correspondence)
		for channel in act[1]:
			attach_strz(channel[0])
			attach_strz(channel[1])
	

def export_node_armature(ob):
	matLocal = MatrixAxisTransform(ob.matrixLocal)
	obName = ob.name;
	parName = '' # parent name
	ipoName = ''
	if ob.parent: parName = ob.parent.name
	if ob.ipo is not None: ipoName = ob.ipo.name
	
	ar_mat = ob.matrixWorld
	ar_data = ob.getData()
	
	out.write('{ ArnType 0x00005001   NDT_HIERARCHY2 } %s\n' % obName)
	out.write('Node Chunk Size: { not calculated }\n')
	out.write(MatrixToDetailString(matLocal))
	out.write('matrixLocal:\n')
	out.write('%s\n' % matLocal)
	
	boneList = []
	bones = ar_data.bones.values()
	boneRoot = 0
	for bone in bones:
		if not bone.parent:
			boneRoot = bone
			break
	
	MakeBoneListRecursive(boneRoot, boneList)
		

	# *** Binary Writing Phase ***
	attach_int(0x00005001)
	attach_strz(obName)
	attach_int(0)                      # Node Chunk SIze
	# ---------------------------------------------------
	attach_strz(parName)
	attach_int(len(bones))
	
	# End of skeleton node data
	
	# Each bone is an individual node
	for b in boneList:
		attach_int(0x00008001)
		attach_strz(b[0])         # Bone name
		attach_int(0)             # Node Chunk SIze
		#------------------------------------------------
		if len(b[1]) is not 0:
			attach_strz( b[1] )       # Bone parent name
		else:
			attach_strz( obName )     # Set bone parent name to Skeleton(Armature) since it is root bone
		
		attach_matrix( b[2] )     # Bone local xform

def start_export(filename):
	attach_strz('ARN25')          # File Descriptor
	attach_int(0)                 # Node Count (unused)
	
	sce = bpy.data.scenes.active
	export_materials()
	export_ipos()
	export_actions()

	for ob in sce.objects:
		WriteHLine()		
		if ob.type == 'Mesh':
			export_node_mesh(ob)
			export_particle_system(ob)
		elif ob.type == 'Camera':
			export_node_camera(ob)
		elif ob.type == 'Lamp':
			export_node_lamp(ob)
		elif ob.type == 'Armature':
			export_node_armature(ob)
		else:
			print 'Object \'', ob.name, '\' of type ', ob.type, ' is not supported type; skipping'
	
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

sceName = bpy.data.scenes.active.name
#Blender.Window.FileSelector(write_obj, 'Aran Export', sys.makename(ext='.txt'))
fileName = '/home/gbu/%s.arn' % sceName

matDic       = {}
outbin       = file(fileName, 'wb')
out          = file(fileName + '.txt', 'w')
binstream    = []

em = Window.EditMode()
Window.EditMode(0)
start_export(fileName)
Window.EditMode(em)

print '== SAVED AS %s ==' % fileName
