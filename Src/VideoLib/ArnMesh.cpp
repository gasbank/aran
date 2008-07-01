#include "StdAfx.h"
#include "ArnMesh.h"

ArnMesh::ArnMesh()
: ArnNode(ANT_MESH), m_d3dMesh(NULL)
{
}

ArnMesh::~ArnMesh(void)
{
	SAFE_RELEASE(m_d3dMesh);
}
