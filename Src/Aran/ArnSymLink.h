#pragma once

#include "ArnNode.h"

class ArnSymLink : public ArnNode
{
public:
	ArnSymLink(void);
	~ArnSymLink(void);

	ArnNode*	getRealNode()				{ return m_node; }
	void		setRealNode(ArnNode* node)	{ m_node = node; }
private:
	ArnNode*	m_node;
};
