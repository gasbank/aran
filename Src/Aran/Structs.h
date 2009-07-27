// Structs.h
// 2007, 2008, 2009 Geoyeob Kim (gasbank@gmail.com)
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnColorValue4f.h"
#include "RST_DATA.h"

typedef std::string STRING;

namespace std {
#if defined _UNICODE || defined UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}

#ifndef WIN32
	#define CONST const
	#ifndef CALLBACK
		#define CALLBACK
	#endif
	#define UNREFERENCED_PARAMETER(x) {x=x;}
	#define S_OK				(0)
	#define E_FAIL				(-1)
	static const int			MB_OK = 1 << 0;
	static const int			MB_ICONERROR = 1 << 1;
	static const int			MB_ICONEXCLAMATION = 1 << 2;
	#ifndef FALSE
		static const int			FALSE = 0;
	#endif
	#ifndef TRUE
		static const int			TRUE = 1;
	#endif
	typedef unsigned int		UINT;
	typedef float				FLOAT;
	typedef double				DOUBLE;
	typedef DWORD				LRESULT;
	typedef DWORD				WPARAM;
	typedef DWORD				LPARAM;
	typedef void*				HWND;
	typedef void*				LPDIRECT3DDEVICE9;
	typedef void*				LPDIRECT3DTEXTURE9;
	typedef void*				LPD3DXANIMATIONCONTROLLER;
	typedef void*				LPD3DXMESH;
	typedef void*				LPDIRECT3DVERTEXBUFFER9;
	typedef void*				LPDIRECT3DINDEXBUFFER9;
#endif

#ifndef FAILED
#define FAILED(x) ((x)<0)
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef WIN32
inline static void ZeroMemory(void* addr, size_t len)
{
    memset(addr, 0, len);
}
inline static void OutputDebugStringW(const wchar_t* msg)
{
    fwprintf(stderr, L"%s", msg);
}
inline static void OutputDebugStringA(const char* msg)
{
    fprintf(stderr, "%s", msg);
}
inline static void MessageBoxA(void* noUse, const char* title, const char* content, int msgType)
{
	UNREFERENCED_PARAMETER(noUse);
	UNREFERENCED_PARAMETER(msgType);
    fprintf(stderr, "%s: %s\n", title, content);
}
inline static void MessageBoxW(void* noUse, const wchar_t* title, const wchar_t* content, int msgType)
{
	UNREFERENCED_PARAMETER(noUse);
	UNREFERENCED_PARAMETER(msgType);
    fwprintf(stderr, L"%s: %s\n", title, content);
}
#endif

#ifndef WIN32
	#if 1
	#define OutputDebugString(x) OutputDebugStringW(x)
	#else
	#define OutputDebugString(x) OutputDebugStringA(x)
	#endif
#endif

enum EXPORT_VERSION
{
	EV_UNDEFINED,
	EV_ARN10,
	EV_ARN11,
	EV_ARN20,
	EV_ARN25,
	EV_ARN30,
	EV_FORCE_DWORD = 0x7fffffff
};

enum NODE_DATA_TYPE // or NDD_DATA_TYPE
{
	NDT_UNKNOWN,

	NDT_CONTAINER1			= 0x1000,	// not used
	NDT_CONTAINER2,						// not used
	NDT_CONTAINER3,						// not used
	NDT_CONTAINER4,						// not used
	NDT_CONTAINER5,						// not used

	NDT_MESH1				= 0x2000,	// ARN10, ARN11
	NDT_MESH2,							// ARN20
	NDT_MESH3,							// ARN25
	NDT_MESH4,							// not used
	NDT_MESH5,							// not used

	NDT_LIGHT1				= 0x3000,	// ARN10, ARN11, ARN20
	NDT_LIGHT2,							// ARN25
	NDT_LIGHT3,							// not used
	NDT_LIGHT4,							// not used
	NDT_LIGHT5,							// not used

	NDT_SKELETON1			= 0x4000,	// ARN10, ARN11, ARN20
	NDT_SKELETON2,						// not used
	NDT_SKELETON3,						// not used
	NDT_SKELETON4,						// not used
	NDT_SKELETON5,						// not used

	NDT_HIERARCHY1			= 0x5000,	// ARN10, ARN11, ARN20
	NDT_HIERARCHY2,						// ARN25 (Armature in Blender)
	NDT_HIERARCHY3,						// not used
	NDT_HIERARCHY4,						// not used
	NDT_HIERARCHY5,						// not used

	NDT_ANIM1				= 0x6000,	// ARN10, ARN11, ARN20
	NDT_ANIM2,							// not used
	NDT_ANIM3,							// not used
	NDT_ANIM4,							// not used
	NDT_ANIM5,							// not used

	NDT_CAMERA1				= 0x7000,	// ARN10, ARN11, ARN20
	NDT_CAMERA2,						// ARN25
	NDT_CAMERA3,						// not used
	NDT_CAMERA4,						// not used
	NDT_CAMERA5,						// not used

	NDT_BONE1				= 0x8000,	// ARN10, ARN11, ARN20
	NDT_BONE2,							// ARN25 (Bone in Blender)
	NDT_BONE3,							// not used
	NDT_BONE4,							// not used
	NDT_BONE5,							// not used

	NDT_MATERIAL1			= 0x9000,	// ARN25 : Global materials node which consists of NDT_MATERIAL2
	NDT_MATERIAL2,						// ARN25 : Individual material data node
	NDT_MATERIAL3,						// not used
	NDT_MATERIAL4,						// not used
	NDT_MATERIAL5,						// not used

	NDT_IPO1				= 0xa000,	// ARN25 : Global IPOs node which consists of NDT_IPO2
	NDT_IPO2,							// ARN25 : Individual IPO data node
	NDT_IPO3,							// not used
	NDT_IPO4,							// not used
	NDT_IPO5,							// not used

	NDT_SYMLINK1			= 0xb000,	// ARN25 : Symbolic link to another node
	NDT_SYMLINK2,						// not used
	NDT_SYMLINK3,						// not used
	NDT_SYMLINK4,						// not used
	NDT_SYMLINK5,						// not used

	NDT_ACTION1				= 0xc000,	// ARN25 : Global Action list node which consists all actions (Action list in Blender)
	NDT_ACTION2,						// ARN30 (XML)
	NDT_ACTION3,						// not used
	NDT_ACTION4,						// not used
	NDT_ACTION5,						// not used

	// Runtime objects RTTI
	NDT_RT_CONTAINER		= 0x10000000,
	NDT_RT_MESH,
	NDT_RT_CAMERA,
	NDT_RT_LIGHT,
	NDT_RT_ANIM,
	NDT_RT_MATERIAL,
	NDT_RT_HIERARCHY,
	NDT_RT_SKELETON,
	NDT_RT_BONE,
	NDT_RT_IPO,
	NDT_RT_ACTIONS,
	NDT_RT_SYMLINK,
	NDT_RT_SCENEGRAPH,

	NDT_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////

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

struct BezTripleData
{
	float vec[3][2];
	//float alfa, weight, radius;	/* alfa: tilt in 3D View, weight: used for softbody goal weight, radius: for bevel tapering */
	//short h1, h2; 				/* h1, h2: the handle type of the two handles */
	//char f1, f2, f3, hide;		/* f1, f2, f3: used for selection status,  hide: used to indicate whether BezTriple is hidden */
};

enum CurveType { IPO_CONST, IPO_LIN, IPO_BEZ };
enum CurveName
{
	CN_LocX		= 0x00000001,
	CN_LocY		= 0x00000002,
	CN_LocZ		= 0x00000004,
	CN_ScaleX	= 0x00000010,
	CN_ScaleY	= 0x00000020,
	CN_ScaleZ	= 0x00000040,
	CN_RotX		= 0x00000100,
	CN_RotY		= 0x00000200,
	CN_RotZ		= 0x00000400,
	CN_QuatW	= 0x00001000,
	CN_QuatX	= 0x00002000,
	CN_QuatY	= 0x00004000,
	CN_QuatZ	= 0x00008000,

	CN_SIZE = 9,
	CN_UNKNOWN = 0x7fffffff // error
};
struct CurveDataShell
{
	char* name;
	unsigned int pointCount;
	CurveType type;
	BezTripleData* points;
};
class CurveData
{
public:
	STRING nameStr;
	CurveName name;
	unsigned int pointCount;
	CurveType type;
	std::vector<BezTripleData> points;
};

template<typename V1, typename V2> void ArnRgbaAssign(V1& v1, const V2& v2)
{
	v1.r = v2.r;
	v1.g = v2.g;
	v1.b = v2.b;
	v1.a = v2.a;
}


class ArnMaterialData
{
public:
	ArnColorValue4f Diffuse;
	ArnColorValue4f Ambient;
	ArnColorValue4f Specular;
	ArnColorValue4f Emissive;
	float Power;

#ifdef WIN32
	const D3DMATERIAL9* getConstDxPtr() const { return reinterpret_cast<const D3DMATERIAL9*>(this); }
	D3DMATERIAL9* getDxPtr() { return reinterpret_cast<D3DMATERIAL9*>(this); }
#endif
};

//
// Data structure for Direct3D compatibility. (D3DLIGHT9)
//
struct ArnLightData
{
#ifdef WIN32
	const D3DLIGHT9*		getConstDxPtr() const { return reinterpret_cast<const D3DLIGHT9*>(this); }
	D3DLIGHT9*				getDxPtr() { return reinterpret_cast<D3DLIGHT9*>(this); }
#endif

	enum
	{
		ARNLIGHT_POINT = 1,
		ARNLIGHT_SPOT = 2,
		ARNLIGHT_DIRECTIONAL = 3
	};
	DWORD					Type;
	ArnColorValue4f			Diffuse;
	ArnColorValue4f			Specular;
	ArnColorValue4f			Ambient;
	ArnVec3					Position;
	ArnVec3					Direction;
	float					Range;
	float					Falloff;
	float					Attenuation0;
	float					Attenuation1;
	float					Attenuation2;
	float					Theta;
	float					Phi;
};


class MaterialData
{
public:
	STRING m_materialName;
	std::vector<STRING> m_texImgList;
	ArnMaterialData m_d3dMaterial;
};

struct SkeletonData
{
	STRING				name;
	STRING				associatedMeshName;
	unsigned int		maxWeightsPerVertex; // same as max bones per vertex
	unsigned int		bonesCount;
};

#include "ArnMatrix.h"
#include "ArnQuat.h"
#include "ArnConsts.h"

struct BoneData
{
	BoneData()
	: nameFixed("")
	, offsetMatrix(ArnConsts::D3DXMAT_IDENTITY)
	, infVertexCount(0)
	, indices()
	, weights()
	{
	}
	virtual ~BoneData() {}

	STRING					nameFixed;
	ArnMatrix               offsetMatrix;
	unsigned int			infVertexCount;
	std::vector<DWORD>		indices;
	std::vector<float>		weights;
};
struct BoneIndWeight
{
	DWORD ind;
	float weight;
};

struct Bone2
{
	const char* boneName;
	const char* boneParentName;
	unsigned int indWeightCount;
	BoneIndWeight* indWeight;
	float* localXform; // 4x4 matrix
};



struct MyFrameDataShell
{
	char*	m_frameName;
	BOOL	m_rootFlag;
	int		m_sibling;
	int		m_firstChild;
};
struct MyFrameData
{
	STRING	m_frameName;
	BOOL	m_rootFlag;
	int		m_sibling;
	int		m_firstChild;
};
class MeshData
{
public:
	unsigned int vertexCount;
	unsigned int faceCount;
	unsigned int materialCount;
	std::vector<STRING> matNameList;
	STRING armatureName;
	std::vector<STRING> boneMatIdxMap;
};
//////////////////////////////////////////////////////////////////////////
// Unions
//////////////////////////////////////////////////////////////////////////


struct POINT3FLOAT
{
	float x, y, z;
	static const POINT3FLOAT ZERO;
};

struct POINT4FLOAT
{
	float x, y, z, w;
	static const POINT4FLOAT ZERO;
};

// Vertex Data Definition (VDD) for ARN format
// (Applied format: ARN10, ARN20)
// Point3 ---> Point3Simple type simplification (to remove 3ds Max 9 .h file dependency when compiling game engine)
typedef union tagTEXCOORD{
	float xy[2];
	float uv[2];

} TEXCOORD, POINT2FLOAT;

typedef union tagARN_KDD // Key Data Definition
{
	float rst[4+3+3]; // rot 4 + scale 3 + trans 3
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
	};

	TEXCOORD tc;
#ifdef WIN32
	static const DWORD ARN_VDD_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
#endif
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
	STRING				strMatName;
	ArnMaterialData		d3dMat;
	STRING				strTexFileName;
} ARN_MTD;

struct ARN_MTD_Data
{
	char*				m_strMatName;
	ArnMaterialData*	m_d3dMat;
	char*				m_strTexFileName;
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
	ArnVec3		pos;
	ArnVec3		targetPos;
	ArnQuat		rot;
	ArnVec3		upVector;
	ArnVec3		lookAtVector;
	float		farClip;
	float		nearClip;
	float		fov;
} ARN_NDD_CAMERA_CHUNK;

// Deprecated
struct ArnNodeHeader
{
	NODE_DATA_TYPE ndt;
	STRING uniqueName; // STRING is Multibyte format
	DWORD chunkSize;
	DWORD chunkStartPos;
};

//----------------------------------------------------------------------------
// D3DXKEY_VECTOR3:
// ----------------
// This structure describes a vector key for use in keyframe animation.
// It specifies a vector Value at a given Time. This is used for scale and
// translation keys.
//----------------------------------------------------------------------------
struct ARNKEY_VECTOR3
{
	float Time;
	ArnVec3 Value;

#ifdef WIN32
	D3DXKEY_VECTOR3* getDxPtr() { return reinterpret_cast<D3DXKEY_VECTOR3*>(this); }
#endif
};


//----------------------------------------------------------------------------
// D3DXKEY_QUATERNION:
// -------------------
// This structure describes a quaternion key for use in keyframe animation.
// It specifies a quaternion Value at a given Time. This is used for rotation
// keys.
//----------------------------------------------------------------------------
struct ARNKEY_QUATERNION
{
	float Time;
	ArnQuat Value;

#ifdef WIN32
	D3DXKEY_QUATERNION* getDxPtr() { return reinterpret_cast<D3DXKEY_QUATERNION*>(this); }
#endif
};


// Hierarchy info

class ArnContainer;

struct ArnFrame
{
	ArnFrame()
	: Name(0)
	, TransformationMatrix(ArnConsts::D3DXMAT_IDENTITY)
	, pMeshContainer()
	, pFrameSibling()
	, pFrameFirstChild()
	{
	}
	virtual ~ArnFrame()
	{
	}
	char*									Name;
	ArnMatrix								TransformationMatrix;
	ArnContainer*							pMeshContainer;
	ArnFrame*								pFrameSibling;
	ArnFrame*								pFrameFirstChild;
};

struct MyFrame : public ArnFrame
{
	MyFrame()
	: isRoot(false)
	, combinedMatrix(ArnConsts::D3DXMAT_IDENTITY)
	, sibling(0xffffffff)
	, firstChild(0xffffffff)
	{
		this->Name = this->nameFixed;
		ZeroMemory(&combinedMatrix, sizeof(ArnMatrix));
		ZeroMemory(&TransformationMatrix, sizeof(ArnMatrix));
	}
	~MyFrame() {}
	char		nameFixed[128];
	bool		isRoot;
	ArnMatrix	combinedMatrix;
	size_t		sibling;
	size_t		firstChild;
};

struct Bone : public BoneData
{
	Bone()
	: keys()
	, translationKeys()
	, scaleKeys()
	, rotationKeys()
	, translationKeysSize(0)
	, scaleKeysSize(0)
	, rotationKeysSize(0)
	{
	}
	~Bone()
	{
		/*
		delete [] translationKeys; translationKeys = 0; translationKeysSize = 0;
		delete [] rotationKeys; rotationKeys = 0; rotationKeysSize = 0;
		delete [] scaleKeys; scaleKeys = 0; scaleKeysSize = 0;
		*/
	}

	// Basic data is moved to BoneData (superclass)

	std::vector<RST_DATA>						keys; // keyframe animation data in ARN file

	// keyframe animation data of ID3DXKeyframedAnimationSet
	// this->keys should be converted to following data format using ModelReader::AllocateAsAnimationSetFormat()
	ARNKEY_VECTOR3*			translationKeys;
	ARNKEY_VECTOR3*			scaleKeys;
	ARNKEY_QUATERNION*		rotationKeys;
	UINT					translationKeysSize;
	UINT					scaleKeysSize;
	UINT					rotationKeysSize;
};


// DEPRECATED: use SkeletonData instead
struct SkeletonNode
{
	char				nameFixed[128];
	char				associatedMeshName[128];
	int					maxWeightsPerVertex; // same as max bones per vertex
	int					bonesCount;
	std::vector<Bone>	bones;
};

#endif // #ifndef __STRUCTS_H_
