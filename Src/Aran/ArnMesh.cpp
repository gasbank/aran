#include "StdAfx.h"
#include "ArnMesh.h"
#include "ArnFile.h"
#include "VideoMan.h"

ArnMesh::ArnMesh(NODE_DATA_TYPE ndt_meshTypeOnly)
: ArnNode(ndt_meshTypeOnly), m_d3dxMesh(0)
{
	if (!(ndt_meshTypeOnly == NDT_MESH1 || ndt_meshTypeOnly == NDT_MESH2 || ndt_meshTypeOnly == NDT_MESH3 || ndt_meshTypeOnly == NDT_MESH4))
	{
		throw MyError(MEE_NOT_A_MESH_TYPE);
	}
}

ArnMesh::~ArnMesh(void)
{
	SAFE_RELEASE(m_d3dxMesh);
}

ArnNode* ArnMesh::createFromNodeBase( const NodeBase* nodeBase )
{
	switch (nodeBase->m_ndt)
	{
	case NDT_MESH1:
		throw MyError(MEE_UNSUPPORTED_NODE);
	case NDT_MESH2:
		return createFromNodeMesh2((const NodeMesh2*)nodeBase);
	case NDT_MESH3:
		throw MyError(MEE_UNSUPPORTED_NODE);
	default:
		throw MyError(MEE_UNDEFINED_ERROR);
	}
}

ArnNode* ArnMesh::createFromNodeMesh2(const NodeMesh2* nm2)
{
	ArnMesh* mesh = new ArnMesh(NDT_MESH2);
	LPD3DXMESH d3dxMesh;
	arn_build_mesh(VideoMan::getSingleton().GetDev(), nm2, &d3dxMesh);
	mesh->setD3DXMesh(d3dxMesh);
	return mesh;
}

ArnNode* ArnMesh::createFromNodeMesh3()
{
	return 0;
}



//////////////////////////////////////////////////////////////////////////


HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN ArnMeshOb* ob, OUT LPD3DXMESH* mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(ob->hdr->faceCount, ob->hdr->vertexCount, D3DXMESH_MANAGED, ARNVERTEX_FVF, dev, mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	(*mesh)->LockVertexBuffer(0, (void**)&v);
	memcpy(v, ob->vertex, ob->hdr->vertexCount * sizeof(ArnVertex));
	(*mesh)->UnlockVertexBuffer();

	(*mesh)->LockIndexBuffer(0, (void**)&ind);
	memcpy(ind, ob->faces, ob->hdr->faceCount * 3 * sizeof(WORD));
	(*mesh)->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	(*mesh)->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, ob->attr, ob->hdr->faceCount * sizeof(DWORD));
	(*mesh)->UnlockAttributeBuffer();

	return S_OK;
}


HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm2, OUT LPD3DXMESH* mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(nm2->m_meshFacesCount, nm2->m_meshVerticesCount, D3DXMESH_MANAGED, ARN_VDD::ARN_VDD_FVF, dev, mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	(*mesh)->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm2->m_vdd, nm2->m_meshVerticesCount * sizeof(ARN_VDD));
	(*mesh)->UnlockVertexBuffer();

	(*mesh)->LockIndexBuffer(0, (void**)&ind);
	unsigned int i;
	for (i = 0; i < nm2->m_meshFacesCount * 3; ++i)
	{
		assert((nm2->m_triangleIndice[i] & 0xffff0000) == 0);
		ind[i] = (WORD)nm2->m_triangleIndice[i];
	}
	(*mesh)->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	(*mesh)->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm2->m_materialRefs, nm2->m_meshFacesCount * sizeof(DWORD));
	(*mesh)->UnlockAttributeBuffer();

	return S_OK;
}