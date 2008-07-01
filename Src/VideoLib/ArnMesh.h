#pragma once
#include "arnnode.h"

class ArnMesh :
	public ArnNode
{
public:
	ArnMesh(void);
	~ArnMesh(void);

	ArnMeshOb& getOb() { return m_ob; }
	LPD3DXMESH& getD3DMesh() { return m_d3dMesh; }

private:
	ArnMeshOb m_ob;
	LPD3DXMESH m_d3dMesh;
};
