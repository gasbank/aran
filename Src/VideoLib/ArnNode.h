#pragma once
#include "arnobject.h"

class ArnNode :
	public ArnObject
{
public:
	ArnNode(ArnNodeType type);
	virtual ~ArnNode(void);

	ArnNode* getParent() { return m_parent; }
	void setParent(ArnNode* node) { m_parent = node; }
	virtual const D3DXMATRIX* getLocalTransform() const = 0;
private:
	ArnNode* m_parent;
};
