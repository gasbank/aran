#pragma once
#include "arnode.h"

class ArLight :
	public ArNode
{
public:
	ArLight(void);
	ArLight(const char* name) : ArNode(name) {}
	virtual ~ArLight(void);
};
