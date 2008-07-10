#include "StdAfx.h"
#include "ArnSkeleton.h"
#include "ArnFile.h"

ArnSkeleton::ArnSkeleton()
: ArnNode(NDT_SKELETON), m_data(0)
{
}

ArnSkeleton::~ArnSkeleton(void)
{
}

ArnNode* ArnSkeleton::createFromNodeBase( const NodeBase* nodeBase )
{
	if (nodeBase->m_ndt != NDT_SKELETON)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeSkeleton* ns = static_cast<const NodeSkeleton*>(nodeBase);
	ArnSkeleton* node = new ArnSkeleton();
	node->setData(ns);

	return node;
}

void ArnSkeleton::setData( const NodeSkeleton* ns )
{
	m_data = ns;
	setName(m_data->m_nodeName);
}