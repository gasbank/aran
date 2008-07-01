#pragma once
#include "arnnode.h"

class ArnMesh :
	public ArnNode
{
public:
	ArnMesh();
	~ArnMesh(void);
	ArnMeshOb& getOb() { return m_ob; }
	LPD3DXMESH& getD3DMesh() { return m_d3dMesh; }
	const char* getName() const { return m_ob.hdr->name; }
	const D3DXMATRIX* getLocalTransform() const { return (D3DXMATRIX*)m_ob.hdr->localTf; }
private:
	ArnMeshOb m_ob;
	LPD3DXMESH m_d3dMesh;
};
