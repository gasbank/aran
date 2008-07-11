#pragma once
#include "ArnNode.h"

struct NodeCamera1;
struct NodeCamera2;
struct NodeBase;

class ArnCamera : public ArnNode
{
public:
	ArnCamera();
	~ArnCamera(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);

	ArnCameraOb&		getOb() { return m_ob; }
	const char*			getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX*	getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }

private:
	void				buildFrom(const NodeCamera1* nc);
	void				buildFrom(const NodeCamera2* nc);

	ArnCameraOb			m_ob;
	const NodeCamera1*	m_data;

	
};
