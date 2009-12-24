#pragma once

#include "ArnMesh.h"

class ArnMeshDx9 : public ArnRenderableObject
{
public:
											~ArnMeshDx9(void);
	static ArnMeshDx9*						createFrom(const ArnMesh* mesh);
	virtual int								render(bool bIncludeShadeless) const;
	virtual void							cleanup();

	/* Should be removed V */
	static ArnMeshDx9*						createFrom(const NodeBase* nodeBase);
	virtual void							interconnect(ArnNode* sceneRoot);
	/* Should be removed ^ */
private:
											ArnMeshDx9(void);
	/* Should be removed V */
	void									buildFrom(const NodeMesh2* nm);
	void									buildFrom(const NodeMesh3* nm);
	/* Should be removed ^ */

	LPD3DXMESH								m_d3dxMesh;
};
