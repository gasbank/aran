// types.h
// 2007 Geoyeob Kim
//
// This file is shared between Aran and ModelExporter C++ Projects at VS2005 Solution.
// Should have dependency on d3dx9.h but NOT have on any 3ds Max related headers
// since Aran game engine itself must not rely on 3ds Max.
//
#pragma once

#include <d3dx9.h>

//////////////////////////////////////////////////////////////////////////
// Enumerations
//////////////////////////////////////////////////////////////////////////

enum EXPORT_VERSION
{
	EV_UNDEFINED,
	EV_ARN10,
	EV_ARN11,
	EV_ARN20,
	EV_FORCE_DWORD = 0x7fffffff,
};

enum NODE_DATA_TYPE // or NDD_DATA_TYPE
{
	NDT_UNKNOWN,
	NDT_MESH1,
	NDT_MESH2,
	NDT_LIGHT,
	NDT_SKELETON,
	NDT_HIERARCHY,
	NDT_ANIM1,
	NDT_ANIM2,
	NDT_MESH3,
	NDT_CAMERA,
	NDT_FORCE_DWORD = 0x7fffffff,
};



//////////////////////////////////////////////////////////////////////////
// Unions
//////////////////////////////////////////////////////////////////////////


typedef union tagPOINT3FLOAT
{
	float xyz[3];
	struct {
		float x, y, z;
	};
} POINT3FLOAT;

typedef union tagPOINT4FLOAT
{
	float xyzw[4];
	struct {
		float x, y, z, w;
	};
} POINT4FLOAT;

// Vertex Data Definition (VDD) for ARN format
// (Applied format: ARN10, ARN20)
// Point3 ---> Point3Simple type simplification (to remove 3ds Max 9 .h file dependency when compiling game engine)
typedef union tagTEXCOORD{
	float xy[2];
	float uv[2];
	struct {
		float u, v;
	};
	struct {
		float x, y;
	};
} TEXCOORD, POINT2FLOAT;

typedef union tagARN_KDD // Key Data Definition
{
	float rst[4+3+3];
	struct {
		POINT4FLOAT rot;
		POINT3FLOAT scale;
		POINT3FLOAT trans;
	};
} ARN_KDD;






//////////////////////////////////////////////////////////////////////////
// Structs
//////////////////////////////////////////////////////////////////////////




typedef struct tagARN_VDD
{
	POINT3FLOAT vertex;
	POINT3FLOAT normal;
	DWORD color; // vertex color (may not be used)
	TEXCOORD tc;

	static const DWORD ARN_VDD_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
} ARN_VDD;

typedef struct tagARN_VDD_S
{
	POINT3FLOAT pos, normal, tangent, binormal, uvw, color;
	DWORD boneIndices; // packed by four-unsigned-byte
	float boneWeights[4];

} ARN_VDD_S;

// Material & Texture Definition (MTD) for ARN format
// (std::string should be converted to null-terminated char[])
typedef struct tagARN_MTD
{
	std::string strMatName;
	D3DMATERIAL9 d3dMat;
	std::string strTexFileName;
} ARN_MTD;



typedef struct tagARN_NDD_HEADER
{
	NODE_DATA_TYPE ndt;
	char* uniqueName;
	int chunkSize;

} ARN_NDD_HEADER;


typedef struct tagARN_NDD_ANIM1
{
	ARN_NDD_HEADER header;

	int keyframeSize;
	ARN_KDD* pKeyframe;

} ARN_NDD_ANIM1;

typedef struct tagARN_NDD_MESH2
{
	ARN_NDD_HEADER header;

	int vertexSize;
	ARN_VDD* pVertex;

	int faceSize;
	int (*pIndex)[3];

	int* fmt; // Face-Material Table

	int mtdSize; // Material & Texture Definition size
	ARN_MTD* pMTD;

	ARN_NDD_ANIM1 anim;

} ARN_NDD_MESH2;

typedef struct tagARN_NDD_MESH3
{
	ARN_NDD_HEADER header;

	int vertexSize;
	int indexSize;

	ARN_MTD mtd;
	int boneSize;
	int frameSize;

	ARN_VDD_S* pVertex;
	int (*pIndex)[3];
	float (*pAnimKey)[16]; // matrix form

} ARN_NDD_MESH3;

typedef struct tagARN_NDD_CAMERA_CHUNK
{
	
	POINT3FLOAT pos, targetPos;
	POINT4FLOAT rot;

	POINT3FLOAT upVector, lookAtVector;

	float farClip;
	float nearClip;

	// No animation yet
} ARN_NDD_CAMERA_CHUNK;






// Deprecated

struct ArnNodeHeader
{
	NODE_DATA_TYPE ndt;
	std::string uniqueName; // std::string is Multibyte format
	DWORD chunkSize;
	DWORD chunkStartPos;
};


