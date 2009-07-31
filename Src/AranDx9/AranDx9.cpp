// AranDx9.cpp : Defines the exported functions for the DLL application.
//

#include "AranDx9PCH.h"
#include "AranDx9.h"
#include "StructsDx9.h"

//////////////////////////////////////////////////////////////////////////

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH& mesh)
{
	if (!dev)
		return E_FAIL; //throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	VideoMan* vman = 0; // TODO: VideoMan
	hr = ArnCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, ARN_VDD::ARN_VDD_FVF, dev, &mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	mesh->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vdd, nm->m_meshVerticesCount * sizeof(ARN_VDD));
	mesh->UnlockVertexBuffer();

	mesh->LockIndexBuffer(0, (void**)&ind);
	unsigned int i;
	for (i = 0; i < nm->m_meshFacesCount * 3; ++i)
	{
		assert((nm->m_triangleIndice[i] & 0xffff0000) == 0);
		ind[i] = (WORD)nm->m_triangleIndice[i];
	}
	mesh->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	mesh->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_materialRefs, nm->m_meshFacesCount * sizeof(DWORD));
	mesh->UnlockAttributeBuffer();

	return S_OK;
}

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH& mesh)
{
	if (!dev)
		return E_FAIL; //throw MyError(MEE_DEVICE_NOT_READY);

	assert(nm->m_armatureName == 0);
	HRESULT hr = 0;

	ArnVertex* v = 0;
	WORD* ind = 0;
	ArnCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, ArnVertex::FVF, dev, &mesh);
	if (FAILED(hr))
	{
		DebugBreak();
		return E_FAIL;
	}
	mesh->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vertex, nm->m_meshVerticesCount * sizeof(ArnVertex));
	mesh->UnlockVertexBuffer();

	mesh->LockIndexBuffer(0, (void**)&ind);
	memcpy(ind, nm->m_faces, nm->m_meshFacesCount * 3 * sizeof(WORD));
	mesh->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	mesh->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_attr, nm->m_meshFacesCount * sizeof(DWORD));
	mesh->UnlockAttributeBuffer();
	return hr;
}

HRESULT arn_build_mesh( IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPDIRECT3DVERTEXBUFFER9& d3dvb, OUT LPDIRECT3DINDEXBUFFER9& d3dib )
{
	if (!dev)
		return E_FAIL; //throw MyError(MEE_DEVICE_NOT_READY);

	assert(nm->m_armatureName);
	assert(d3dvb || d3dib == 0);
	HRESULT hr = 0;

	ArnBlendedVertex* v = 0;
	WORD* ind = 0;

	/*
	ArnVertex        ---     x, y, z, nx, ny, nz, u, v
	ArnBlendedVertex ---     x, y, z, nx, ny, nz, u, v, w0, w1, w2, m0~4
	*/

	dev->CreateVertexBuffer(sizeof(ArnBlendedVertex) * nm->m_meshVerticesCount, D3DUSAGE_WRITEONLY, ArnBlendedVertex::FVF, D3DPOOL_MANAGED, &d3dvb, 0);
	d3dvb->Lock(0, 0, (void**)&v, 0);
	unsigned i;
	for (i = 0; i < nm->m_meshVerticesCount; ++i)
	{
		v[i].x				= nm->m_vertex[i].x;
		v[i].y				= nm->m_vertex[i].y;
		v[i].z				= nm->m_vertex[i].z;
		v[i].normal[0]		= nm->m_vertex[i].nx;
		v[i].normal[1]		= nm->m_vertex[i].ny;
		v[i].normal[2]		= nm->m_vertex[i].nz;
		v[i].u				= nm->m_vertex[i].u;
		v[i].v				= nm->m_vertex[i].v;
		v[i].weight0		= nm->m_weights[3*i + 0];
		v[i].weight1		= nm->m_weights[3*i + 1];
		v[i].weight2		= nm->m_weights[3*i + 2];
		v[i].matrixIndices	= FOUR_BYTES_INTO_DWORD(nm->m_matIdx[4*i + 0], nm->m_matIdx[4*i + 1], nm->m_matIdx[4*i + 2], nm->m_matIdx[4*i + 3]);
	}
	d3dvb->Unlock();

	dev->CreateIndexBuffer(sizeof(WORD) * nm->m_meshFacesCount * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &d3dib, 0);
	d3dib->Lock(0, 0, (void**)&ind, 0);
	memcpy(ind, nm->m_faces, nm->m_meshFacesCount * 3 * sizeof(WORD));
	d3dib->Unlock();

	// TODO: Attribute handling on non-d3dx structure.
	/*DWORD* attrBuf = 0;
	mesh->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_attr, nm->m_meshFacesCount * sizeof(DWORD));
	mesh->UnlockAttributeBuffer();*/

	return hr;
}

HRESULT ArnCreateMeshFVF(IN DWORD NumFaces, IN DWORD NumVertices, IN DWORD FVF, IN LPDIRECT3DDEVICE9 dev, OUT LPD3DXMESH* ppMesh)
{
	V_OKAY( D3DXCreateMeshFVF(NumFaces, NumVertices, 0, FVF, dev, ppMesh) );
	return S_OK;
}


const D3DXVECTOR3* ArnVec3GetConstDxPtr( const ArnVec3& v )
{
	return reinterpret_cast<const D3DXVECTOR3*>(*v);
}

D3DXVECTOR3* ArnVec3GetDxPtr( ArnVec3& v )
{
	return reinterpret_cast<D3DXVECTOR3*>(*v);
}

const D3DMATERIAL9* ArnMaterialGetConstDxPtr(const ArnMaterialData& amd)
{
	return reinterpret_cast<const D3DMATERIAL9*>(&amd);
}

D3DMATERIAL9* ArnMaterialGetDxPtr(ArnMaterialData& amd)
{
	return reinterpret_cast<D3DMATERIAL9*>(&amd);
}

const D3DXMATRIX* ArnMatrixGetConstDxPtr(const ArnMatrix& mat)
{
	return reinterpret_cast<const D3DXMATRIX*>(&mat);
}

D3DXMATRIX* ArnMatrixGetDxPtr(ArnMatrix& mat)
{
	return reinterpret_cast<D3DXMATRIX*>(&mat);
}