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

	switch (nodeBase->m_ndt)
	{
	case NDT_MATERIAL1:
		node->buildFrom(static_cast<const NodeMaterial1*>(nodeBase));
		break;
	default:
		delete node;
		throw MyError(MEE_UNDEFINED_ERROR);
	}
	return node;
}

void ArnMaterial::buildFrom( const NodeMaterial1* nm )
{

}