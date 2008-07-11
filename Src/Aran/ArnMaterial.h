#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMaterial1;

// Node that can contain multiple materials
class ArnMaterial : public ArnNode
{
public:
	ArnMaterial();
	~ArnMaterial(void);
	
	static ArnNode*				createFrom(const NodeBase* nodeBase);

private:
	void						buildFrom(const NodeMaterial1* nm);

	std::vector<MaterialData>	m_materials;
};
