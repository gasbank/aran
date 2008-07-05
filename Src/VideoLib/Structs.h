// Structs.h
// 2008 Geoyeob Kim (gasbank@gmail.com)
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

#include <d3dx9.h>

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

//////////////////////////////////////////////////////////////////////////

enum ArnNodeType
{
	ANT_MESH = 0x10,
	ANT_CAMERA = 0x20,
	ANT_LIGHT = 0x30,
	ANT_MATERIAL = 0x40,
	ANT_IPO = 0x50,
	ANT_FORCE_DWORD = 0x7fffffff,
};
enum
{
	ARNVERTEX_FVF = D3DFVF_XYZ | D3DFVF_NORMAL
};
struct ArnVertex
{
	float x, y, z, nx, ny, nz;
};
struct ArnMeshObHdr
{
	ArnNodeType type;
	char name[64];
	char parName[64]; // parent name
	
	float localTf[4][4];
	float loc[3];
	float scl[3];
	float rot[3];
	float rotQuat[4];
	unsigned int materialCount;
	unsigned int vertexCount;
	unsigned int faceCount;
};
struct ArnMeshOb
{
	ArnMeshObHdr* hdr;
	DWORD* attrToMaterialMap;
	ArnVertex* vertex;
	unsigned short (*faces)[3];
	DWORD* attr;
};
struct ArnCameraObHdr
{
	ArnNodeType type;
	char name[64];
	char parName[64]; // parent name

	float localTf[4][4];
	float loc[3];
	float rot[3];
	int camType;		// 0: perspective, 1: orthogonal
	float angle;
	float clipStart, clipEnd;
	float scale;
};
struct ArnCameraOb
{
	ArnCameraObHdr* hdr;
};
struct ArnLightObHdr
{
	ArnNodeType type;
	char name[64];
	char parName[64]; // parent name

	float localTf[4][4];

};
struct ArnLightOb
{
	ArnLightObHdr* hdr;
	D3DLIGHT9* d3dLight;
};
struct ArnMaterialObHdr
{
	ArnNodeType type;
	char name[64];
	D3DMATERIAL9 d3dMaterial;
};
struct ArnMaterialOb
{
	ArnMaterialObHdr* hdr;
};



struct ArnCurve
{
	enum CurveType { CONSTANT, LINEAR, BEZIER };
	char name[64];
	unsigned int pointCount;
	CurveType type;
	float* data;
};

struct ArnIpoObHdr
{
	static const ArnNodeType type = ANT_IPO;
	char name[64];
	unsigned int curveCount;
};

struct ArnIpoOb
{
	ArnIpoObHdr* hdr;
	ArnCurve* curves;
};




#endif // #ifndef __STRUCTS_H_
