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

ArnMaterial* ArnMaterial::createFrom( const NodeBase* nodeBase )
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
}

void ArnMaterial::interconnect( ArnNode* sceneRoot )
{
	ArnNode::interconnect(sceneRoot);
}

void ArnMaterial::initRendererObject()
{
	foreach(ArnTexture* tex, m_d3dTextureList)
	{
		tex->initGl();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetSimpleColoredMaterial(ArnMaterialData* material, ArnColorValue4f color)
{
	material->Ambient.r = material->Diffuse.r = material->Emissive.r = material->Specular.r = color.r;
	material->Ambient.g = material->Diffuse.g = material->Emissive.g = material->Specular.g = color.g;
	material->Ambient.b = material->Diffuse.b = material->Emissive.b = material->Specular.b = color.b;
	material->Power = 1.0f;
}

void ArnSetupMaterialGl(const ArnMaterial* mtrl)
{
	// TODO: Material (ambient? specular?)

	glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&mtrl->getD3DMaterialData().Ambient);
	//glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&POINT4FLOAT::ZERO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (const GLfloat*)&mtrl->getD3DMaterialData().Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, (const GLfloat*)&mtrl->getD3DMaterialData().Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, (const GLfloat*)&mtrl->getD3DMaterialData().Emissive);

	/*
	glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&POINT4FLOAT::ZERO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (const GLfloat*)&mtrl->getD3DMaterialData().Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, (const GLfloat*)&POINT4FLOAT::ZERO);
	*/

	// TODO: Shininess...
	float shininess = 100;
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);

	if (mtrl->getTextureCount())
	{
		const ArnTexture* tex = mtrl->getFirstTexture();
		glBindTexture(GL_TEXTURE_2D, tex->getTextureId());
		GLenum err = glGetError( );
		assert(err == 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (mtrl->isShadeless())
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);
}
