#include "StdAfx.h"
#include "ArnMaterial.h"
#include "ArnFile.h"

ArnMaterial::ArnMaterial()
: ArnNode(NDT_RT_MATERIAL)
{
}

ArnMaterial::~ArnMaterial(void)
{
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
	// Deep copying from ARN data buffer
	unsigned int i;
	for (i = 0; i < nm->m_materials.size(); ++i)
	{
		MaterialData md;
		md.m_materialName	= nm->m_materials[i].m_materialName;
		md.m_d3dMaterial	= *nm->m_materials[i].m_d3dMaterial;
		m_materials.push_back(md);
	}
}