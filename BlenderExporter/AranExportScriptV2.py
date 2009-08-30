#!BPY

# 2009 Geoyeob Kim
# As a part of Aran project since 2007

"""
Name: '000 Aran Exporter (.xml + .bin)'
Blender: 249
Group: 'Export'
Tooltip: 'Aran Exporter (.xml + .bin)'
"""

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

def createTransformElementFromMatrix(doc, mat):
	scale = mat.scalePart()
	rotEuler = mat.toEuler()
	try:
		trans = mat.translationPart()
	except AttributeError:
		trans = 0
	
	xform = doc.createElement('transform')
	xform.setAttribute('type', 'srt')
	xform_scale = doc.createElement('scaling')
	xform_scale.setAttribute('x', str(scale.x))
	xform_scale.setAttribute('y', str(scale.y))
	xform_scale.setAttribute('z', str(scale.z))
	xform_rot = doc.createElement('rotation')
	xform_rot.setAttribute('type', 'euler')
	xform_rot.setAttribute('unit', 'deg')
	xform_rot.setAttribute('x', str(rotEuler.x))
	xform_rot.setAttribute('y', str(rotEuler.y))
	xform_rot.setAttribute('z', str(rotEuler.z))
	if trans:
		xform_trans = doc.createElement('translation')
		xform_trans.setAttribute('x', str(trans.x))
		xform_trans.setAttribute('y', str(trans.y))
		xform_trans.setAttribute('z', str(trans.z))

	xform.appendChild(xform_scale)
	xform.appendChild(xform_rot)
	if trans:
		xform.appendChild(xform_trans)

	return xform

def createChunk(doc, writePlace, types):
	chunk = doc.createElement('chunk')
	assert(writePlace == 'xml' or writePlace == 'bin')
	chunk.setAttribute('place', writePlace)
	template = doc.createElement('template')
	for (usage, type) in types:
		assert(type == 'float' or type == 'float2' or type == 'float3' or type == 'float8' or type == 'int' or type == 'int3' or type == 'int4')
		fieldElm = doc.createElement('field')
		fieldElm.setAttribute('usage', usage)
		fieldElm.setAttribute('type', type)
		template.appendChild(fieldElm)
	chunk.appendChild(template)

	if writePlace == 'xml':
		arrayData = doc.createElement('arraydata')
		chunk.appendChild(arrayData)
	elif writePlace == 'bin':
		chunk.setAttribute('startoffset', str(binFile.tell()))
		chunk.setAttribute('endoffset', '0')
	else:
		assert(0 and 'Should not happen!')

	return chunk

def appendToChunk(chunk, data):
	writePlace = chunk.getAttribute('place')
	if writePlace == 'xml':
		arrayData = chunk.getElementsByTagName('arraydata')[0]
		dataElm = doc.createElement('data')
		valueStr = ''
		for d in data:
			valueStr = valueStr + str(d) + ';'
		dataElm.setAttribute('value', valueStr)
		arrayData.appendChild(dataElm)
	elif writePlace == 'bin':
		template = chunk.getElementsByTagName('template')[0]
		index = 0
		for t in template.childNodes:
			templType = t.getAttribute('type')
			if templType == 'float':
				d = struct.pack('f', data[index])
				binFile.write(d)
				index = index + 1
			elif templType == 'float2':
				d1 = struct.pack('f', data[index+0])
				d2 = struct.pack('f', data[index+1])
				binFile.write(d1)
				binFile.write(d2)
				index = index + 2
			elif templType == 'float3':
				d1 = struct.pack('f', data[index+0])
				d2 = struct.pack('f', data[index+1])
				d3 = struct.pack('f', data[index+2])
				binFile.write(d1)
				binFile.write(d2)
				binFile.write(d3)
				index = index + 3
			elif templType == 'float8':
				d1 = struct.pack('f', data[index+0])
				d2 = struct.pack('f', data[index+1])
				d3 = struct.pack('f', data[index+2])
				d4 = struct.pack('f', data[index+3])
				d5 = struct.pack('f', data[index+4])
				d6 = struct.pack('f', data[index+5])
				d7 = struct.pack('f', data[index+6])
				d8 = struct.pack('f', data[index+7])
				binFile.write(d1)
				binFile.write(d2)
				binFile.write(d3)
				binFile.write(d4)
				binFile.write(d5)
				binFile.write(d6)
				binFile.write(d7)
				binFile.write(d8)
				index = index + 8
			elif templType == 'int':
				d = struct.pack('i', data[index])
				binFile.write(d)
				index = index + 1
			elif templType == 'int3':
				d1 = struct.pack('i', data[index+0])
				d2 = struct.pack('i', data[index+1])
				d3 = struct.pack('i', data[index+2])
				binFile.write(d1)
				binFile.write(d2)
				binFile.write(d3)
				index = index + 3
			elif templType == 'int4':
				d1 = struct.pack('i', data[index+0])
				d2 = struct.pack('i', data[index+1])
				d3 = struct.pack('i', data[index+2])
				d4 = struct.pack('i', data[index+3])
				binFile.write(d1)
				binFile.write(d2)
				binFile.write(d3)
				binFile.write(d4)
				index = index + 4
			else:
				raise Exception, 'Should not happen!'
	else:
		assert(0 and 'Should not happen!')

def finalizeChunk(chunk):
	if chunk.getAttribute('place') == 'bin':
		assert(int(chunk.getAttribute('startoffset')) <= binFile.tell())
		chunk.setAttribute('endoffset', str(binFile.tell()))


def createCameraData(doc, ob):
	cam = ob.getData(mesh=0)
	cameraData = doc.createElement('camera')
	cameraData.setAttribute('type', cam.type)
	cameraData.setAttribute('fovdeg', str(cam.angle))
	cameraData.setAttribute('nearclip', str(cam.clipStart))
	cameraData.setAttribute('farclip', str(cam.clipEnd))
	cameraData.setAttribute('scale', str(cam.scale))
	return cameraData

def createLightData(doc, ob):
	light = ob.getData(mesh=0)
	lightData = doc.createElement('light')
	lightType = 'Unknown'
	if light.type == 0:
		lightType = 'point'
	elif light.type == 1:
		lightType = 'directional'
	elif light.type == 2:
		lightType = 'spot'
	else:
		assert(0 and 'Should not happen!')
	lightData.setAttribute('type', lightType)
	lightData.setAttribute('r', str(light.R))
	lightData.setAttribute('g', str(light.G))
	lightData.setAttribute('b', str(light.B))
	return lightData

def makeBoneElementRecursive(doc, skelData, bone):
	boneElm = doc.createElement('object')
	boneElm.setAttribute('name', bone.name)
	boneElm.setAttribute('rtclass', 'ArnBone')
	boneXformElm = createTransformElementFromMatrix(doc, bone.matrix['BONESPACE'])
	boneElm.appendChild(boneXformElm)
	
	coordSpace = 'BONESPACE'
	boneTag = doc.createElement('bone')
	headTag = doc.createElement('head')
	headTag.setAttribute('x', str(bone.head[coordSpace].x))
	headTag.setAttribute('y', str(bone.head[coordSpace].y))
	headTag.setAttribute('z', str(bone.head[coordSpace].z))
	tailTag = doc.createElement('tail')	
	tailTag.setAttribute('x', str(bone.tail[coordSpace].x))
	tailTag.setAttribute('y', str(bone.tail[coordSpace].y))
	tailTag.setAttribute('z', str(bone.tail[coordSpace].z))
	rollTag = doc.createElement('roll')
	rollTag.setAttribute('value', str(bone.roll[coordSpace]))
	boneTag.appendChild(headTag)
	boneTag.appendChild(tailTag)
	boneTag.appendChild(rollTag)
	
	boneElm.appendChild(boneTag)
	
	for boneChild in bone.children:
		skelData.appendChild(makeBoneElementRecursive(doc, boneElm, boneChild))
		
	skelData.appendChild(boneElm)
	return skelData

def createSkeletonData(doc, ob):
	skel = ob.getData()
	skelData = doc.createElement('skeleton')
	
	for bone in skel.bones.values():
		if not bone.parent:
			makeBoneElementRecursive(doc, skelData, bone)

	if ob.getPose():
		pbones = ob.getPose().bones.values()
		for pbone in pbones:
			for c in pbone.constraints:
				if c.type is Constraint.Type.LIMITROT:
					cElm = doc.createElement('constraint')
					cElm.setAttribute('type', 'limitrot')
					targetBone = doc.createElement('target')
					targetBone.appendChild(doc.createTextNode(pbone.name))
					cElm.appendChild(targetBone)
					limitFlags = c[Constraint.Settings.LIMIT]
					if limitFlags & 1:
						limitElm = CreateLimitElm(doc, 'AngX', c[Constraint.Settings.XMIN], c[Constraint.Settings.XMAX])
						limitElm.setAttribute('unit', 'deg')
						cElm.appendChild(limitElm)
					if limitFlags & 2:
						limitElm = CreateLimitElm(doc, 'AngY', c[Constraint.Settings.YMIN], c[Constraint.Settings.YMAX])
						limitElm.setAttribute('unit', 'deg')
						cElm.appendChild(limitElm)
					if limitFlags & 4:
						limitElm = CreateLimitElm(doc, 'AngZ', c[Constraint.Settings.ZMIN], c[Constraint.Settings.ZMAX])
						limitElm.setAttribute('unit', 'deg')
						cElm.appendChild(limitElm)
					skelData.appendChild(cElm)
	return skelData
	
def createMeshData(doc, ob):
	mesh = ob.getData(mesh=1)
	meshData = doc.createElement('mesh')
	
	meshData.setAttribute('uv', str(mesh.faceUV))
	if mesh.mode & Mesh.Modes.TWOSIDED:
		meshData.setAttribute('twosided', str(1))
	# Vertex Group
	for group in mesh.getVertGroupNames():
		groupElm = doc.createElement('vertgroup')
		groupElm.setAttribute('name', group)
		
		chunk = createChunk(doc, exportPlace, [('vertid', 'int'), ('weight', 'float')])
		for ind in mesh.getVertsFromGroup(group, 1):
			appendToChunk(chunk, [ind[0], ind[1]])
		finalizeChunk(chunk)
		groupElm.appendChild(chunk)
		meshData.appendChild(groupElm)
	
	# Vertex
	vertex = doc.createElement('vertex')
	chunk = createChunk(doc, exportPlace, [('coordinates', 'float3'), ('normal', 'float3')])
	for v in mesh.verts:
		appendToChunk(chunk, [v.co.x, v.co.y, v.co.z, v.no.x, v.no.y, v.no.z])
	finalizeChunk(chunk)
	vertex.appendChild(chunk)
	meshData.appendChild(vertex)
	
	# Face (material map embedded, tri/quad faces handled separately)
	face = doc.createElement('face')
	faceGroupsByMtrl = {}
	for f in mesh.faces:
		if f.mat in faceGroupsByMtrl:
			faceGroupsByMtrl[f.mat].append(f)
		else:
			faceGroupsByMtrl[f.mat] = [f]
	for mtrlId, faceList in faceGroupsByMtrl.iteritems():
		groupElm = doc.createElement('facegroup')
		groupElm.setAttribute('mtrl', str(mtrlId))
		
		# Tri faces
		triChunk = createChunk(doc, exportPlace, [('index', 'int'), ('vertid', 'int3')])
		for f in faceList:
			if len(f.verts) is 3:
				vertInd = [f.index]
				for v in f.verts:
					vertInd.append(v.index)
				appendToChunk(triChunk, vertInd)
		finalizeChunk(triChunk)
		
		# Quad faces
		quadChunk = createChunk(doc, exportPlace, [('index', 'int'), ('vertid', 'int4')])
		for f in faceList:
			if len(f.verts) is 4:
				vertInd = [f.index]
				for v in f.verts:
					vertInd.append(v.index)
					#vertInd.insert(1, v.index)
				appendToChunk(quadChunk, vertInd)
		finalizeChunk(quadChunk)
		
		groupElm.appendChild(triChunk)
		groupElm.appendChild(quadChunk)
		face.appendChild(groupElm)
	meshData.appendChild(face)
	
	# Linked materials
	for mtrl in mesh.materials:
		mtrlElm = doc.createElement('material')
		mtrlElm.setAttribute('name', 'MTRL_' + mtrl.name)
		#mtrlElm.setIdAttribute('name')
		linkedMaterials.append(mtrl.name) # Remember actually used materials and export them only.
		meshData.appendChild(mtrlElm)
	
	# Texture UV
	if mesh.faceUV:
		uv = doc.createElement('uv')
		triquadUvChunk = createChunk(doc, exportPlace, [('triquaduv', 'float8')])
		for f in mesh.faces:
			uvcoords = []
			for u in f.uv:
				uvcoords.append(u.x)
				uvcoords.append(-(u.y - 0.5) + 0.5)
			if len(f.verts) is 3:
				uvcoords.append(0)
				uvcoords.append(0)
			appendToChunk(triquadUvChunk, uvcoords)
		finalizeChunk(triquadUvChunk)
		uv.appendChild(triquadUvChunk)
		meshData.appendChild(uv)

	return meshData

def exportActions(doc, actionNames):
	scene = doc.firstChild
	for action in bpy.data.actions:
		if action.name in actionNames:
			objElm = doc.createElement('object')
			objElm.setAttribute('name', action.name)
			objElm.setAttribute('rtclass', 'ArnAction')
			actElm = doc.createElement('action')
			objElm.appendChild(actElm)
			for channel in action.getAllChannelIpos():
				ipo = action.getChannelIpo(channel)
				if ipo:
					objipomapElm = doc.createElement('objectipomap')
					objipomapElm.setAttribute('obj', channel)
					objipomapElm.setAttribute('ipo', ipo.name)
					linkedIpos.append(ipo.name)
					actElm.appendChild(objipomapElm)
			scene.appendChild(objElm)


def exportIpos(doc, ipoNames):
	scene = doc.firstChild
	for ipo in bpy.data.ipos:
		if ipo.name in ipoNames:
			objElm = doc.createElement('object')
			objElm.setAttribute('name', ipo.name)
			objElm.setAttribute('rtclass', 'ArnIpo')
			ipoElm = doc.createElement('ipo')
			objElm.appendChild(ipoElm)
			
			for curve in ipo.curves:
				curveElm = doc.createElement('curve')
				curveElm.setAttribute('name', curve.name)
				if curve.interpolation == 0:
					curveElm.setAttribute('type', 'const')
				elif curve.interpolation == 1:
					curveElm.setAttribute('type', 'linear')
				elif curve.interpolation == 2:
					curveElm.setAttribute('type', 'bezier')
				else:
					raise Exception, 'Unknown curve type'
				pointElm = doc.createElement('controlpoint')
				chunk = createChunk(doc, 'bin', [('handle1', 'float2'), ('knot', 'float2'), ('handle2', 'float2')])
				for point in curve.bezierPoints:
					handle1x = point.vec[0][0]
					handle1y = point.vec[0][1]
					knotx    = point.vec[1][0] #point.pt[0]
					knoty    = point.vec[1][1] #point.pt[1]
					handle2x = point.vec[2][0]
					handle2y = point.vec[2][1]
					# Blender stores its Rot IPO's Y values with divided by 10.
					if curve.name in ['RotX', 'RotY', 'RotZ']:
						handle1y = handle1y * 10
						knoty    = knoty    * 10
						handle2y = handle2y * 10
					appendToChunk(chunk, [handle1x, handle1y, knotx, knoty, handle2x, handle2y])
				finalizeChunk(chunk)
				pointElm.appendChild(chunk)
				curveElm.appendChild(pointElm)
				ipoElm.appendChild(curveElm)
			
			scene.appendChild(objElm)


def exportMaterials(doc, mtrlNames):
	scene = doc.firstChild
	for mtrl in bpy.data.materials:
		if mtrl.name in mtrlNames:
			objElm = doc.createElement('object')
			objElm.setAttribute('name', 'MTRL_' + mtrl.name)
			objElm.setAttribute('rtclass', 'ArnMaterial')
			mtrlElm = doc.createElement('material')
			objElm.appendChild(mtrlElm)
			diffuse = doc.createElement('diffuse')
			diffuse.setAttribute('r', str(mtrl.rgbCol[0]))
			diffuse.setAttribute('g', str(mtrl.rgbCol[1]))
			diffuse.setAttribute('b', str(mtrl.rgbCol[2]))
			diffuse.setAttribute('a', str(mtrl.alpha))
			ambient = doc.createElement('ambient')
			ambient.setAttribute('r', str(mtrl.mirCol[0]))
			ambient.setAttribute('g', str(mtrl.mirCol[1]))
			ambient.setAttribute('b', str(mtrl.mirCol[2]))
			ambient.setAttribute('a', str(mtrl.alpha))
			specular = doc.createElement('specular')
			specular.setAttribute('r', str(mtrl.specCol[0]))
			specular.setAttribute('g', str(mtrl.specCol[1]))
			specular.setAttribute('b', str(mtrl.specCol[2]))
			specular.setAttribute('a', str(mtrl.alpha))
			emissive = doc.createElement('emissive')
			emissive.setAttribute('r', str(0))
			emissive.setAttribute('g', str(0))
			emissive.setAttribute('b', str(0))
			emissive.setAttribute('a', str(0))
			power = doc.createElement('power')
			power.setAttribute('value', str(1.0))
			if mtrl.getMode() & Material.Modes['SHADELESS']:
				mtrlElm.setAttribute('shadeless', '1')
			mtrlElm.appendChild(diffuse)
			mtrlElm.appendChild(ambient)
			mtrlElm.appendChild(specular)
			mtrlElm.appendChild(emissive)
			mtrlElm.appendChild(power)
					
			for tex in mtrl.getTextures():
				if tex != None and tex.tex != None and tex.tex.image != None:
					texElm = doc.createElement('texture')
					texElm.setAttribute('type', 'image')
					texElm.setAttribute('path', tex.tex.image.filename)
					if tex.mtAlpha:
						texElm.setAttribute('alpha', '1')
					if tex.mtCol:
						texElm.setAttribute('color', '1')
					if tex.mtNor:
						texElm.setAttribute('normal', '1')
					mtrlElm.appendChild(texElm)
			
			scene.appendChild(objElm)


def CreateLimitElm(doc, name, v0, v1):
	limitElm = doc.createElement('limit')
	limitElm.setAttribute('type', name)
	limitStr = '%f %f' % (v0, v1)
	limitElm.appendChild(doc.createTextNode(limitStr))
	return limitElm


def ProcessSceneObject(doc, ob, processedObjects, sceneElm):
	if ob in processedObjects.keys():
		return processedObjects[ob]
	if ob.parent is None:
		parentElm = sceneElm
	else:
		parentElm = ProcessSceneObject(doc, ob.parent, processedObjects, sceneElm)
	
	if ob.type == 'Mesh':
		rtclass = 'ArnMesh'
	elif ob.type == 'Camera':
		rtclass = 'ArnCamera'
	elif ob.type == 'Lamp':
		rtclass = 'ArnLight'
	elif ob.type == 'Armature':
		rtclass = 'ArnSkeleton'
	else:
		print 'Object \'', ob.name, '\' of type ', ob.type, ' is not supported type; skipping'
	
	obj = doc.createElement('object')
	obj.setAttribute('rtclass', rtclass)
	obj.setAttribute('name', ob.name)
	
	xform = createTransformElementFromMatrix(doc, ob.matrixLocal)
	obj.appendChild(xform)
	
	if ob.ipo:
		ipoElm = doc.createElement('ipo')
		ipoElm.setAttribute('name', ob.ipo.name)
		linkedIpos.append(ob.ipo.name)
		obj.appendChild(ipoElm)
	
	if ob.type == 'Mesh':
		meshData = createMeshData(doc, ob)
		obj.appendChild(meshData)
	elif ob.type == 'Camera':
		cameraData = createCameraData(doc, ob)
		obj.appendChild(cameraData)
	elif ob.type == 'Lamp':
		lightData = createLightData(doc, ob)
		obj.appendChild(lightData)
	elif ob.type == 'Armature':
		skeletonData = createSkeletonData(doc, ob)
		obj.appendChild(skeletonData)
	else:
		errorerror
		
	if ob.action:
		actElm = doc.createElement('action')
		actElm.setAttribute('name', ob.action.name)
		linkedActions.append(ob.action.name)
		obj.appendChild(actElm)
	for actionStrip in ob.actionStrips:
		actStripElm = doc.createElement('actionstrip')
		actStripElm.setAttribute('name', actionStrip.action.name)
		linkedActions.append(actionStrip.action.name)
		obj.appendChild(actStripElm)
	
	if ob.boundingBox:
		dimElm = doc.createElement('boundingbox')
		textData = ''
		boundbox = ob.getBoundBox(0)
		chunk = createChunk(doc, exportPlace, [('coordinates', 'float3')])
		for bb in boundbox:
			appendToChunk(chunk, [bb.x, bb.y, bb.z])
		finalizeChunk(chunk)
		dimElm.appendChild(chunk)
		obj.appendChild(dimElm)
	
	if ob.rbFlags & Object.RBFlags.ACTOR:
		actorElm = doc.createElement('actor')
		if ob.rbFlags & Object.RBFlags.DYNAMIC and ob.rbFlags & Object.RBFlags.RIGIDBODY:
			rbElm = doc.createElement('rigidbody')
			rbElm.setAttribute('mass', str(ob.rbMass))
			actorElm.appendChild(rbElm)
		
		if ob.rbFlags & Object.RBFlags.BOUNDS:
			bounds = 'others'
			if ob.rbShapeBoundType is Object.RBShapes.BOX:
				bounds = 'box'
			actorElm.setAttribute('bounds', bounds)
		obj.appendChild(actorElm)

	for c in ob.constraints:
		if c.type is Constraint.Type.RIGIDBODYJOINT:
			if c[Constraint.Settings.TARGET]:
				cElm = doc.createElement('constraint')
				cElm.setAttribute('type', 'rigidbodyjoint')
				targetElm = doc.createElement('target')
				targetElm.appendChild(doc.createTextNode(c[Constraint.Settings.TARGET].name))
				cElm.appendChild(targetElm)
				
				cType = c[Constraint.Settings.CONSTR_RB_TYPE]
				if cType is 1:
					cTypeStr = 'ball'
				elif cType is 2:
					cTypeStr = 'hinge'
				elif cType is 12:
					cTypeStr = 'generic'
				else:
					errorerror
				jointTypeElm = doc.createElement('type')
				jointTypeElm.appendChild(doc.createTextNode(cTypeStr))
				cElm.appendChild(jointTypeElm)

				pivotElm = doc.createElement('pivot')
				pivotElm.appendChild(doc.createTextNode('%f %f %f' % (c[Constraint.Settings.CONSTR_RB_PIVX], c[Constraint.Settings.CONSTR_RB_PIVY], c[Constraint.Settings.CONSTR_RB_PIVZ])))
				axElm = doc.createElement('ax')
				axElm.setAttribute('unit', 'rad')
				axElm.appendChild(doc.createTextNode('%f %f %f' % (c[Constraint.Settings.CONSTR_RB_AXX], c[Constraint.Settings.CONSTR_RB_AXY], c[Constraint.Settings.CONSTR_RB_AXZ])))
				cElm.appendChild(pivotElm)
				cElm.appendChild(axElm)

				limitFlags = c[Constraint.Settings.LIMIT]
				if limitFlags & 1:
					limitElm = CreateLimitElm(doc, 'LinX', c[Constraint.Settings.CONSTR_RB_MINLIMIT0], c[Constraint.Settings.CONSTR_RB_MAXLIMIT0])
					cElm.appendChild(limitElm)
				if limitFlags & 2:
					limitElm = CreateLimitElm(doc, 'LinY', c[Constraint.Settings.CONSTR_RB_MINLIMIT1], c[Constraint.Settings.CONSTR_RB_MAXLIMIT1])
					cElm.appendChild(limitElm)
				if limitFlags & 4:
					limitElm = CreateLimitElm(doc, 'LinZ', c[Constraint.Settings.CONSTR_RB_MINLIMIT2], c[Constraint.Settings.CONSTR_RB_MAXLIMIT2])
					cElm.appendChild(limitElm)
				if limitFlags & 8:
					limitElm = CreateLimitElm(doc, 'AngX', c[Constraint.Settings.CONSTR_RB_MINLIMIT3], c[Constraint.Settings.CONSTR_RB_MAXLIMIT3])
					cElm.appendChild(limitElm)
				if limitFlags & 16:
					limitElm = CreateLimitElm(doc, 'AngY', c[Constraint.Settings.CONSTR_RB_MINLIMIT4], c[Constraint.Settings.CONSTR_RB_MAXLIMIT4])
					cElm.appendChild(limitElm)
				if limitFlags & 32:
					limitElm = CreateLimitElm(doc, 'AngZ', c[Constraint.Settings.CONSTR_RB_MINLIMIT5], c[Constraint.Settings.CONSTR_RB_MAXLIMIT5])
					cElm.appendChild(limitElm)
				obj.appendChild(cElm)

	parentElm.appendChild(obj)
	assert(ob not in processedObjects.keys())
	processedObjects[ob] = obj

#
#
#
# Export Script Entry
#
#
#
global xmlFile
global binFile
global exportPlace
global linkedMaterials
global linkedIpos
global linkedActions

exportPlace = 'bin'

sce = bpy.data.scenes.active # Get active scene automatically
doc = Document()
homedir = os.path.expanduser('~')
print 'Export Directory: %s' % homedir
xmlFile = open('%s/%s.xml' % (homedir, sce.name), 'w')
binFile = open('%s/%s.bin' % (homedir, sce.name), 'wb')
linkedMaterials = []
linkedIpos      = []
linkedActions   = []

scene = doc.createElement('object')
scene.setAttribute('name', sce.name)
scene.setAttribute('rtclass', 'ArnSceneGraph')
scene.setAttribute('blender', Blender.Get('filename'))
doc.appendChild(scene)

processedObjects = {}

for ob in sce.objects:
	ProcessSceneObject(doc, ob, processedObjects, scene)
	
#print Constraint.Settings

# Export only actucally used materials, actions and ipos.
# (Actions should be exported first to resolve dependencies between
#  actions and ipos.)

linkedMaterialsSet = set(linkedMaterials)
exportMaterials(doc, linkedMaterialsSet)

linkedActionsSet = set(linkedActions)
exportActions(doc, linkedActionsSet)

linkedIposSet = set(linkedIpos)
exportIpos(doc, linkedIposSet)


#doc.getElementById('BlueMtrl').setAttribute('xxxxxx', 'xxx')

# Write output file and exit
scene.setAttribute('binuncompressedsize', str(binFile.tell()))
xmlFile.write(doc.toprettyxml())
xmlFile.close()
binFile.close() # Raw binary data file writing finished

# Compress binary file
binFile = open('%s/%s.bin' % (homedir, sce.name), 'rb')
compressedData = zlib.compress(binFile.read())
binFile.close()
binFile = open('%s/%s.bin' % (homedir, sce.name), 'wb')
binFile.write(compressedData)
binFile.close()

doc.unlink()
