#pragma once
#include "arnnode.h"
struct NodeBase;
struct NodeLight;
class ArnLight :
	public ArnNode
{
public:
	ArnLight();
	~ArnLight(void);
	ArnLightOb& getOb() { return m_ob; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	
private:
	void setData(const NodeLight* nl);

	ArnLightOb m_ob;
	const NodeLight* m_data;
};
