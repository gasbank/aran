#include "AranPCH.h"
#include "ArnMaterial.h"
#include "ArnFile.h"
#include "VideoMan.h"

ArnMaterial::ArnMaterial()
: ArnNode(NDT_RT_MATERIAL), m_bTextureLoaded(false)
{
}

ArnMaterial::~ArnMaterial(void)
{
	TextureList::iterator it = m_d3dTextureList.begin();
	while (it != m_d3dTextureList.end())
	{
		SAFE_RELEASE(*it);
		++it;
	}
}

ArnNode* ArnMaterial::createFrom( const NodeBase* nodeBase )
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

void ArnMaterial::buildFrom( const NodeMaterial1* nm )
{
	m_materialCount = nm->m_materialCount;

	m_bTextureLoaded = true; // No texture loading is needed.
}

void ArnMaterial::buildFrom( const NodeMaterial2* nm )
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

void ArnMaterial::loadTexture()
{
	if (!m_bTextureLoaded)
	{
		unsigned int i;
		for (i = 0; i < m_nodeMaterial->m_texCount; ++i)
		{
			m_data.m_texImgList.push_back(m_nodeMaterial->m_texNameList[i]);

			if (VideoMan::getSingletonPtr())
			{
				LPDIRECT3DTEXTURE9 d3dTex;
				try
				{
					V_VERIFY(D3DXCreateTextureFromFileA(VideoMan::getSingleton().GetDev(), m_nodeMaterial->m_texNameList[i], &d3dTex));
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
}