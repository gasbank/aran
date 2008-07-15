#include "AranPCH.h"
#include "ArnIpo.h"
#include "ArnFile.h"

ArnIpo::ArnIpo(void)
: ArnNode(NDT_RT_IPO)
{
}

ArnIpo::~ArnIpo(void)
{
}

ArnNode* ArnIpo::createFrom( const NodeBase* nodeBase )
{
	ArnIpo* node = new ArnIpo();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_IPO1:
			node->buildFrom(static_cast<const NodeIpo1*>(nodeBase));
			break;
		case NDT_IPO2:
			node->buildFrom(static_cast<const NodeIpo2*>(nodeBase));
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

void ArnIpo::buildFrom( const NodeIpo1* ni )
{

}

void ArnIpo::buildFrom( const NodeIpo2* ni )
{
	setParentName(ni->m_parentName);
}