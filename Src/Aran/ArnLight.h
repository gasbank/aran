#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeLight1;
struct NodeLight2;

class ArnLight :
	public ArnNode
{
public:
	ArnLight();
	~ArnLight(void);
	
	static ArnNode*		createFrom(const NodeBase* nodeBase);

	ArnLightOb&			getOb() { return m_ob; }
	const char*			getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX*	getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }

private:
	void				buildFrom(const NodeLight1* nl);
	void				buildFrom(const NodeLight2* nl);

	ArnLightOb			m_ob;
	const NodeLight1*	m_data;
};
