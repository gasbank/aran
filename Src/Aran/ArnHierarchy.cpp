#include "StdAfx.h"
#include "ArnHierarchy.h"
#include "ArnFile.h"
ArnHierarchy::ArnHierarchy(void)
: ArnNode(NDT_SKELETON), m_data(0)
{
}

ArnHierarchy::~ArnHierarchy(void)
{
}

ArnNode* ArnHierarchy::createFromNodeBase( const NodeBase* nodeBase )
{
	if (nodeBase->m_ndt != NDT_HIERARCHY)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeHierarchy* nh = static_cast<const NodeHierarchy*>(nodeBase);
	ArnHierarchy* node = new ArnHierarchy();
	node->setData(nh);

	return node;
}

void ArnHierarchy::setData( const NodeHierarchy* nh )
{
	m_data = nh;
	setName(m_data->m_nodeName);
}