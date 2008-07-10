// Structs.h
// 2007, 2008 Geoyeob Kim (gasbank@gmail.com)
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

enum
{
	ARNVERTEX_FVF = D3DFVF_XYZ | D3DFVF_NORMAL
};

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
	// ARN20 or before
	NDT_MESH1,
	NDT_MESH2,
	NDT_LIGHT,
	NDT_SKELETON,
	NDT_HIERARCHY,
	NDT_ANIM1,
	NDT_ANIM2,
	NDT_MESH3,
	NDT_CAMERA,
	NDT_BONE,
	// ARN21 or later
	NDT_MESH4		= 0x10,
	NDT_CAMERA2		= 0x20,
	NDT_LIGHT2		= 0x30,
	NDT_MATERIAL	= 0x40,
	NDT_IPO			= 0x50,
	NDT_CONTAINER	= 0x60,
	NDT_FORCE_DWORD = 0x7fffffff,
};

//////////////////////////////////////////////////////////////////////////

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



struct ArnVertex
{
	float x, y, z, nx, ny, nz;
};
struct ArnMeshObHdr
{
	NODE_DATA_TYPE type;
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
	NODE_DATA_TYPE type;
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
	NODE_DATA_TYPE type;
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
	NODE_DATA_TYPE type;
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
	static const NODE_DATA_TYPE type = NDT_IPO;
	char name[64];
	unsigned int curveCount;
};

struct ArnIpoOb
{
	ArnIpoObHdr* hdr;
	ArnCurve* curves;
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
// ModelExporter (classic)

typedef struct tagARN_VDD
{
	POINT3FLOAT vertex;
	POINT3FLOAT normal;
	union
	{
		DWORD color; // vertex color (may not be used)
		struct 
		{
			unsigned char r, g, b, a;
		};
	};
	
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
// (STRING should be converted to 0-terminated char[])
typedef struct tagARN_MTD
{
	STRING strMatName;
	D3DMATERIAL9 d3dMat;
	STRING strTexFileName;
} ARN_MTD;

struct ARN_MTD_Data
{
	char* m_strMatName;
	D3DMATERIAL9* m_d3dMat;
	char* m_strTexFileName;
};

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
	STRING uniqueName; // STRING is Multibyte format
	DWORD chunkSize;
	DWORD chunkStartPos;
};


struct RST_DATA
{
	float x, y, z, w; // rotation
	float sx, sy, sz; // scaling
	float tx, ty, tz; // translation
};



#endif // #ifndef __STRUCTS_H_
