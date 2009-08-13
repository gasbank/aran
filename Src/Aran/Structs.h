// Structs.h
// 2007, 2008, 2009 Geoyeob Kim (gasbank@gmail.com)
#ifndef __STRUCTS_H_
#define __STRUCTS_H_

#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnColorValue4f.h"

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

//////////////////////////////////////////////////////////////////////////

class CurveData
{
public:
	std::string nameStr;
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

enum ArnLightType
{
	ARNLIGHT_POINT = 1,
	ARNLIGHT_SPOT = 2,
	ARNLIGHT_DIRECTIONAL = 3,

	ALT_FORCE_DWORD = 0x7fffffff
};

//
// Data structure for Direct3D compatibility. (D3DLIGHT9)
//
struct ArnLightData
{
	ArnLightType			Type; // ArnLightType
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
	std::string m_materialName;
	std::vector<std::string> m_texImgList;
	ArnMaterialData m_d3dMaterial;
};

struct SkeletonData
{
	std::string				name;
	std::string				associatedMeshName;
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
	, offsetMatrix(ArnConsts::ARNMAT_IDENTITY)
	, infVertexCount(0)
	, indices()
	, weights()
	{
	}
	virtual ~BoneData() {}

	std::string					nameFixed;
	ArnMatrix               offsetMatrix;
	unsigned int			infVertexCount;
	std::vector<DWORD>		indices;
	std::vector<float>		weights;
};



struct MyFrameData
{
	std::string	m_frameName;
	BOOL	m_rootFlag;
	int		m_sibling;
	int		m_firstChild;
};

//////////////////////////////////////////////////////////////////////////
// Unions
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// ModelExporter (classic)

typedef struct tagARN_VDD_S
{
	ArnVec3 pos, normal, tangent, binormal, uvw, color;
	DWORD boneIndices; // packed by four-unsigned-byte
	float boneWeights[4];

} ARN_VDD_S;

// Material & Texture Definition (MTD) for ARN format
// (std::string should be converted to 0-terminated char[])
struct ARN_MTD
{
	std::string				strMatName;
	ArnMaterialData		d3dMat;
	std::string				strTexFileName;
};


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


struct ARN_NDD_CAMERA_CHUNK
{
	ArnVec3		pos;
	ArnVec3		targetPos;
	ArnQuat		rot;
	ArnVec3		upVector;
	ArnVec3		lookAtVector;
	float		farClip;
	float		nearClip;
	float		fov;
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
};

// Hierarchy info

class ArnContainer;

struct ArnFrame
{
	ArnFrame()
	: Name(0)
	, TransformationMatrix(ArnConsts::ARNMAT_IDENTITY)
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
	, combinedMatrix(ArnConsts::ARNMAT_IDENTITY)
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

struct ARN_CAMERA
{
	// eye: Position of camera
	// at: Look-at vector
	// up: Up-vector

	ArnVec3 eye, at, up;
	float farClip, nearClip;
	float angle; // in radian
};

#endif // #ifndef __STRUCTS_H_
