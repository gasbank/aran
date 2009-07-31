#include "AranPCH.h"
#include "ArnMaterial.h"
#include "VideoMan.h"
#include "ArnTexture.h"

ArnMaterial::ArnMaterial()
: ArnNode(NDT_RT_MATERIAL)
, m_bTextureLoaded(false)
, m_bShadeless(false)
{
}

ArnMaterial::~ArnMaterial(void)
{
	foreach (ArnTexture* tex, m_d3dTextureList)
		delete tex;
}

ArnMaterial*
ArnMaterial::createFrom( const NodeBase* nodeBase )
{
	ArnMaterial* node = new ArnMaterial();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_MATERIAL1:
			node->buildFrom(static_cast<const NodeMaterial1*>(nodeBase));
			break;
		case NDT_MATERIAL2:
			node->buildFrom(static_cast<const NodeMaterial2*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
}

void
ArnMaterial::buildFrom( const NodeMaterial1* nm )
{
	m_materialCount = nm->m_materialCount;

	m_bTextureLoaded = true; // No texture loading is needed.
}

void
ArnMaterial::buildFrom( const NodeMaterial2* nm )
{
	// Deep copying from ARN data buffer
	setParentName(nm->m_parentName);
	m_materialCount = 1;
	m_data.m_materialName = getName();
	m_data.m_d3dMaterial = *nm->m_d3dMaterial;

	// Late loading of texture when this material is used by ArnMesh implemented.
	// Just copy NodeMaterial information to member variables.
	m_nodeMaterial = nm;
	m_bTextureLoaded = false;
}

void
ArnMaterial::loadTexture()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
	/*
	if (!m_bTextureLoaded)
	{
		unsigned int i;
		for (i = 0; i < m_nodeMaterial->m_texCount; ++i)
		{
			m_data.m_texImgList.push_back(m_nodeMaterial->m_texNameList[i]);

			if (VideoMan::getSingletonPtr())
			{
				ArnTexture* d3dTex = 0;
				try
				{
					d3dTex = ArnTexture::createFrom(m_nodeMaterial->m_texNameList[i]);
				}
				catch (const MyError& e)
				{
					_LogWrite(e.toString(), LOG_OKAY);
					d3dTex = 0;
				}

				m_d3dTextureList.push_back(d3dTex);
			}
		}
		m_bTextureLoaded = true;
	}
	*/
}

void
ArnMaterial::interconnect( ArnNode* sceneRoot )
{
	ArnNode::interconnect(sceneRoot);
}

void
ArnMaterial::initRendererObject()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

const ArnTexture*
ArnMaterial::getFirstTexture() const
{
	if (getChildren().size())
	{
		const ArnTexture* ret = dynamic_cast<const ArnTexture*>(*getChildren().begin());
		assert(ret);
		return ret;
	}
	else
	{
		return 0;
	}
}

ArnTexture*
ArnMaterial::getD3DTexture( unsigned int idx ) const
{
	ArnTexture* ret = dynamic_cast<ArnTexture*>(getNodeAt(idx));
	if (ret)
		return ret;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ArnMaterial::attachTexture( ArnTexture* ARN_OWNERSHIP tex )
{
	attachChild(tex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetSimpleColoredMaterial(ArnMaterialData* material, ArnColorValue4f color)
{
	material->Ambient.r = material->Diffuse.r = material->Emissive.r = material->Specular.r = color.r;
	material->Ambient.g = material->Diffuse.g = material->Emissive.g = material->Specular.g = color.g;
	material->Ambient.b = material->Diffuse.b = material->Emissive.b = material->Specular.b = color.b;
	material->Power = 1.0f;
}
