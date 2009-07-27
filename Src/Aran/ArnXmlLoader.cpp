#include "AranPCH.h"
#include "ArnXmlLoader.h"
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static void
GetAttr(std::string& val, const DOMElement* elm, const char* attrName)
{
	XMLCh* a = XMLString::transcode(attrName);
	const XMLCh* v = elm->getAttribute(a);
	XMLString::release(&a);
	if (v)
	{
		char* value = XMLString::transcode(v);
		val = value;
		XMLString::release(&value);
	}
	else
	{
		val = "";
	}
}

static float
ParseFloatFromAttr(const DOMElement* elm, const char* attrName)
{
	std::string val;
	GetAttr(val, elm, attrName);
	return atof(val.c_str());
}

static int
ParseIntFromAttr(const DOMElement* elm, const char* attrName)
{
	std::string val;
	GetAttr(val, elm, attrName);
	return atoi(val.c_str());
}

static void
ParseRgbFromElement(float* r, float* g, float* b, const DOMElement* elm)
{
	*r = ParseFloatFromAttr(elm, "r");
	*g = ParseFloatFromAttr(elm, "g");
	*b = ParseFloatFromAttr(elm, "b");
}

static void
ParseRgbaFromElement(float* r, float* g, float* b, float* a, const DOMElement* elm)
{
	ParseRgbFromElement(r, g, b, elm);
	*a = ParseFloatFromAttr(elm, "a");
}

static void
ParseArnVec3FromElement(ArnVec3* v, const DOMElement* elm)
{
	v->x = ParseFloatFromAttr(elm, "x");
	v->y = ParseFloatFromAttr(elm, "y");
	v->z = ParseFloatFromAttr(elm, "z");
}

static void
AssertTagNameEquals(const DOMElement* elm, const char* tagName)
{
	XMLCh* x = XMLString::transcode(tagName);
	if (!XMLString::equals(elm->getTagName(), x))
	{
		XMLString::release(&x);
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	XMLString::release(&x);
}

static bool
AttrEquals(const DOMElement* elm, const char* attrName, const char* attrVal)
{
	std::string val;
	GetAttr(val, elm, attrName);
	return strcmp(val.c_str(), attrVal) ? false : true;
}

static void
AssertAttrEquals(const DOMElement* elm, const char* attrName, const char* attrVal)
{
	if (!AttrEquals(elm, attrName, attrVal))
		ARN_THROW_UNEXPECTED_CASE_ERROR
}


static DOMNodeList*
GetElementsByTagName(const DOMElement* elm, const char* tagName)
{
	XMLCh* x = XMLString::transcode(tagName);
	DOMNodeList* ret = elm->getElementsByTagName(x);
	XMLString::release(&x);
	return ret;
}

static DOMElement*
GetUniqueChildElement(const DOMElement* elm, const char* tagName)
{
	DOMNodeList* children = GetElementsByTagName(elm, tagName);
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

static void
ParseTransformFromElement(ArnMatrix* mat, ArnVec3* scale, ArnQuat* rQuat, ArnVec3* trans,
                          const DOMElement* elm)
{
	AssertTagNameEquals(elm, "transform");
	if (AttrEquals(elm, "type", "srt"))
	{
		DOMElement* scaling = GetUniqueChildElement(elm, "scaling");
		DOMElement* rotation = GetUniqueChildElement(elm, "rotation");
		DOMElement* translation = GetUniqueChildElement(elm, "translation");

		ArnVec3 s(1,1,1);
		ArnVec3 r(0,0,0);
		ArnVec3 t(0,0,0);
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
			AssertAttrEquals(rotation, "type", "euler");
			AssertAttrEquals(rotation, "unit", "deg");
			ParseArnVec3FromElement(&r, rotation);
			r.x = ArnToRadian(r.x);
			r.y = ArnToRadian(r.y);
			r.z = ArnToRadian(r.z);
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


////////////////////////////////////////////////////////////////////////////////////////////////////

static void
SetupArnNodeCommonPart(ArnNode* ret, const DOMElement* elm)
{
	std::string name;
	GetAttr(name, elm, "name");
	ret->setName(name.c_str());
}

static void
SetupArnXformableCommonPart(ArnXformable* ret, const DOMElement* elm)
{
	// Setup ArnXformable common part.
	// This includes ArnNode part and
	// 'transformation', and 'ipo'.
	SetupArnNodeCommonPart(ret, elm);

	ArnVec3 trans, scale;
	ArnQuat rot;
	DOMElement* transformElm = GetUniqueChildElement(elm, "transform");
	ParseTransformFromElement(0, &scale, &rot, &trans, transformElm);
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	DOMElement* ipoElm = GetUniqueChildElement(elm, "ipo");
	if (ipoElm)
	{
		std::string ipoName;
		GetAttr(ipoName, ipoElm, "name");
		ret->setIpoName(ipoName.c_str());
	}
	else
	{
		ret->setIpoName("");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

template <typename T> void
parseFromToken(char* target, int& dataOffset, Tokenizer::const_iterator& it,
               const int doCount, T func(const char*))
{
	assert(doCount >= 1);
	for (int i = 0; i < doCount; ++i)
	{
		*(T*)(target + i*sizeof(T)) = func((*it).c_str());
		++it;
		dataOffset += sizeof(T);
	}
}

inline float
atof2(const char* c)
{
	return (float)atof(c);
}

ArnBinaryChunk*
ArnBinaryChunk::createFrom(DOMElement* elm, char* binaryChunkBasePtr)
{
	ArnBinaryChunk* ret = new ArnBinaryChunk();
	DOMElement* templ = dynamic_cast<DOMElement*>(GetElementsByTagName(elm, "template")->item(0));
	assert(templ);
	DOMNodeList* templChildren = GetElementsByTagName(templ, "field");
	const XMLSize_t childCount = templChildren->getLength();
	assert(childCount);
	for (XMLSize_t xx = 0; xx < childCount; ++xx)
	{
		DOMElement* childElm = dynamic_cast<DOMElement*>(templChildren->item(xx));
		std::string typeStr, usageStr;
		GetAttr(typeStr, childElm, "type");
		GetAttr(usageStr, childElm, "usage");
		ret->addField( typeStr.c_str(), usageStr.c_str() );
	}

	std::string placeStr;
	GetAttr(placeStr, elm, "place");
	if (strcmp(placeStr.c_str(), "xml") == 0)
	{
		DOMElement* arraydata = dynamic_cast<DOMElement*>(GetElementsByTagName(elm, "arraydata")->item(0));
		DOMNodeList* arraydataChildren = GetElementsByTagName(arraydata, "data");
		ret->m_recordCount = arraydataChildren->getLength();
		assert(ret->m_recordCount >= 0); // There can be no record in the chunk.
		ret->m_data = new char[ ret->m_recordCount * ret->m_recordSize ];
		ret->m_deallocateData = true;
		int dataOffset = 0;
		for (XMLSize_t xx = 0; xx < (XMLSize_t)ret->m_recordCount; ++xx)
		{
			DOMElement* data = dynamic_cast<DOMElement*>(arraydataChildren->item(xx));
			std::string attrStr;
			GetAttr(attrStr, data, "value");
			//std::cout << "Original string: " << attrStr.c_str() << std::endl;
		    boost::char_separator<char> sep(";");
			Tokenizer tok(attrStr, sep);
			Tokenizer::const_iterator it = tok.begin();
			/*
			for (; it != tok.end(); ++it)
			{
				std::cout << "<" << *it << ">" << std::endl;
			}
			*/

			foreach(const Field& field, ret->m_recordDef)
			{
				switch (field.type)
				{
				case ACFT_FLOAT:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 1, atof2);
					break;
				case ACFT_FLOAT2:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 2, atof2);
					break;
				case ACFT_FLOAT3:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 3, atof2);
					break;
				case ACFT_FLOAT8:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 8, atof2);
					break;
				case ACFT_INT:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 1, atoi);
					break;
				case ACFT_INT3:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 3, atoi);
					break;
				case ACFT_INT4:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 4, atoi);
					break;
				default:
					assert(!"Should not reach here!");
					break;
				}
			}
			assert(dataOffset % ret->m_recordSize == 0);
		}
		assert(dataOffset == ret->m_recordCount * ret->m_recordSize);
	}
	else if (strcmp(placeStr.c_str(), "bin") == 0)
	{
		const int startOffset = ParseIntFromAttr(elm, "startoffset");
		const int endOffset = ParseIntFromAttr(elm, "endoffset");
		const int dataSize = endOffset - startOffset;
		const int recordCount = dataSize / ret->m_recordSize;
		assert(dataSize % ret->m_recordSize == 0);
		if (recordCount)
			ret->m_data = binaryChunkBasePtr + startOffset;
		else
			ret->m_data = 0;
		ret->m_deallocateData = false;
		ret->m_recordCount = recordCount;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}

	return ret;
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
		std::cout << "Exception: XML file opening error? - " << xmlFile << std::endl;
		return 0;
	}

	xercesc::DOMDocument* xmlDoc = parser->getDocument();
	DOMElement* elm = xmlDoc->getDocumentElement();
	std::string sceneNameStr;
	GetAttr(sceneNameStr, elm, "name");
	std::cout << " - Root scene name: " << sceneNameStr.c_str() << std::endl;

	AssertAttrEquals(elm, "rtclass", "ArnSceneGraph");
	ArnSceneGraph* ret = ArnSceneGraph::createFromEmptySceneGraph();
	SetupArnNodeCommonPart(ret, elm);

	std::string binaryFileName(xmlFile);
	binaryFileName[binaryFileName.size() - 3] = 'b';
	binaryFileName[binaryFileName.size() - 2] = 'i';
	binaryFileName[binaryFileName.size() - 1] = 'n';

	ret->m_binaryChunk = ArnBinaryChunk::createFrom(binaryFileName.c_str());

	DOMNodeList* childrenObj = GetElementsByTagName(elm, "object");
	const XMLSize_t childrenCount = childrenObj->getLength();
	for (XMLSize_t xx = 0; xx < childrenCount; ++xx)
	{
		DOMNode* child = childrenObj->item(xx);
		DOMElement* childElm = dynamic_cast<DOMElement*>(child);
		assert(childElm);

		ArnNode* childObj = 0;
		if (ret->m_binaryChunk)
		{
			char* binDataPtr = ret->m_binaryChunk->getRawDataPtr();
			childObj = CreateArnNodeFromXmlElement(childElm, binDataPtr);
		}

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
	AssertAttrEquals(elm, "rtclass", "ArnMesh");
	ArnMesh* ret = new ArnMesh();
	SetupArnXformableCommonPart(ret, elm);

	DOMNodeList* vertexNodes = GetElementsByTagName(elm, "vertex");
	DOMNodeList* faceNodes = GetElementsByTagName(elm, "face");
	assert(vertexNodes && faceNodes);
	DOMElement* vertexElm  = dynamic_cast<DOMElement*>(vertexNodes->item(0));
	DOMElement* faceElm = dynamic_cast<DOMElement*>(faceNodes->item(0));
	assert(vertexElm && faceElm);

	ArnMesh::VertexGroup vg;
	DOMElement* vertexChunkElm = dynamic_cast<DOMElement*>(GetElementsByTagName(vertexElm, "chunk")->item(0));
	vg.mtrlIndex = 0;
	vg.vertexChunk = ArnBinaryChunk::createFrom(vertexChunkElm, binaryChunkBasePtr);
	ret->m_vertexGroup.push_back(vg);

	DOMNodeList* faceGroup = GetElementsByTagName(faceElm, "facegroup");
	const XMLSize_t faceGroupCount = faceGroup->getLength();
	for (XMLSize_t xx = 0; xx < faceGroupCount; ++xx)
	{
		DOMElement* faceGroupElm = dynamic_cast<DOMElement*>(faceGroup->item(xx));
		std::string ssMtrl;
		GetAttr(ssMtrl, faceGroupElm, "mtrl");
		ArnMesh::FaceGroup fg;
		fg.mtrlIndex = atoi(ssMtrl.c_str());
		fg.triFaceChunk = 0;
		fg.quadFaceChunk = 0;
		assert(fg.mtrlIndex >= 0);

		DOMNodeList* chunk = GetElementsByTagName(faceGroupElm, "chunk");
		const XMLSize_t chunkCount = chunk->getLength();
		assert(chunkCount == 2);

		DOMElement* chunkElm;
		chunkElm = dynamic_cast<DOMElement*>(chunk->item(0));
		fg.triFaceChunk = ArnBinaryChunk::createFrom(chunkElm, binaryChunkBasePtr);
		chunkElm = dynamic_cast<DOMElement*>(chunk->item(1));
		fg.quadFaceChunk = ArnBinaryChunk::createFrom(chunkElm, binaryChunkBasePtr);
		assert(fg.triFaceChunk && fg.quadFaceChunk);

		ret->m_faceGroup.push_back(fg);
	}

	DOMNodeList* mtrlRefs = GetElementsByTagName(elm, "material");
	const XMLSize_t mtrlRefsCount = mtrlRefs->getLength();
	for (XMLSize_t xx = 0; xx < mtrlRefsCount; ++xx)
	{
		DOMElement* mtrlElm = dynamic_cast<DOMElement*>( mtrlRefs->item(xx) );
		std::string ss;
		GetAttr(ss, mtrlElm, "name");
		ret->m_mtrlRefNameList.push_back(ss.c_str());
	}

	DOMNodeList* uvElmList = GetElementsByTagName(elm, "uv");
	if (uvElmList->getLength())
	{
		DOMElement* uvElm = static_cast<DOMElement*>(GetElementsByTagName(elm, "uv")->item(0));
		DOMElement* triquadUvElm = dynamic_cast<DOMElement*>(GetElementsByTagName(uvElm, "chunk")->item(0));
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
	AssertAttrEquals(elm, "rtclass", "ArnMaterial");
	ArnMaterial* ret = new ArnMaterial();
	SetupArnNodeCommonPart(ret, elm);

	DOMElement* materialElm = GetUniqueChildElement(elm, "material");
	DOMElement* diffuseElm = GetUniqueChildElement(materialElm, "diffuse");
	DOMElement* ambientElm = GetUniqueChildElement(materialElm, "ambient");
	DOMElement* specularElm = GetUniqueChildElement(materialElm, "specular");
	DOMElement* emissiveElm = GetUniqueChildElement(materialElm, "emissive");
	DOMElement* powerElm = GetUniqueChildElement(materialElm, "power");
	assert(diffuseElm && ambientElm && specularElm && emissiveElm && powerElm);

	int shadeless = ParseIntFromAttr(materialElm, "shadeless");
	if (shadeless)
		ret->m_bShadeless = true;

	float r, g, b, a;
	// TODO: Light colors...
	std::string mtrlName;
	GetAttr(mtrlName, elm, "name");
	ret->m_data.m_materialName = mtrlName.c_str();
	ParseRgbaFromElement(&r, &g, &b, &a, diffuseElm);
	ret->m_data.m_d3dMaterial.Diffuse = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, ambientElm);
	ret->m_data.m_d3dMaterial.Ambient = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, specularElm);
	ret->m_data.m_d3dMaterial.Specular = ArnColorValue4f(r, g, b, a);
	ParseRgbaFromElement(&r, &g, &b, &a, emissiveElm);
	ret->m_data.m_d3dMaterial.Emissive = ArnColorValue4f(r, g, b, 1);
	ret->m_data.m_d3dMaterial.Power = ParseFloatFromAttr(powerElm, "value");

	DOMNodeList* textureList = GetElementsByTagName(materialElm, "texture");
	const XMLSize_t textureCount = textureList->getLength();
	for (XMLSize_t xx = 0; xx < textureCount; ++xx)
	{
		DOMElement* textureElm = static_cast<DOMElement*>(textureList->item(xx));
		if (AttrEquals(textureElm, "type", "image"))
		{
			std::string texImageFileName;
			GetAttr(texImageFileName, textureElm, "path");
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
	AssertAttrEquals(elm, "rtclass", "ArnCamera");
	ArnCamera* ret = new ArnCamera();
	SetupArnXformableCommonPart(ret, elm);

	DOMElement* cameraElm = GetUniqueChildElement(elm, "camera");
	float farClip = ParseFloatFromAttr(cameraElm, "farclip");
	float nearClip = ParseFloatFromAttr(cameraElm, "nearclip");
	float fovdeg = ParseFloatFromAttr(cameraElm, "fovdeg");
	ret->setFarClip(farClip);
	ret->setNearClip(nearClip);
	ret->setFov(ArnToRadian(fovdeg));
	std::string typeStr;
	GetAttr(typeStr, cameraElm, "type");
	if (strcmp(typeStr.c_str(), "persp") == 0)
	{
	}
	else if (strcmp(typeStr.c_str(), "ortho") == 0)
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
	AssertAttrEquals(elm, "rtclass", "ArnLight");
	ArnLight* ret = new ArnLight();
	SetupArnXformableCommonPart(ret, elm);

	DOMElement* lightElm = GetUniqueChildElement(elm, "light");
	float r, g, b;
	ParseRgbFromElement(&r, &g, &b, lightElm);
	ret->m_d3dLight.Ambient = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Diffuse = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Specular = ArnColorValue4f(r, g, b, 1);
	std::string lightTypeStr;
	GetAttr(lightTypeStr, lightElm, "type");
	if (strcmp(lightTypeStr.c_str(), "point") == 0)
	{
		// Point light
		ret->m_d3dLight.Type = 1;
		ret->m_d3dLight.Position = ret->getLocalXform_Trans();
	}
	else if (strcmp(lightTypeStr.c_str(), "spot") == 0)
	{
		// Spot light
		ret->m_d3dLight.Type = 2;
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	else if (strcmp(lightTypeStr.c_str(), "directional") == 0)
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
	AssertAttrEquals(elm, "rtclass", "ArnSkeleton");
	ArnSkeleton* ret = new ArnSkeleton();
	SetupArnXformableCommonPart(ret, elm);

	DOMNodeList* actionstrips = GetElementsByTagName(elm, "actionstrip");
	const XMLSize_t actionstripCount = actionstrips->getLength();
	for (XMLSize_t xx = 0; xx < actionstripCount; ++xx)
	{
		DOMElement* actStripElm = reinterpret_cast<DOMElement*>(actionstrips->item(xx));
		std::string asName;
		GetAttr(asName, actStripElm, "name");
		ret->m_actionStripNames.push_back(asName);
	}
	DOMElement* actionElm = GetUniqueChildElement(elm, "action");
	if (actionElm)
	{
		std::string defActName;
		GetAttr(defActName, actionElm, "name");
		ret->setDefaultActionName(defActName.c_str());
	}
	else
	{
		ret->setDefaultActionName("");
	}

	DOMElement* skelElm = GetUniqueChildElement(elm, "skeleton");
	DOMNodeList* boneList = GetElementsByTagName(skelElm, "object");
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
	AssertAttrEquals(elm, "rtclass", "ArnBone");
	ArnBone* ret = new ArnBone();
	SetupArnXformableCommonPart(ret, elm);

	DOMElement* boneElm = GetUniqueChildElement(elm, "bone");
	DOMElement* headElm = GetUniqueChildElement(boneElm, "head");
	DOMElement* tailElm = GetUniqueChildElement(boneElm, "tail");
	DOMElement* rollElm = GetUniqueChildElement(boneElm, "roll");

	ArnVec3 headPos, tailPos;
	ParseArnVec3FromElement(&headPos, headElm);
	ParseArnVec3FromElement(&tailPos, tailElm);
	float roll = ParseFloatFromAttr(rollElm, "value");
	ret->setHeadPos(headPos);
	ret->setTailPos(tailPos);
	ret->setRoll(ArnToRadian(roll));

	DOMNodeList* boneList = GetElementsByTagName(elm, "object");
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
	AssertAttrEquals(elm, "rtclass", "ArnIpo");
	ArnIpo* ret = new ArnIpo();
	SetupArnNodeCommonPart(ret, elm);

	DOMElement* ipoElm = dynamic_cast<DOMElement*>(GetElementsByTagName(elm, "ipo")->item(0));
	assert(ipoElm);
	DOMNodeList* curveList = GetElementsByTagName(ipoElm, "curve");
	const XMLSize_t curveListSize = curveList->getLength();
	for (XMLSize_t xx = 0; xx < curveListSize; ++xx)
	{
		DOMElement* curveElm = dynamic_cast<DOMElement*>( curveList->item(xx) );
		std::string curveTypeStr;
		GetAttr(curveTypeStr, curveElm, "type");
		std::string curveNameStr;
		GetAttr(curveNameStr, curveElm, "name");
		DOMElement* controlPointElm = GetUniqueChildElement(curveElm, "controlpoint");

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

		DOMElement* cpChunk = GetUniqueChildElement(controlPointElm, "chunk");
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
	AssertAttrEquals(elm, "rtclass", "ArnAction");
	ArnAction* ret = new ArnAction();
	SetupArnNodeCommonPart(ret, elm);

	DOMElement* actionElm = GetUniqueChildElement(elm, "action");
	DOMNodeList* list = GetElementsByTagName(actionElm, "objectipomap");
	const XMLSize_t listCount = list->getLength();
	for (XMLSize_t xx = 0; xx < listCount; ++xx)
	{
		DOMElement* mapElm = dynamic_cast<DOMElement*>(list->item(xx));
		std::string ssObjName;
		GetAttr(ssObjName, mapElm, "obj");
		std::string ssIpoName;
		GetAttr(ssIpoName, mapElm, "ipo");
		ret->m_objectIpoNameMap[ssObjName.c_str()] = ssIpoName.c_str();
	}
	return ret;
}

ArnNode*
CreateArnNodeFromXmlElement(DOMElement* elm, char* binaryChunkBasePtr)
{
	std::string rtclassStr;
	GetAttr(rtclassStr, elm, "rtclass");
	ArnNode* ret = 0;
	if (strcmp(rtclassStr.c_str(), "ArnMesh") == 0)
	{
		ret = ArnMesh::createFrom(elm, binaryChunkBasePtr);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnCamera") == 0)
	{
		ret = ArnCamera::createFrom(elm);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnMaterial") == 0)
	{
		ret = ArnMaterial::createFrom(elm);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnSkeleton") == 0)
	{
		ret = ArnSkeleton::createFrom(elm);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnIpo") == 0)
	{
		ret = ArnIpo::createFrom(elm, binaryChunkBasePtr);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnAction") == 0)
	{
		ret = ArnAction::createFrom(elm);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnLight") == 0)
	{
		ret = ArnLight::createFrom(elm);
	}
	else if (strcmp(rtclassStr.c_str(), "ArnBone") == 0)
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
