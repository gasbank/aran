#pragma once
#include "arnnode.h"

class ArnLight :
	public ArnNode
{
public:
	ArnLight();
	~ArnLight(void);
	ArnLightOb& getOb() { return m_ob; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }
private:
	ArnLightOb m_ob;
};
