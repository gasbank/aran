#include "AranPCH.h"
#include "ArnXmlLoader.h"
#include "ArnXmlString.h"
#include "ArnSceneGraph.h"
#include "ArnMesh.h"
#include "ArnCamera.h"
#include "ArnLight.h"
#include "ArnMaterial.h"
#include "ArnSkeleton.h"
#include "ArnBone.h"
#include "ArnIpo.h"
#include "ArnAction.h"
#include "ArnBinaryChunk.h"
#include "ArnMath.h"
#include "ArnTexture.h"

ArnXmlLoader::ArnXmlLoader()
{
	//ctor
}

ArnXmlLoader::~ArnXmlLoader()
{
	//dtor
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline DOMElement*
GetUniqueChildElement(const DOMElement* elm, const XMLCh* tagName)
{
	DOMNodeList* children = elm->getElementsByTagName(tagName);
	const XMLSize_t childrenCount = children->getLength();
	int directChildrenCount = 0;
	DOMNode* uniqueChildren = 0;
	for (XMLSize_t xx = 0; xx < childrenCount; ++xx)
	{
		DOMNode::NodeType nt = (DOMNode::NodeType)children->item(xx)->getNodeType();
		if (nt == DOMNode::ELEMENT_NODE && children->item(xx)->getParentNode() == elm)
		{
			++directChildrenCount;
			uniqueChildren = children->item(xx);
		}
	}
	if (directChildrenCount == 1)
		return dynamic_cast<DOMElement*>(uniqueChildren);
	else if (directChildrenCount == 0)
		return 0;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ParseRgbaFromElement(float* r, float* g, float* b, float* a, const DOMElement* elm)
{
	*r = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_r)).c_str()) );
	*g = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_g)).c_str()) );
	*b = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_b)).c_str()) );
	*a = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_a)).c_str()) );
}

void
ParseRgbFromElement(float* r, float* g, float* b, const DOMElement* elm)
{
	*r = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_r)).c_str()) );
	*g = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_g)).c_str()) );
	*b = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_b)).c_str()) );
}

void
ParseArnVec3FromElement(ArnVec3* v, const DOMElement* elm)
{
	v->x = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_x)).c_str()) );
	v->y = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_y)).c_str()) );
	v->z = float( atof(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_z)).c_str()) );
}

void
ParseTransformFromElement(ArnMatrix* mat, ArnVec3* scale, ArnQuat* rQuat, ArnVec3* trans, const DOMElement* elm)
{
	assert(XMLString::equals(elm->getTagName(), GetArnXmlString().TAG_transform));
	if (XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_type), GetArnXmlString().VAL_srt))
	{
		DOMElement* scaling = GetUniqueChildElement(elm, GetArnXmlString().TAG_scaling);
		DOMElement* rotation = GetUniqueChildElement(elm, GetArnXmlString().TAG_rotation);
		DOMElement* translation = GetUniqueChildElement(elm, GetArnXmlString().TAG_translation);

		ArnVec3 s = CreateArnVec3(1,1,1);
		ArnVec3 r = CreateArnVec3(0,0,0);
		ArnVec3 t = CreateArnVec3(0,0,0);
		if (scaling)
		{
			ParseArnVec3FromElement(&s, scaling);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		if (rotation)
		{
			assert(XMLString::equals(rotation->getAttribute(GetArnXmlString().ATTR_type), GetArnXmlString().VAL_euler));
			assert(XMLString::equals(rotation->getAttribute(GetArnXmlString().ATTR_unit), GetArnXmlString().VAL_deg));
			ParseArnVec3FromElement(&r, rotation);
			if (XMLString::equals(rotation->getAttribute(GetArnXmlString().ATTR_unit), GetArnXmlString().VAL_deg))
			{
				r.x = ArnToRadian(r.x);
				r.y = ArnToRadian(r.y);
				r.z = ArnToRadian(r.z);
			}
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		if (translation)
		{
			ParseArnVec3FromElement(&t, translation);
		}

		ArnQuat rQ;
		rQ = ArnEulerToQuat(&r);
		assert(rQuat && scale && trans);
		*rQuat = rQ;
		*scale = s;
		*trans = t;
		if (mat) // Matrix output is optional.
			ArnMatrixTransformation(mat, 0, 0, &s, 0, &rQ, &t);
	}
	else
	{
		assert(!"Unsupported type.");
	}
}

ArnSceneGraph*
ArnSceneGraph::createFrom(const char* xmlFile)
{
	XercesDOMParser* parser = new XercesDOMParser();
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true);    // optional

	ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
	parser->setErrorHandler(errHandler);

	try
	{
		parser->parse(xmlFile);
	}
	catch (const XMLException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n"
			<< message << "\n";
		XMLString::release(&message);
		return 0;
	}
	catch (const DOMException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n"
			<< message << "\n";
		XMLString::release(&message);
		return 0;
	}
	catch (...)
	{
		std::cout << "Unexpected Exception: XML file opening error? - " << xmlFile << std::endl;
		return 0;
	}

	xercesc::DOMDocument* xmlDoc = parser->getDocument();
	DOMElement* elm = xmlDoc->getDocumentElement();
	char* sceneNameStr = XMLString::transcode(elm->getAttribute(GetArnXmlString().ATTR_name));
	std::cout << " - Root scene name: " << sceneNameStr << std::endl;

	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnSceneGraph));
	ArnSceneGraph* ret = ArnSceneGraph::createFromEmptySceneGraph();
	const XMLCh* name = elm->getAttribute(GetArnXmlString().ATTR_name);
	std::string sceneName = ScopedString(name).c_str();
	ret->setName(sceneName.c_str());
	std::string binaryFileName(xmlFile);
	binaryFileName[binaryFileName.size() - 3] = 'b';
	binaryFileName[binaryFileName.size() - 2] = 'i';
	binaryFileName[binaryFileName.size() - 1] = 'n';

	ret->m_binaryChunk = ArnBinaryChunk::createFrom(binaryFileName.c_str());

	DOMNodeList* childrenObj = elm->getElementsByTagName(GetArnXmlString().TAG_object);
	const XMLSize_t childrenCount = childrenObj->getLength();
	for (XMLSize_t xx = 0; xx < childrenCount; ++xx)
	{
		DOMNode* child = childrenObj->item(xx);
		DOMElement* childElm = dynamic_cast<DOMElement*>(child);
		assert(childElm);

		ArnNode* childObj = 0;
		if (ret->m_binaryChunk)
			childObj = CreateArnNodeFromXmlElement(childElm, ret->m_binaryChunk->getRawDataPtr());
		else
			childObj = CreateArnNodeFromXmlElement(childElm, 0);
		ret->attachChild(childObj);
	}

	delete parser;
	delete errHandler;
	return ret;
}

ArnMesh*
ArnMesh::createFrom(const DOMElement* elm, char* binaryChunkBasePtr)
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnMesh));

	ArnMesh* ret = new ArnMesh();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	ArnVec3 trans, scale;
	ArnQuat rot;
	ParseTransformFromElement(0, &scale, &rot, &trans, GetUniqueChildElement(elm, GetArnXmlString().TAG_transform));
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMElement* ipoElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_ipo);
	if (ipoElm)
		ret->setIpoName(ScopedString(ipoElm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	else
		ret->setIpoName("");

	DOMNodeList* vertexNodes = elm->getElementsByTagName(GetArnXmlString().TAG_vertex);
	DOMNodeList* faceNodes = elm->getElementsByTagName(GetArnXmlString().TAG_face);
	assert(vertexNodes && faceNodes);
	DOMElement* vertexElm  = dynamic_cast<DOMElement*>(vertexNodes->item(0));
	DOMElement* faceElm = dynamic_cast<DOMElement*>(faceNodes->item(0));
	assert(vertexElm && faceElm);

	ArnMesh::VertexGroup vg;
	DOMElement* vertexChunkElm = dynamic_cast<DOMElement*>(vertexElm->getElementsByTagName(GetArnXmlString().TAG_chunk)->item(0));
	vg.mtrlIndex = 0;
	vg.vertexChunk = ArnBinaryChunk::createFrom(vertexChunkElm, binaryChunkBasePtr);
	ret->m_vertexGroup.push_back(vg);

	DOMNodeList* faceGroup = faceElm->getElementsByTagName(GetArnXmlString().TAG_facegroup);
	const XMLSize_t faceGroupCount = faceGroup->getLength();
	for (XMLSize_t xx = 0; xx < faceGroupCount; ++xx)
	{
		DOMElement* faceGroupElm = dynamic_cast<DOMElement*>(faceGroup->item(xx));
		ScopedString ssMtrl(faceGroupElm->getAttribute(GetArnXmlString().ATTR_mtrl));
		ArnMesh::FaceGroup fg;
		fg.mtrlIndex = atoi(ssMtrl.c_str());
		fg.triFaceChunk = 0;
		fg.quadFaceChunk = 0;
		assert(fg.mtrlIndex >= 0);

		DOMNodeList* chunk = faceGroupElm->getElementsByTagName(GetArnXmlString().TAG_chunk);
		const XMLSize_t chunkCount = chunk->getLength();
		assert(chunkCount == 2);

		DOMElement* chunkElm = dynamic_cast<DOMElement*>(chunk->item(0));
		fg.triFaceChunk = ArnBinaryChunk::createFrom(chunkElm, binaryChunkBasePtr);
		chunkElm = dynamic_cast<DOMElement*>(chunk->item(1));
		fg.quadFaceChunk = ArnBinaryChunk::createFrom(chunkElm, binaryChunkBasePtr);
		assert(fg.triFaceChunk && fg.quadFaceChunk);

		ret->m_faceGroup.push_back(fg);
	}

	DOMNodeList* mtrlRefs = elm->getElementsByTagName(GetArnXmlString().TAG_material);
	const XMLSize_t mtrlRefsCount = mtrlRefs->getLength();
	for (XMLSize_t xx = 0; xx < mtrlRefsCount; ++xx)
	{
		DOMElement* mtrlElm = dynamic_cast<DOMElement*>( mtrlRefs->item(xx) );
		ScopedString ss(mtrlElm->getAttribute(GetArnXmlString().ATTR_name));
		ret->m_mtrlRefNameList.push_back(ss.c_str());
	}

	DOMNodeList* uvElmList = elm->getElementsByTagName(GetArnXmlString().TAG_uv);
	if (uvElmList->getLength())
	{
		DOMElement* uvElm = static_cast<DOMElement*>(elm->getElementsByTagName(GetArnXmlString().TAG_uv)->item(0));
		DOMElement* triquadUvElm = dynamic_cast<DOMElement*>(uvElm->getElementsByTagName(GetArnXmlString().TAG_chunk)->item(0));
		assert(triquadUvElm);
		ret->m_triquadUvChunk = ArnBinaryChunk::createFrom(triquadUvElm, binaryChunkBasePtr);
	}
	else
	{
		ret->m_triquadUvChunk = 0;
	}
	ret->m_renderFunc = &ArnMesh::renderXml;
	ret->m_initRendererObjectFunc = &ArnMesh::initRendererObjectXml;
	return ret;
}

ArnMaterial*
ArnMaterial::createFrom(const DOMElement* elm)
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnMaterial));
	ArnMaterial* ret = new ArnMaterial();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	DOMElement* materialElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_material);
	DOMElement* diffuseElm = GetUniqueChildElement(materialElm, GetArnXmlString().TAG_diffuse);
	DOMElement* ambientElm = GetUniqueChildElement(materialElm, GetArnXmlString().TAG_ambient);
	DOMElement* specularElm = GetUniqueChildElement(materialElm, GetArnXmlString().TAG_specular);
	DOMElement* emissiveElm = GetUniqueChildElement(materialElm, GetArnXmlString().TAG_emissive);
	DOMElement* powerElm = GetUniqueChildElement(materialElm, GetArnXmlString().TAG_power);
	assert(diffuseElm && ambientElm && specularElm && emissiveElm && powerElm);

	int shadeless = atoi(ScopedString(materialElm->getAttribute(GetArnXmlString().ATTR_shadeless)).c_str());
	if (shadeless)
		ret->m_bShadeless = true;

	float r, g, b, a;
	// TODO: Light colors...
	ret->m_data.m_materialName = ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str();
	ParseRgbaFromElement(&r, &g, &b, &a, diffuseElm);
	ret->m_data.m_d3dMaterial.Diffuse = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, ambientElm);
	ret->m_data.m_d3dMaterial.Ambient = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, specularElm);
	ret->m_data.m_d3dMaterial.Specular = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, emissiveElm);
	ret->m_data.m_d3dMaterial.Emissive = ArnColorValue4f(r, g, b, 1);
	ret->m_data.m_d3dMaterial.Power = float( atof(ScopedString(powerElm->getAttribute(GetArnXmlString().ATTR_value)).c_str()) );

	DOMNodeList* textureList = materialElm->getElementsByTagName(GetArnXmlString().TAG_texture);
	const XMLSize_t textureCount = textureList->getLength();
	for (XMLSize_t xx = 0; xx < textureCount; ++xx)
	{
		DOMElement* textureElm = static_cast<DOMElement*>(textureList->item(xx));
		if (XMLString::equals(textureElm->getAttribute(GetArnXmlString().ATTR_type), GetArnXmlString().VAL_image))
		{
			ScopedString texImageFileName(textureElm->getAttribute(GetArnXmlString().ATTR_path));
			ArnTexture* tex = ArnTexture::createFrom(texImageFileName.c_str());
			ret->attachTexture(tex);
		}
		else
		{
			ARN_THROW_NOT_IMPLEMENTED_ERROR
		}
	}
	return ret;
}

ArnCamera*
ArnCamera::createFrom( DOMElement* elm )
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnCamera));
	ArnCamera* ret = new ArnCamera();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	ArnVec3 trans, scale;
	ArnQuat rot;
	ParseTransformFromElement(0, &scale, &rot, &trans, GetUniqueChildElement(elm, GetArnXmlString().TAG_transform));
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMElement* ipoElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_ipo);
	if (ipoElm)
		ret->setIpoName(ScopedString(ipoElm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	else
		ret->setIpoName("");

	DOMElement* cameraElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_camera);
	float farClip = float( atof(ScopedString(cameraElm->getAttribute(GetArnXmlString().ATTR_farclip)).c_str()) );
	float nearClip = float( atof(ScopedString(cameraElm->getAttribute(GetArnXmlString().ATTR_nearclip)).c_str()) );
	float fovdeg = float( atof(ScopedString(cameraElm->getAttribute(GetArnXmlString().ATTR_fovdeg)).c_str()) );
	ret->setFarClip(farClip);
	ret->setNearClip(nearClip);
	ret->setFov(ArnToRadian(fovdeg));
	const XMLCh* typeCh = cameraElm->getAttribute(GetArnXmlString().ATTR_type);
	if (XMLString::equals(typeCh, GetArnXmlString().VAL_persp))
	{
	}
	else if (XMLString::equals(typeCh, GetArnXmlString().VAL_ortho))
	{
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return ret;
}

ArnLight*
ArnLight::createFrom( const DOMElement* elm )
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnLight));
	ArnLight* ret = new ArnLight();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	ArnVec3 trans, scale;
	ArnQuat rot;
	ParseTransformFromElement(0, &scale, &rot, &trans, GetUniqueChildElement(elm, GetArnXmlString().TAG_transform));
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMElement* lightElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_light);
	float r, g, b;
	ParseRgbFromElement(&r, &g, &b, lightElm);
	ret->m_d3dLight.Ambient = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Diffuse = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Specular = ArnColorValue4f(r, g, b, 1);
	const XMLCh* lightType = lightElm->getAttribute(GetArnXmlString().ATTR_type);
	if (XMLString::equals(lightType, GetArnXmlString().VAL_point))
	{
		// Point light
		ret->m_d3dLight.Type = 1;
		ret->m_d3dLight.Position = trans;
	}
	else if (XMLString::equals(lightType, GetArnXmlString().VAL_spot))
	{
		// Spot light
		ret->m_d3dLight.Type = 2;
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	else if (XMLString::equals(lightType, GetArnXmlString().VAL_directional))
	{
		// Directional light
		ret->m_d3dLight.Type = 3;
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return ret;
}

ArnSkeleton*
ArnSkeleton::createFrom( const DOMElement* elm )
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnSkeleton));
	ArnSkeleton* ret = new ArnSkeleton();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	ArnVec3 trans, scale;
	ArnQuat rot;
	ParseTransformFromElement(0, &scale, &rot, &trans, GetUniqueChildElement(elm, GetArnXmlString().TAG_transform));
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMNodeList* actionstrips = elm->getElementsByTagName(GetArnXmlString().TAG_actionstrip);
	const XMLSize_t actionstripCount = actionstrips->getLength();
	for (XMLSize_t xx = 0; xx < actionstripCount; ++xx)
	{
		DOMElement* actStripElm = reinterpret_cast<DOMElement*>(actionstrips->item(xx));
		ret->m_actionStripNames.push_back(ScopedString(actStripElm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	}
	DOMElement* actionElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_action);
	if (actionElm)
		ret->setDefaultActionName(ScopedString(actionElm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	else
		ret->setDefaultActionName("");


	DOMElement* skelElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_skeleton);
	DOMNodeList* boneList = skelElm->getElementsByTagName(GetArnXmlString().TAG_object);
	const XMLSize_t rootBoneCount = boneList->getLength();
	for (XMLSize_t xx = 0; xx < rootBoneCount; ++xx)
	{
		DOMElement* boneElm = dynamic_cast<DOMElement*>( boneList->item(xx) );
		if (boneElm->getParentNode() == skelElm)
		{
			ArnNode* bone = CreateArnNodeFromXmlElement(boneElm, 0);
			ret->attachChild(bone);
		}
	}

	return ret;
}

ArnBone*
ArnBone::createFrom( const DOMElement* elm )
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnBone));
	ArnBone* ret = new ArnBone();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());

	ArnVec3 trans, scale;
	ArnQuat rot;
	ParseTransformFromElement(0, &scale, &rot, &trans, GetUniqueChildElement(elm, GetArnXmlString().TAG_transform));
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMElement* boneElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_bone);
	DOMElement* headElm = GetUniqueChildElement(boneElm, GetArnXmlString().TAG_head);
	DOMElement* tailElm = GetUniqueChildElement(boneElm, GetArnXmlString().TAG_tail);
	DOMElement* rollElm = GetUniqueChildElement(boneElm, GetArnXmlString().TAG_roll);

	ArnVec3 headPos, tailPos;
	ParseArnVec3FromElement(&headPos, headElm);
	ParseArnVec3FromElement(&tailPos, tailElm);
	float roll = float( atof(ScopedString(rollElm->getAttribute(GetArnXmlString().ATTR_value)).c_str()) );
	ret->setHeadPos(headPos);
	ret->setTailPos(tailPos);
	ret->setRoll(ArnToRadian(roll));

	DOMNodeList* boneList = elm->getElementsByTagName(GetArnXmlString().TAG_object);
	const XMLSize_t boneCount = boneList->getLength();
	for (XMLSize_t xx = 0; xx < boneCount; ++xx)
	{
		DOMElement* boneElm = dynamic_cast<DOMElement*>( boneList->item(xx) );
		if (boneElm->getParentNode() == elm)
		{
			ArnNode* bone = CreateArnNodeFromXmlElement(boneElm, 0);
			ret->attachChild(bone);
		}
	}
	return ret;
}

ArnIpo*
ArnIpo::createFrom(const DOMElement* elm, char* binaryChunkBasePtr)
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnIpo));
	ArnIpo* ret = new ArnIpo();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	DOMElement* ipoElm = dynamic_cast<DOMElement*>(elm->getElementsByTagName(GetArnXmlString().TAG_ipo)->item(0));
	DOMNodeList* curveList = ipoElm->getElementsByTagName(GetArnXmlString().TAG_curve);
	const XMLSize_t curveListSize = curveList->getLength();
	for (XMLSize_t xx = 0; xx < curveListSize; ++xx)
	{
		DOMElement* curveElm = dynamic_cast<DOMElement*>( curveList->item(xx) );
		ScopedString curveTypeStr(curveElm->getAttribute(GetArnXmlString().ATTR_type));
		ScopedString curveNameStr(curveElm->getAttribute(GetArnXmlString().ATTR_name));
		DOMElement* controlPointElm = GetUniqueChildElement(curveElm, GetArnXmlString().TAG_controlpoint);

		ret->m_curves.push_back(CurveData());
		CurveData& cd = ret->m_curves.back();
		cd.nameStr = curveNameStr.c_str();
		cd.name = ArnIpo::CurveNameStrToEnum(cd.nameStr.c_str());
		if (strcmp(curveTypeStr.c_str(), "const") == 0)
			cd.type = IPO_CONST;
		else if (strcmp(curveTypeStr.c_str(), "linear") == 0)
			cd.type = IPO_LIN;
		else if (strcmp(curveTypeStr.c_str(), "bezier") == 0)
			cd.type = IPO_BEZ;
		else
			ARN_THROW_UNEXPECTED_CASE_ERROR

		DOMElement* cpChunk = GetUniqueChildElement(controlPointElm, GetArnXmlString().TAG_chunk);
		ArnBinaryChunk* controlPointChunk = ArnBinaryChunk::createFrom(cpChunk, binaryChunkBasePtr);
		assert(controlPointChunk->getRecordSize() == sizeof(BezTripleData));
		cd.pointCount = controlPointChunk->getRecordCount();
		cd.points.resize(cd.pointCount);
		memcpy(&cd.points[0], controlPointChunk->getRawDataPtr(), sizeof(BezTripleData) * cd.pointCount);

		foreach (const BezTripleData& btd, cd.points)
		{
			if ( ret->getEndKeyframe() < (int)btd.vec[1][0] )
				ret->setEndKeyframe((int)btd.vec[1][0]);
		}
		delete controlPointChunk;
	}
	ret->m_curveCount = curveListSize;
	ret->m_ipoCount = 1; // TODO: Is semantically correct one?
	return ret;
}

ArnAction*
ArnAction::createFrom(const DOMElement* elm)
{
	assert(XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_rtclass), GetArnXmlString().VAL_ArnAction));
	ArnAction* ret = new ArnAction();
	ret->setName(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_name)).c_str());
	DOMElement* actionElm = GetUniqueChildElement(elm, GetArnXmlString().TAG_action);
	DOMNodeList* list = actionElm->getElementsByTagName(GetArnXmlString().TAG_objectipomap);
	const XMLSize_t listCount = list->getLength();
	for (XMLSize_t xx = 0; xx < listCount; ++xx)
	{
		DOMElement* mapElm = dynamic_cast<DOMElement*>(list->item(xx));
		ScopedString ssObjName(mapElm->getAttribute(GetArnXmlString().ATTR_obj));
		ScopedString ssIpoName(mapElm->getAttribute(GetArnXmlString().ATTR_ipo));
		ret->m_objectIpoNameMap[ssObjName.c_str()] = ssIpoName.c_str();
	}
	return ret;
}

ArnNode*
CreateArnNodeFromXmlElement(DOMElement* elm, char* binaryChunkBasePtr)
{
	const XMLCh* rtclass = elm->getAttribute(GetArnXmlString().ATTR_rtclass);
	ArnNode* ret = 0;
	if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnMesh))
	{
		ret = ArnMesh::createFrom(elm, binaryChunkBasePtr);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnCamera))
	{
		ret = ArnCamera::createFrom(elm);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnMaterial))
	{
		ret = ArnMaterial::createFrom(elm);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnSkeleton))
	{
		ret = ArnSkeleton::createFrom(elm);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnIpo))
	{
		ret = ArnIpo::createFrom(elm, binaryChunkBasePtr);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnAction))
	{
		ret = ArnAction::createFrom(elm);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnLight))
	{
		ret = ArnLight::createFrom(elm);
	}
	else if (XMLString::equals(rtclass, GetArnXmlString().VAL_ArnBone))
	{
		ret = ArnBone::createFrom(elm);
	}
	else
	{
		assert(!"Unknown runtime class identifier (rtclass) attribute!");
	}
	assert(ret);
	return ret;
}

//////////////////////////////////////////////////////////////////////////

int InitializeXmlParser()
{
	try
	{
		XMLPlatformUtils::Initialize();
		return 0;
	}
	catch (const XMLException& e)
	{
		char* message = XMLString::transcode(e.getMessage());
		std::cout << "Error during initialization! :\n";
		std::cout << message << "\n";
		XMLString::release(&message);
		return -1;
	}
}

void DeallocateXmlParser()
{
	XMLPlatformUtils::Terminate();
}
