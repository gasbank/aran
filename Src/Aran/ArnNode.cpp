#include "StdAfx.h"
#include "ArnNode.h"

ArnNode::ArnNode(ArnNodeType type)
: ArnObject(type), m_parent(NULL)
{
}

ArnNode::~ArnNode(void)
{
}
