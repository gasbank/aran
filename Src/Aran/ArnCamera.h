#pragma once
#include "ArnNode.h"
struct NodeCamera;
struct NodeBase;
class ArnCamera :
	public ArnNode
{
public:
	ArnCamera();
	~ArnCamera(void);
	ArnCameraOb& getOb() { return m_ob; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	void setData(const NodeCamera* nc);
private:
	ArnCameraOb m_ob;
	const NodeCamera* m_data;
};
