#include "StdAfx.h"
#include "ArnNode.h"

ArnNode::ArnNode(NODE_DATA_TYPE type)
: ArnObject(type), m_parent(0)
{
}

ArnNode::~ArnNode(void)
{
	deleteAllChildren();
}

void ArnNode::attachChild( ArnNode* child )
{
	m_children.insert(child);
}

void ArnNode::deleteAllChildren()
{
	ChildrenSet::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		delete (*it);
	}
}