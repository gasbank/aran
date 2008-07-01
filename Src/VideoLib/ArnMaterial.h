#pragma once

#include "ArnObject.h"

class ArnMaterial : public ArnObject
{
public:
	ArnMaterial();
	~ArnMaterial(void);
	ArnMaterialOb& getOb() { return m_ob; }
	const char* getName() const { return m_ob.hdr->name; }
private:
	ArnMaterialOb m_ob;
};
