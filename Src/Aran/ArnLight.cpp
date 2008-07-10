#include "StdAfx.h"
#include "ArnLight.h"
#include "ArnFile.h"
ArnLight::ArnLight()
: ArnNode(NDT_LIGHT)
{
}

ArnLight::~ArnLight(void)
{
}

ArnNode* ArnLight::createFromNodeBase( const NodeBase* nodeBase )
{
	if (nodeBase->m_ndt != NDT_LIGHT)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeLight* nl = static_cast<const NodeLight*>(nodeBase);
	ArnLight* node = new ArnLight();
	node->setData(nl);

	return node;
}

void ArnLight::setData( const NodeLight* nl )
{
	m_data = nl;
	setName(m_data->m_nodeName);
}