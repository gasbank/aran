#include "StdAfx.h"
#include "ArnHierarchy.h"
#include "ArnFile.h"
ArnHierarchy::ArnHierarchy(void)
: ArnNode(NDT_RT_HIERARCHY), m_data(0)
{
}

ArnHierarchy::~ArnHierarchy(void)
{
}

ArnNode* ArnHierarchy::createFromNodeBase( const NodeBase* nodeBase )
{
	if (nodeBase->m_ndt != NDT_HIERARCHY1)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeHierarchy1* nh = static_cast<const NodeHierarchy1*>(nodeBase);
	ArnHierarchy* node = new ArnHierarchy();
	node->setData(nh);

	return node;
}

void ArnHierarchy::setData( const NodeHierarchy1* nh )
{
	m_data = nh;
	setName(m_data->m_nodeName);
}