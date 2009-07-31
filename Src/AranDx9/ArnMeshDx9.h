#pragma once

#include "ArnMesh.h"

class ArnMeshDx9 : public ArnMesh
{
public:
	static ArnMeshDx9*						createFrom(const NodeBase* nodeBase);
	~ArnMeshDx9(void);
	// ********************************* INTERNAL USE ONLY START *********************************
	virtual void							interconnect(ArnNode* sceneRoot);
	// *********************************  INTERNAL USE ONLY END  *********************************
private:
											ArnMeshDx9(void);
	void									buildFrom(const NodeMesh2* nm);
	void									buildFrom(const NodeMesh3* nm);

	LPD3DXMESH								m_d3dxMesh;
};
