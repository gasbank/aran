#pragma once
#include "arnnode.h"

class ArnCamera :
	public ArnNode
{
public:
	ArnCamera();
	~ArnCamera(void);
	ArnCameraOb& getOb() { return m_ob; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }
private:
	ArnCameraOb m_ob;
};
