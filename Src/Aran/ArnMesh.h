#pragma once
#include "ArnMovable.h"

struct NodeBase;
struct NodeMesh2;
struct NodeMesh3;
class ArnMaterial;

class ArnMesh : public ArnMovable
{
public:
	typedef std::vector<ArnMaterial*> MaterialRefList;

	ArnMesh();
	~ArnMesh(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);

	const LPD3DXMESH&	getD3DXMesh() { return m_d3dxMesh; }
	void				setD3DXMesh(const LPD3DXMESH d3dxMesh) { m_d3dxMesh = d3dxMesh; }
	const MeshData&		getMeshData() const { return m_data; }
	
	// *** INTERNAL USE ONLY START ***
	virtual void		interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
	void				buildFrom(const NodeMesh2* nm);
	void				buildFrom(const NodeMesh3* nm);

	MeshData			m_data;
	LPD3DXMESH			m_d3dxMesh;
	MaterialRefList		m_materialRefList;
};

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH* mesh);
HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH* mesh);