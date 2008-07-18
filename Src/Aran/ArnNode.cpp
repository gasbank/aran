#include "AranPCH.h"
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
	child->detachParent();
	m_children.push_back(child);
	child->setParentName(getName());
	child->setParent(this);
}
void ArnNode::detachChild( ArnNode* child )
{
	m_children.remove(child);
	child->setParent(0);
}
void ArnNode::deleteAllChildren()
{
	ChildrenList::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		delete (*it);
	}
}

ArnNode* ArnNode::getLastNode()
{
	ChildrenList::const_reverse_iterator it = m_children.rbegin();
	if (it != m_children.rend())
		return *it;
	else
		return 0;
}

ArnNode* ArnNode::getNodeByName(const STRING& name)
{
	if (getName() == name)
		return this;

	ChildrenList::const_iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		ArnNode* ret = (*it)->getNodeByName(name);
		if (ret != 0)
			return ret;
	}

	return 0;
}

ArnNode* ArnNode::getNodeAt( unsigned int idx )
{
	if (idx < m_children.size())
	{
		ChildrenList::const_iterator it = m_children.begin();
		while (idx)
		{
			++it;
			--idx;
		}
		return (*it);
	}
	else
		throw MyError(MEE_STL_INDEX_OUT_OF_BOUNDS);
}

void ArnNode::interconnect( ArnNode* sceneRoot )
{
	ChildrenList::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		(*it)->interconnect(sceneRoot);
	}
}

void ArnNode::update( double fTime, float fElapsedTime )
{
	ChildrenList::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		(*it)->update(fTime, fElapsedTime);
	}
}