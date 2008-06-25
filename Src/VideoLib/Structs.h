// Structs.h
// 2008 Geoyeob Kim (gasbank@gmail.com)
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

#include <d3dx9.h>
#define ARNTYPE_MESH		(0x10)
#define ARNTYPE_CAMERA		(0x20)
#define ARNTYPE_LIGHT		(0x30)

#ifdef __cplusplus
class MY_CUSTOM_MESH_VERTEX
{
public:
	MY_CUSTOM_MESH_VERTEX() {}
	MY_CUSTOM_MESH_VERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v)
	{
		this->x = x; this->y = y; this->z = z;
		this->nx = nx; this->ny = ny; this->nz = nz;
		this->u = u; this->v = v;
	}
	float x, y, z, nx, ny, nz, u, v;
	static const DWORD MY_CUSTOM_MESH_VERTEX_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
};
#endif

typedef struct tagArnVertex
{
	float x, y, z, nx, ny, nz;
} ArnVertex;

typedef struct tagArnMeshObHdr
{
	char type;
	char name[63];
	float loc[3];
	float scl[3];
	float rot[3];
	int vertexCount;
	int faceCount;
} ArnMeshObHdr;
typedef struct tagArnMeshOb
{
	char* raw;
	ArnMeshObHdr* hdr;
	ArnVertex* vertex;
	int (*faces)[3];
} ArnMeshOb;


#endif // #ifndef __STRUCTS_H_
