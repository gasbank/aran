#include "AranPCH.h"
#include "ArnMesh.h"
#include "ArnFile.h"
#include "VideoMan.h"

ArnMesh::ArnMesh()
: ArnNode(NDT_RT_MESH), m_d3dxMesh(0)
{
}

ArnMesh::~ArnMesh(void)
{
	SAFE_RELEASE(m_d3dxMesh);
}

ArnNode* ArnMesh::createFrom( const NodeBase* nodeBase )
{
	ArnMesh* node = new ArnMesh();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_MESH2:
			node->buildFrom(static_cast<const NodeMesh2*>(nodeBase));
			break;
		case NDT_MESH3:
			node->buildFrom(static_cast<const NodeMesh3*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
}

void ArnMesh::buildFrom(const NodeMesh2* nm)
{
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;

	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, &d3dxMesh);
		setD3DXMesh(d3dxMesh);
	}
}

void ArnMesh::buildFrom(const NodeMesh3* nm)
{
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;
	setParentName(nm->m_parentName);
	setLocalXform(*nm->m_localXform);
	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, &d3dxMesh);
		setD3DXMesh(d3dxMesh);
	}
}


//////////////////////////////////////////////////////////////////////////



HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH* mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, D3DXMESH_MANAGED, ARN_VDD::ARN_VDD_FVF, dev, mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	(*mesh)->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vdd, nm->m_meshVerticesCount * sizeof(ARN_VDD));
	(*mesh)->UnlockVertexBuffer();

	(*mesh)->LockIndexBuffer(0, (void**)&ind);
	unsigned int i;
	for (i = 0; i < nm->m_meshFacesCount * 3; ++i)
	{
		assert((nm->m_triangleIndice[i] & 0xffff0000) == 0);
		ind[i] = (WORD)nm->m_triangleIndice[i];
	}
	(*mesh)->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	(*mesh)->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_materialRefs, nm->m_meshFacesCount * sizeof(DWORD));
	(*mesh)->UnlockAttributeBuffer();

	return S_OK;
}

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH* mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, D3DXMESH_MANAGED, ArnVertex::FVF, dev, mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	(*mesh)->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vertex, nm->m_meshVerticesCount * sizeof(ArnVertex));
	(*mesh)->UnlockVertexBuffer();

	(*mesh)->LockIndexBuffer(0, (void**)&ind);
	memcpy(ind, nm->m_faces, nm->m_meshFacesCount * 3 * sizeof(WORD));
	(*mesh)->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	(*mesh)->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_attr, nm->m_meshFacesCount * sizeof(DWORD));
	(*mesh)->UnlockAttributeBuffer();

	return S_OK;
}