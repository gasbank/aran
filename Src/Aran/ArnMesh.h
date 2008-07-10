#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMesh2;

class ArnMesh : public ArnNode
{
public:
	ArnMesh(NODE_DATA_TYPE ndt_meshTypeOnly);
	~ArnMesh(void);
	ArnMeshOb& getOb() { return m_ob; }
	const LPD3DXMESH& getD3DXMesh() { return m_d3dxMesh; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }
	void setD3DXMesh(const LPD3DXMESH d3dxMesh) { m_d3dxMesh = d3dxMesh; }

	// factory method
	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	
private:
	// factory method helpers
	static ArnNode* createFromNodeMesh2(const NodeMesh2* nm2);
	static ArnNode* createFromNodeMesh3();

	ArnMeshOb m_ob;
	LPD3DXMESH m_d3dxMesh;
};

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN ArnMeshOb* ob, OUT LPD3DXMESH* mesh);
HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm2, OUT LPD3DXMESH* mesh);