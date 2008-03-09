#pragma once

#include "ArNode.h"

class ArNodeFactory
{
public:
	ArNodeFactory(void);
	~ArNodeFactory(void);

	static ArNode* create(ArNode::Type type, const char* name);
};
