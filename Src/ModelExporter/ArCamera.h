#pragma once
#include "arnode.h"

class ArCamera :
	public ArNode
{
public:
	ArCamera(void);
	ArCamera(const char* name) : ArNode(name) {}
	virtual ~ArCamera(void);
};
