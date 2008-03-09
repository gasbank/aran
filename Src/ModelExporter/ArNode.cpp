#include "stdafx.h"
#include "ArNode.h"

ArNode::ArNode(void)
{
}

ArNode::~ArNode(void)
{
}

void ArNode::setParent( ArNode* const val )
{
	m_parent = val;
	if (m_parent)
		m_parent->addChild(this);
}

void ArNode::addChild( ArNode* const val )
{
	m_children.push_back(val);
}