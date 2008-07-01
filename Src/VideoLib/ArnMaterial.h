#pragma once

#include "ArnObject.h"

class ArnMaterial : public ArnObject
{
public:
	ArnMaterial(void);
	~ArnMaterial(void);

	ArnMaterialOb& getOb() { return m_ob; }

private:
	ArnMaterialOb m_ob;
};
