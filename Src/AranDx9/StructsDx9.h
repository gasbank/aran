#pragma once


class MY_CUSTOM_MESH_VERTEX
{
public:
	MY_CUSTOM_MESH_VERTEX()
		: x(0)
		, y(0)
		, z(0)
		, nx(0)
		, ny(0)
		, nz(1)
		, u(0)
		, v(0)
	{
	}
	MY_CUSTOM_MESH_VERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v)
		: x(x)
		, y(y)
		, z(z)
		, nx(nx)
		, ny(ny)
		, nz(nz)
		, u(u)
		, v(v)
	{
	}
	float x, y, z, nx, ny, nz, u, v;
#ifdef WIN32
	static const DWORD MY_CUSTOM_MESH_VERTEX_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
#endif
};

struct ARN_VDD
{
	ArnVec3 vertex;
	ArnVec3 normal;
	union
	{
		DWORD color; // vertex color (may not be used)
	};

	TEXCOORD tc;
#ifdef WIN32
	static const DWORD ARN_VDD_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
#endif
};

struct ArnVertex
{
	float x, y, z, nx, ny, nz, u, v;

#ifdef WIN32
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
#endif
};

struct ArnBlendedVertex
{
	ArnBlendedVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, float w0, float w1, float w2, DWORD matIndices)
		: x(x)
		, y(y)
		, z(z)
		, weight0(w0)
		, weight1(w1)
		, weight2(w2)
		, matrixIndices(matIndices)
		, u(u)
		, v(v)
	{
		normal[0] = nx;
		normal[1] = ny;
		normal[2] = nz;
	}
	float x, y, z;
	float weight0;
	float weight1;
	float weight2;
	DWORD matrixIndices; // 0x44332211 --> 11, 22, 33, 44 are individual bone matrix indices
	float normal[3];
	float u, v;
#ifdef WIN32
	static const DWORD FVF = D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX0;
#endif
};

