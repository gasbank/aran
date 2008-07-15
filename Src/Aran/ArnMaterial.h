#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMaterial1;
struct NodeMaterial2;

class ArnMaterial : public ArnNode
{
public:
	ArnMaterial();
	~ArnMaterial(void);
	
	static ArnNode*				createFrom(const NodeBase* nodeBase);
	unsigned int				getMaterialCount() const { return m_materialCount; }
	const D3DMATERIAL9&			getD3DMaterialData() const { return m_data.m_d3dMaterial; }
private:
	void						buildFrom(const NodeMaterial1* nm);
	void						buildFrom(const NodeMaterial2* nm);

	unsigned int m_materialCount;
	MaterialData m_data;
};
