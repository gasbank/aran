#pragma once
#include "arnnode.h"

class ArnCamera :
	public ArnNode
{
public:
	ArnCamera(void);
	~ArnCamera(void);
	ArnCameraOb& getOb() { return m_ob; }
private:
	ArnCameraOb m_ob;
};
