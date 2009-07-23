#pragma once

#include "ArnNode.h"

class ArnContainer : public ArnNode
{
public:
	ArnContainer(void);
	~ArnContainer(void);

	// *** INTERNAL USE ONLY START ***
	virtual void			interconnect(ArnNode* sceneRoot) { ArnNode::interconnect(sceneRoot); }
	// *** INTERNAL USE ONLY END ***
};
