#include "AranPCH.h"
#include "ArnLight.h"
#include "ArnFile.h"
ArnLight::ArnLight()
: ArnNode(NDT_RT_LIGHT)
{
}

ArnLight::~ArnLight(void)
{
}

ArnNode* ArnLight::createFrom( const NodeBase* nodeBase )
{
	ArnLight* node = new ArnLight();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_LIGHT1:
			node->buildFrom(static_cast<const NodeLight1*>(nodeBase));
			break;
		case NDT_LIGHT2:
			node->buildFrom(static_cast<const NodeLight2*>(nodeBase));
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

void ArnLight::buildFrom( const NodeLight1* nl )
{	
	m_d3dLight = *nl->m_light;
}

void ArnLight::buildFrom( const NodeLight2* nl )
{
	setLocalXform(*nl->m_localXform);
	m_d3dLight = *nl->m_light;
}