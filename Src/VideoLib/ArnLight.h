#pragma once
#include "arnnode.h"

class ArnLight :
	public ArnNode
{
public:
	ArnLight(void);
	~ArnLight(void);
	ArnLightOb& getOb() { return m_ob; }
private:
	ArnLightOb m_ob;
};
