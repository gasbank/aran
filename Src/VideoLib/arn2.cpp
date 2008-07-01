// arn2.c
// 2008 Geoyeob Kim (gasbank@gmail.com)
#include "stdafx.h"
#include "arn2.h"
#include "load_arn.h"

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN ArnMeshOb* ob, OUT LPD3DXMESH* mesh)
{
	HRESULT hr = 0;
	ArnVertex* v = NULL;
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

	/*DWORD* attributeBuffer = NULL;
	this->lpCustomMesh->LockAttributeBuffer(0, &attributeBuffer);
	int i;
	attributeBuffer[ 0] = 1;
	this->lpCustomMesh->UnlockAttributeBuffer();*/

	return S_OK;
}