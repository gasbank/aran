#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMaterial1;

class ArnMaterial : public ArnNode
{
public:
	ArnMaterial();
	~ArnMaterial(void);
	
	static ArnNode*		createFrom(const NodeBase* nodeBase);

	ArnMaterialOb&		getOb() { return m_ob; }
	const char*			getName() const { return m_ob.hdr->name; }
private:
	void				buildFrom(const NodeMaterial1* nm);

	ArnMaterialOb		m_ob;
};
