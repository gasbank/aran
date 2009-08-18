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

static bool gs_xmlInitialized = false;

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
GetAttr(std::string& val, const TiXmlElement* elm, const char* attrName)
{
	const char* str = elm->Attribute(attrName);
	assert(str);
	val = str;
}

static float
ParseFloatFromAttr(const TiXmlElement* elm, const char* attrName)
{
	float f;
	int ret = elm->QueryFloatAttribute(attrName, &f);
	if (ret == TIXML_SUCCESS)
	{
	}
	else if (ret == TIXML_NO_ATTRIBUTE)
	{
		f = 0;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return f;
}

static int
ParseIntFromAttr(const TiXmlElement* elm, const char* attrName)
{
	int i;
	int ret = elm->QueryIntAttribute(attrName, &i);
	if (ret == TIXML_SUCCESS)
	{
	}
	else if (ret == TIXML_NO_ATTRIBUTE)
	{
		i = 0;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return i;
}

static void
ParseRgbFromElementAttr(float* r, float* g, float* b, const TiXmlElement* elm)
{
	*r = ParseFloatFromAttr(elm, "r");
	*g = ParseFloatFromAttr(elm, "g");
	*b = ParseFloatFromAttr(elm, "b");
}

static void
ParseRgbaFromElementAttr(float* r, float* g, float* b, float* a, const TiXmlElement* elm)
{
	ParseRgbFromElementAttr(r, g, b, elm);
	*a = ParseFloatFromAttr(elm, "a");
}

static void
ParseArnVec3FromElementAttr(ArnVec3* v, const TiXmlElement* elm)
{
	v->x = ParseFloatFromAttr(elm, "x");
	v->y = ParseFloatFromAttr(elm, "y");
	v->z = ParseFloatFromAttr(elm, "z");
}

static void
ParseArnVec3FromElement(ArnVec3* v, const TiXmlElement* elm)
{
	if (sscanf(elm->GetText(), "%f %f %f", &v->x, &v->y, &v->z) != 3)
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

static void
Parse2FloatsFromElement(float* f0, float* f1, const TiXmlElement* elm)
{
	if (sscanf(elm->GetText(), "%f %f", f0, f1) != 2)
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

static void
AssertTagNameEquals(const TiXmlElement* elm, const char* tagName)
{
	if (strcmp(elm->Value(), tagName) != 0)
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

static bool
AttrEquals(const TiXmlElement* elm, const char* attrName, const char* attrVal)
{
	std::string val;
	GetAttr(val, elm, attrName);
	return strcmp(val.c_str(), attrVal) ? false : true;
}

static void
AssertAttrEquals(const TiXmlElement* elm, const char* attrName, const char* attrVal)
{
	if (!AttrEquals(elm, attrName, attrVal))
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

static const TiXmlElement*
GetUniqueChildElement(const TiXmlElement* elm, const char* tagName)
{
	int directChildrenCount = 0;
	const TiXmlElement* uniqueChildren = 0;
	for (const TiXmlElement* e = elm->FirstChildElement(tagName); e; e = e->NextSiblingElement(tagName))
	{
		assert(e->Parent() == elm);
		++directChildrenCount;
		uniqueChildren = e;
	}
	if (directChildrenCount == 1)
	{
		return uniqueChildren;
	}
	else if (directChildrenCount == 0)
	{
		return 0;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

static void
ParseTransformFromElement(ArnMatrix* mat, ArnVec3* scale, ArnQuat* rQuat, ArnVec3* trans,
                          const TiXmlElement* elm)
{
	AssertTagNameEquals(elm, "transform");
	if (AttrEquals(elm, "type", "srt"))
	{
		const TiXmlElement* scaling = GetUniqueChildElement(elm, "scaling");
		const TiXmlElement* rotation = GetUniqueChildElement(elm, "rotation");
		const TiXmlElement* translation = GetUniqueChildElement(elm, "translation");

		ArnVec3 s(1,1,1);
		ArnVec3 r(0,0,0);
		ArnVec3 t(0,0,0);
		if (scaling)
		{
			ParseArnVec3FromElementAttr(&s, scaling);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		if (rotation)
		{
			AssertAttrEquals(rotation, "type", "euler");
			AssertAttrEquals(rotation, "unit", "deg");
			ParseArnVec3FromElementAttr(&r, rotation);
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
			ParseArnVec3FromElementAttr(&t, translation);
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
SetupArnNodeCommonPart(ArnNode* ret, const TiXmlElement* elm)
{
	std::string name;
	GetAttr(name, elm, "name");
	ret->setName(name.c_str());
}

static void
SetupArnXformableCommonPart(ArnXformable* ret, const TiXmlElement* elm)
{
	// Setup ArnXformable common part.
	// This includes ArnNode part and
	// 'transformation', and 'ipo'.
	SetupArnNodeCommonPart(ret, elm);

	ArnVec3 trans, scale;
	ArnQuat rot;
	const TiXmlElement* transformElm = GetUniqueChildElement(elm, "transform");
	ParseTransformFromElement(0, &scale, &rot, &trans, transformElm);
	ret->setLocalXform_Scale(scale);
	ret->setLocalXform_Rot(rot);
	ret->setLocalXform_Trans(trans);
	ret->recalcLocalXform();

	const TiXmlElement* ipoElm = GetUniqueChildElement(elm, "ipo");
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
parseFromToken(const char* targetConst, int& dataOffset, Tokenizer::const_iterator& it,
               const int doCount, T func(const char*))
{
	assert(doCount >= 1);
	char* target = const_cast<char*>(targetConst); // TODO: const_cast used.
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
ArnBinaryChunk::createFrom(const TiXmlElement* elm, const char* binaryChunkBasePtr)
{
	ArnBinaryChunk* ret = new ArnBinaryChunk();
	const TiXmlElement* templ = elm->FirstChildElement("template");
	assert(templ);

	for (const TiXmlElement* e = templ->FirstChildElement("field"); e; e = e->NextSiblingElement("field"))
	{
		std::string typeStr, usageStr;
		GetAttr(typeStr, e, "type");
		GetAttr(usageStr, e, "usage");
		ret->addField( typeStr.c_str(), usageStr.c_str() );
	}
	assert(ret->getFieldCount());

	std::string placeStr;
	GetAttr(placeStr, elm, "place");
	if (strcmp(placeStr.c_str(), "xml") == 0)
	{
		const TiXmlElement* arraydata = elm->FirstChildElement("arraydata");
		ret->m_recordCount = 0;
		for (const TiXmlElement* e = arraydata->FirstChildElement("data"); e; e = e->NextSiblingElement("data"))
		{
			++ret->m_recordCount;
		}

		if (ret->m_recordCount > 0)
		{
			ret->m_data = new char[ ret->m_recordCount * ret->m_recordSize ];
		}
		else if (ret->m_recordCount == 0)
		{
			ret->m_data = 0;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
		ret->m_deallocateData = true;
		int dataOffset = 0;
		for (const TiXmlElement* e = arraydata->FirstChildElement("data"); e; e = e->NextSiblingElement("data"))
		{
			std::string attrStr;
			GetAttr(attrStr, e, "value");
		    boost::char_separator<char> sep(";");
			Tokenizer tok(attrStr, sep);
			Tokenizer::const_iterator it = tok.begin();

			foreach (const Field& field, ret->m_recordDef)
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
	assert(xmlFile);
	if (strlen(xmlFile) == 0)
		return 0;
	assert(strcmp(xmlFile + strlen(xmlFile) - 4, ".xml") == 0);
	assert(gs_xmlInitialized);

	TiXmlDocument xmlDoc(xmlFile);
	if (!xmlDoc.LoadFile())
	{
		std::cerr << "Scene file " << xmlFile << " not found." << std::endl;
		return 0;
	}
	const TiXmlElement* elm = xmlDoc.RootElement();
	AssertTagNameEquals(elm, "object");
	std::string sceneNameStr;
	GetAttr(sceneNameStr, elm, "name");
	std::cout << " - Root scene name: " << sceneNameStr.c_str() << std::endl;

	AssertAttrEquals(elm, "rtclass", "ArnSceneGraph");
	ArnSceneGraph* ret = ArnSceneGraph::createFromEmptySceneGraph();
	SetupArnNodeCommonPart(ret, elm);

	std::string binaryFileName(xmlFile);
	// Only the extension is differ from 'xml' to 'bin'.
	binaryFileName[binaryFileName.size() - 3] = 'b';
	binaryFileName[binaryFileName.size() - 2] = 'i';
	binaryFileName[binaryFileName.size() - 1] = 'n';

	unsigned int binUncompressedSize = (unsigned int)ParseIntFromAttr(elm, "binuncompressedsize");
	ret->m_binaryChunk = ArnBinaryChunk::createFrom(binaryFileName.c_str(), true, binUncompressedSize);

	for (const TiXmlElement* e = elm->FirstChildElement("object"); e; e = e->NextSiblingElement("object"))
	{
		const TiXmlElement* childElm = e;
		assert(childElm);
		ArnNode* childObj = 0;
		if (ret->m_binaryChunk)
		{
			const char* binDataPtr = ret->m_binaryChunk->getConstRawDataPtr();
			childObj = CreateArnNodeFromXmlElement(childElm, binDataPtr);
		}
		else
		{
			childObj = CreateArnNodeFromXmlElement(childElm, 0);
		}
		ret->attachChild(childObj);
	}
	return ret;
}

ArnMesh*
ArnMesh::createFrom(const TiXmlElement* elm, const char* binaryChunkBasePtr)
{
	AssertAttrEquals(elm, "rtclass", "ArnMesh");
	ArnMesh* ret = new ArnMesh();
	SetupArnXformableCommonPart(ret, elm);

	const TiXmlElement* meshElm = GetUniqueChildElement(elm, "mesh");
	const TiXmlElement* vertexElm  = GetUniqueChildElement(meshElm, "vertex");
	const TiXmlElement* faceElm = GetUniqueChildElement(meshElm, "face");
	assert(vertexElm && faceElm);

	if (ParseIntFromAttr(meshElm, "twosided"))
	{
		ret->setTwoSided(true);
	}

	// vertex-chunk element (contains the whole vertices of this mesh)
	const TiXmlElement* vertexChunkElm = GetUniqueChildElement(vertexElm, "chunk");
	ret->m_vertexChunk = ArnBinaryChunk::createFrom(vertexChunkElm, binaryChunkBasePtr);

	// Process vertex groups
	// Note: There is an implicitly created vertex group 0 which includes
	//       the entire vertices having material 0 and unit-weights.
	ArnMesh::VertexGroup vg0;
	vg0.mtrlIndex = 0;
	vg0.vertGroupChunk = 0;
	ret->m_vertexGroup.push_back(vg0);

	// TODO: Vertex groups
	for (const TiXmlElement* e = elm->FirstChildElement("vertgroup"); e; e = e->NextSiblingElement("vertgroup"))
	{
	}

	// Process face groups
	for (const TiXmlElement* e = faceElm->FirstChildElement("facegroup"); e; e = e->NextSiblingElement("facegroup"))
	{
		const TiXmlElement* faceGroupElm = e;
		std::string ssMtrl;
		GetAttr(ssMtrl, faceGroupElm, "mtrl");
		ArnMesh::FaceGroup fg;
		fg.mtrlIndex = atoi(ssMtrl.c_str());
		fg.triFaceChunk = 0;
		fg.quadFaceChunk = 0;
		assert(fg.mtrlIndex >= 0);

		int chunkCount = 0;
		for (const TiXmlElement* e2 = faceGroupElm->FirstChildElement("chunk"); e2; e2 = e2->NextSiblingElement("chunk"))
		{
			switch (chunkCount)
			{
			case 0:
				fg.triFaceChunk = ArnBinaryChunk::createFrom(e2, binaryChunkBasePtr);
				break;
			case 1:
				fg.quadFaceChunk = ArnBinaryChunk::createFrom(e2, binaryChunkBasePtr);
				break;
			default:
				ARN_THROW_UNEXPECTED_CASE_ERROR
			}
			++chunkCount;
		}
		assert(chunkCount == 2);
		assert(fg.triFaceChunk && fg.quadFaceChunk);
		ret->m_faceGroup.push_back(fg);
	}

	// Process materials
	for (const TiXmlElement* e = meshElm->FirstChildElement("material"); e; e = e->NextSiblingElement("material"))
	{
		std::string ss;
		GetAttr(ss, e, "name");
		ret->m_mtrlRefNameList.push_back(ss.c_str());
	}

	// Process UV coordinates
	ret->m_triquadUvChunk = 0;
	for (const TiXmlElement* e = meshElm->FirstChildElement("uv"); e; e = e->NextSiblingElement("uv"))
	{
		const TiXmlElement* triquadUvElm = e->FirstChildElement("chunk");
		assert(triquadUvElm);
		ret->m_triquadUvChunk = ArnBinaryChunk::createFrom(triquadUvElm, binaryChunkBasePtr);
	}

	// Process physics-related
	const TiXmlElement* bbElm = GetUniqueChildElement(elm, "boundingbox");
	if (bbElm)
	{
		const TiXmlElement* bbChunkElm = GetUniqueChildElement(bbElm, "chunk");
		assert(bbChunkElm);
		std::auto_ptr<ArnBinaryChunk> abc(ArnBinaryChunk::createFrom(bbChunkElm, binaryChunkBasePtr));
		assert(abc->getRecordCount() == 8); // Should have 8 corner points of bounding box
		ArnVec3 bb[8];
		for (int i = 0; i < 8; ++i)
			bb[i] = *reinterpret_cast<const ArnVec3*>(abc->getRecordAt(i));
		ret->setBoundingBoxPoints(bb);
	}
	const TiXmlElement* actorElm = GetUniqueChildElement(elm, "actor");
	if (actorElm)
	{
		assert(ret->m_bPhyActor == false);
		ret->m_bPhyActor = true;
		std::string s;
		GetAttr(s, actorElm, "bounds");
		if (s == "box")
		{
			ret->m_abbt = ABBT_BOX;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
		const TiXmlElement* rigidbodyElm = GetUniqueChildElement(actorElm, "rigidbody");
		if (rigidbodyElm)
		{
			ret->m_mass = ParseFloatFromAttr(rigidbodyElm, "mass"); // Have dynamics property
		}
		else
		{
			ret->m_mass = 0; // This object will be completely fixed in dynamics environment.
		}
		if (ret->getLocalXform_Scale().compare(ArnConsts::ARNVEC3_ONE) > 0.001)
		{
			// When a mesh is governed by dynamics, there is no scaling to make valid bounding volume.
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
	}

	// Process constraints
	for (const TiXmlElement* e = elm->FirstChildElement("constraint"); e; e = e->NextSiblingElement("constraint"))
	{
		const char* type = e->Attribute("type");
		if (strcmp(type, "rigidbodyjoint") == 0)
		{
			const TiXmlElement* targetElm = e->FirstChildElement("target");
			const TiXmlElement* pivotElm = e->FirstChildElement("pivot");
			const TiXmlElement* axElm = e->FirstChildElement("ax");
			assert(targetElm && pivotElm && axElm);
			ArnJointData ajd;
			ajd.target = targetElm->GetText();
			ParseArnVec3FromElement(&ajd.pivot, pivotElm);
			ParseArnVec3FromElement(&ajd.ax, axElm);

			for (const TiXmlElement* ee = e->FirstChildElement("limit"); ee; ee = ee->NextSiblingElement("limit"))
			{
				const char* limitType = ee->Attribute("type");
				ArnJointData::ArnJointLimit ajl;
				ajl.type = limitType;
				Parse2FloatsFromElement(&ajl.minimum, &ajl.maximum, ee);
				ajd.limits.push_back(ajl);
			}
			ret->addJointData(ajd);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
	}
	return ret;
}

ArnMaterial*
ArnMaterial::createFrom(const TiXmlElement* elm)
{
	AssertAttrEquals(elm, "rtclass", "ArnMaterial");
	ArnMaterial* ret = new ArnMaterial();
	SetupArnNodeCommonPart(ret, elm);

	const TiXmlElement* materialElm = GetUniqueChildElement(elm, "material");
	const TiXmlElement* diffuseElm = GetUniqueChildElement(materialElm, "diffuse");
	const TiXmlElement* ambientElm = GetUniqueChildElement(materialElm, "ambient");
	const TiXmlElement* specularElm = GetUniqueChildElement(materialElm, "specular");
	const TiXmlElement* emissiveElm = GetUniqueChildElement(materialElm, "emissive");
	const TiXmlElement* powerElm = GetUniqueChildElement(materialElm, "power");
	assert(diffuseElm && ambientElm && specularElm && emissiveElm && powerElm);

	int shadeless = ParseIntFromAttr(materialElm, "shadeless");
	if (shadeless)
		ret->m_bShadeless = true;

	float r, g, b, a;
	// TODO: Light colors...
	std::string mtrlName;
	GetAttr(mtrlName, elm, "name");
	ret->m_data.m_materialName = mtrlName.c_str();
	ParseRgbaFromElementAttr(&r, &g, &b, &a, diffuseElm);
	ret->m_data.m_d3dMaterial.Diffuse = ArnColorValue4f(r, g, b, a);
	if (a == 0)
		std::cerr << " *** Warning: material " << mtrlName.c_str() << " diffuse alpha is zero." << std::endl;
	ParseRgbaFromElementAttr(&r, &g, &b, &a, ambientElm);
	ret->m_data.m_d3dMaterial.Ambient = ArnColorValue4f(r, g, b, a);
	if (a == 0)
		std::cerr << " *** Warning: material " << mtrlName.c_str() << " ambient alpha is zero." << std::endl;
	ParseRgbaFromElementAttr(&r, &g, &b, &a, specularElm);
	ret->m_data.m_d3dMaterial.Specular = ArnColorValue4f(r, g, b, a);
	if (a == 0)
		std::cerr << " *** Warning: material " << mtrlName.c_str() << " specular alpha is zero." << std::endl;
	ParseRgbaFromElementAttr(&r, &g, &b, &a, emissiveElm);
	ret->m_data.m_d3dMaterial.Emissive = ArnColorValue4f(r, g, b, 1);
	if (a == 0)
		std::cerr << " *** Warning: material " << mtrlName.c_str() << " emissive alpha is zero." << std::endl;
	ret->m_data.m_d3dMaterial.Power = ParseFloatFromAttr(powerElm, "value");

	for (const TiXmlElement* e = materialElm->FirstChildElement("texture"); e; e = e->NextSiblingElement("texture"))
	{
		const TiXmlElement* textureElm = e;
		if (AttrEquals(textureElm, "type", "image"))
		{
			std::string texImageFileName;
			GetAttr(texImageFileName, textureElm, "path");
			// Preceding two slashes of file path indicate
			// the present working directory; should be removed first.
			if (strcmp(texImageFileName.substr(0, 2).c_str(), "//") == 0)
			{
				texImageFileName = texImageFileName.substr(2, texImageFileName.length() - 2);
			}
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
ArnCamera::createFrom(const TiXmlElement* elm)
{
	AssertAttrEquals(elm, "rtclass", "ArnCamera");
	ArnCamera* ret = new ArnCamera();
	SetupArnXformableCommonPart(ret, elm);

	const TiXmlElement* cameraElm = GetUniqueChildElement(elm, "camera");
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
ArnLight::createFrom( const TiXmlElement* elm )
{
	AssertAttrEquals(elm, "rtclass", "ArnLight");
	ArnLight* ret = new ArnLight();
	SetupArnXformableCommonPart(ret, elm);

	const TiXmlElement* lightElm = GetUniqueChildElement(elm, "light");
	float r, g, b;
	ParseRgbFromElementAttr(&r, &g, &b, lightElm);
	ret->m_d3dLight.Ambient = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Diffuse = ArnColorValue4f(r, g, b, 1);
	ret->m_d3dLight.Specular = ArnColorValue4f(r, g, b, 1);
	std::string lightTypeStr;
	GetAttr(lightTypeStr, lightElm, "type");
	if (strcmp(lightTypeStr.c_str(), "point") == 0)
	{
		// Point light
		ret->m_d3dLight.Type = ARNLIGHT_POINT;
		ret->m_d3dLight.Position = ret->getLocalXform_Trans();
	}
	else if (strcmp(lightTypeStr.c_str(), "spot") == 0)
	{
		// Spot light
		ret->m_d3dLight.Type = ARNLIGHT_SPOT;
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	else if (strcmp(lightTypeStr.c_str(), "directional") == 0)
	{
		// Directional light
		ret->m_d3dLight.Type = ARNLIGHT_DIRECTIONAL;
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return ret;
}

ArnSkeleton*
ArnSkeleton::createFrom( const TiXmlElement* elm )
{
	AssertAttrEquals(elm, "rtclass", "ArnSkeleton");
	ArnSkeleton* ret = new ArnSkeleton();
	SetupArnXformableCommonPart(ret, elm);

	for (const TiXmlElement* e = elm->FirstChildElement("actionstrip"); e; e = e->NextSiblingElement("actionstrip"))
	{
		const TiXmlElement* actStripElm = e;
		std::string asName;
		GetAttr(asName, actStripElm, "name");
		ret->m_actionStripNames.push_back(asName);
	}
	const TiXmlElement* actionElm = GetUniqueChildElement(elm, "action");
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

	const TiXmlElement* skelElm = GetUniqueChildElement(elm, "skeleton");
	for (const TiXmlElement* e = skelElm->FirstChildElement("object"); e; e = e->NextSiblingElement("object"))
	{
		const TiXmlElement* boneElm = e;
		if (boneElm->Parent() == skelElm)
		{
			ArnNode* bone = CreateArnNodeFromXmlElement(boneElm, 0);
			ret->attachChild(bone);
		}
	}
	for (const TiXmlElement* e = skelElm->FirstChildElement("constraint"); e; e = e->NextSiblingElement("constraint"))
	{
		if (e->Parent() == skelElm)
		{
			const char* type = e->Attribute("type");
			if (strcmp(type, "limitrot") == 0)
			{
				const TiXmlElement* targetElm = e->FirstChildElement("target");
				assert(targetElm);
				const char* targetBoneName = targetElm->GetText();
				ArnNode* boneNode = ret->getNodeByName(targetBoneName);
				ArnBone* bone = dynamic_cast<ArnBone*>(boneNode);
				assert(bone);
				for (const TiXmlElement* ee = e->FirstChildElement("limit"); ee; ee = ee->NextSiblingElement("limit"))
				{
					const char* limitType = ee->Attribute("type");
					const char* unit = ee->Attribute("unit");
					AxisEnum axis;
					float minimum, maximum;
					Parse2FloatsFromElement(&minimum, &maximum, ee);
					if (strcmp(unit, "deg") == 0)
					{
						minimum = ArnToRadian(minimum);
						maximum = ArnToRadian(maximum);
					}
					if (strcmp(limitType, "AngX") == 0)
						axis = AXIS_X;
					else if (strcmp(limitType, "AngY") == 0)
						axis = AXIS_Y;
					else if (strcmp(limitType, "AngZ") == 0)
						axis = AXIS_Z;
					else
						ARN_THROW_UNEXPECTED_CASE_ERROR
					bone->setRotLimit(axis, minimum, maximum);
				}
			}
			else
			{
				ARN_THROW_UNEXPECTED_CASE_ERROR
			}
		}
	}

	return ret;
}

ArnBone*
ArnBone::createFrom( const TiXmlElement* elm )
{
	AssertAttrEquals(elm, "rtclass", "ArnBone");
	ArnBone* ret = new ArnBone();
	SetupArnXformableCommonPart(ret, elm);

	const TiXmlElement* boneElm = GetUniqueChildElement(elm, "bone");
	const TiXmlElement* headElm = GetUniqueChildElement(boneElm, "head");
	const TiXmlElement* tailElm = GetUniqueChildElement(boneElm, "tail");
	const TiXmlElement* rollElm = GetUniqueChildElement(boneElm, "roll");

	ArnVec3 headPos, tailPos;
	ParseArnVec3FromElementAttr(&headPos, headElm);
	ParseArnVec3FromElementAttr(&tailPos, tailElm);
	float roll = ParseFloatFromAttr(rollElm, "value");
	ret->setHeadPos(headPos);
	ret->setTailPos(tailPos);
	ret->setRoll(ArnToRadian(roll));

	for (const TiXmlElement* e = elm->FirstChildElement("object"); e; e = e->NextSiblingElement("object"))
	{
		const TiXmlElement* boneElm = e;
		if (boneElm->Parent() == elm)
		{
			ArnNode* bone = CreateArnNodeFromXmlElement(boneElm, 0);
			ret->attachChild(bone);
		}
	}
	return ret;
}

ArnIpo*
ArnIpo::createFrom(const TiXmlElement* elm, const char* binaryChunkBasePtr)
{
	AssertAttrEquals(elm, "rtclass", "ArnIpo");
	ArnIpo* ret = new ArnIpo();
	SetupArnNodeCommonPart(ret, elm);

	const TiXmlElement* ipoElm = elm->FirstChildElement("ipo");
	assert(ipoElm);
	unsigned int curveListSize = 0;
	for (const TiXmlElement* e = ipoElm->FirstChildElement("curve"); e; e = e->NextSiblingElement("curve"))
	{
		const TiXmlElement* curveElm = e;
		std::string curveTypeStr;
		GetAttr(curveTypeStr, curveElm, "type");
		std::string curveNameStr;
		GetAttr(curveNameStr, curveElm, "name");
		const TiXmlElement* controlPointElm = GetUniqueChildElement(curveElm, "controlpoint");

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

		const TiXmlElement* cpChunk = GetUniqueChildElement(controlPointElm, "chunk");
		ArnBinaryChunk* controlPointChunk = ArnBinaryChunk::createFrom(cpChunk, binaryChunkBasePtr);
		assert(controlPointChunk->getRecordSize() == sizeof(BezTripleData));
		cd.pointCount = controlPointChunk->getRecordCount();
		cd.points.resize(cd.pointCount);
		memcpy(&cd.points[0], controlPointChunk->getConstRawDataPtr(), sizeof(BezTripleData) * cd.pointCount);

		foreach (const BezTripleData& btd, cd.points)
		{
			if ( ret->getEndKeyframe() < (int)btd.vec[1][0] )
				ret->setEndKeyframe((int)btd.vec[1][0]);
		}
		delete controlPointChunk;
		++curveListSize;
	}
	ret->m_curveCount = curveListSize;
	ret->m_ipoCount = 1; // TODO: Is semantically correct one?
	return ret;
}

ArnAction*
ArnAction::createFrom(const TiXmlElement* elm)
{
	AssertAttrEquals(elm, "rtclass", "ArnAction");
	ArnAction* ret = new ArnAction();
	SetupArnNodeCommonPart(ret, elm);

	const TiXmlElement* actionElm = GetUniqueChildElement(elm, "action");
	for (const TiXmlElement* e = actionElm->FirstChildElement("objectipomap"); e; e = e->NextSiblingElement("objectipomap"))
	{
		const TiXmlElement* mapElm = e;
		std::string ssObjName;
		GetAttr(ssObjName, mapElm, "obj");
		std::string ssIpoName;
		GetAttr(ssIpoName, mapElm, "ipo");
		ret->addMap(ssObjName.c_str(), ssIpoName.c_str());
	}
	return ret;
}

ArnNode*
CreateArnNodeFromXmlElement(const TiXmlElement* elm, const char* binaryChunkBasePtr)
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

int ArnInitializeXmlParser()
{
	if (gs_xmlInitialized == false)
	{
		gs_xmlInitialized = true;
		return 0;
	}
	else
	{
		return -1;
	}

}

int ArnCleanupXmlParser()
{
	if (gs_xmlInitialized)
	{
		gs_xmlInitialized = false;
		return 0;
	}
	else
	{
		return -1;
	}
}
