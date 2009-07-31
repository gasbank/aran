#pragma once

//
// Originally defined in Aran/Structs.h
//
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
	NDT_RT_TEXTURE,
	NDT_RT_RENDERABLEOBJECT,

	NDT_FORCE_DWORD = 0x7fffffff
};

struct MY_COLOR_VERTEX
{
	float x, y, z;
	DWORD color;
};
//
// Classes and Structures
//
enum RendererType
{
	RENDERER_UNKNOWN,
	RENDERER_DX9,
	RENDERER_GL
};

// Vertex Data Definition (VDD) for ARN format
// (Applied format: ARN10, ARN20)
// Point3 ---> Point3Simple type simplification (to remove 3ds Max 9 .h file dependency when compiling game engine)
typedef union tagTEXCOORD{
	float xy[2];
	float uv[2];

} TEXCOORD, POINT2FLOAT;

struct ArnMaterialData;

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

typedef union tagARN_KDD // Key Data Definition
{
	float rst[4+3+3]; // rot 4 + scale 3 + trans 3
} ARN_KDD;

typedef struct tagARN_NDD_ANIM1
{
	ARN_NDD_HEADER header;

	int keyframeSize;
	ARN_KDD* pKeyframe;

} ARN_NDD_ANIM1;

struct ARN_VDD;
struct ARN_MTD;

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

// Deprecated
struct ArnNodeHeader
{
	NODE_DATA_TYPE ndt;
	std::string uniqueName; // std::string is Multibyte format
	DWORD chunkSize;
	DWORD chunkStartPos;
};

//
// Originally defined at Aran/ArnFile.h
//
struct NodeBase
{
	virtual ~NodeBase() {}

	NODE_DATA_TYPE m_ndt;
	char* m_nodeName;
	unsigned int m_nodeChunkSize;
};

struct NodeUnidentified : public NodeBase
{
	int m_dummy;
};

struct NodeMaterial1 : public NodeBase
{
	unsigned int m_materialCount;
};

struct NodeMaterial2 : public NodeBase
{
	char*				m_parentName;
	char*				m_materialName;
	ArnMaterialData*	m_d3dMaterial;
	unsigned int		m_texCount;
	std::vector<char*>	m_texNameList;
};



struct NodeMesh2 : public NodeBase
{
	~NodeMesh2() { delete [] m_mtds; }

	unsigned int m_meshVerticesCount;
	ARN_VDD* m_vdd;
	unsigned int m_meshFacesCount;
	unsigned int* m_triangleIndice;
	unsigned int* m_materialRefs;
	unsigned int m_materialCount;
	ARN_MTD_Data* m_mtds;
};

class ArnMatrix;
struct ArnVertex;

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

struct NodeMesh3 : public NodeBase
{
	const char* m_parentName;
	const char* m_ipoName;
	const ArnMatrix* m_localXform;
	BOOL m_bArmature;
	//D3DXMATRIX* m_unusedXform;
	unsigned int m_materialCount;
	unsigned int m_meshVerticesCount;
	unsigned int m_meshFacesCount;
	const DWORD* m_attrToMaterialMap; // unused
	std::vector<const char*> m_matNameList;
	const ArnVertex* m_vertex;
	const unsigned short* m_faces;
	const DWORD* m_attr;
	const char* m_armatureName;
	std::vector<Bone2> m_bones;

	std::vector<const char*> m_boneMatIdxMap;
	float* m_weights; // Start address of v0[w0, w1, w2] v1[w0, w1, w2] ... w? are floats
	unsigned char* m_matIdx; // v0[m0, m1, m2, m3], v1[m0, m1, m2, m3] ... m? are unsigned chars(0~255)

};

struct RST_DATA
{
	float x, y, z, w; // rotation
	float sx, sy, sz; // scaling
	float tx, ty, tz; // translation
};

struct NodeAnim1 : public NodeBase
{
	unsigned int m_keyCount;
	RST_DATA* m_rstKeys;
};

struct NodeBone1 : public NodeBase
{
	ArnMatrix* m_offsetMatrix;
	unsigned int m_infVertexCount; // influencing vertex count
	unsigned int* m_vertexIndices;
	float* m_weights;
};

struct NodeBone2 : public NodeBase
{
	const char* m_parentBoneName;
	ArnMatrix* m_offsetMatrix;
	unsigned int m_infVertCount;
	BoneIndWeight* m_indWeightArray;
};

struct NodeSkeleton1 : public NodeBase
{
	char* m_associatedMeshName;
	unsigned int m_maxWeightsPerVertex;
	unsigned int m_boneCount;
};

struct NodeHierarchy2 : public NodeBase
{
	const char* m_parentName;
	unsigned int m_boneCount;
	std::vector<Bone2> m_bones;
};

struct MyFrameDataShell
{
	char*	m_frameName;
	BOOL	m_rootFlag;
	int		m_sibling;
	int		m_firstChild;
};

struct NodeHierarchy1 : public NodeBase
{
	~NodeHierarchy1()
	{
		delete [] m_frames;
	}
	unsigned int m_frameCount;
	MyFrameDataShell* m_frames;
};

struct ArnLightData;

struct NodeLight1 : public NodeBase
{
	ArnLightData* m_light;
};
struct NodeLight2 : public NodeBase
{
	char* m_parentName;
	char* m_ipoName;
	ArnMatrix* m_localXform;
	ArnLightData* m_light;
};

struct ARN_NDD_CAMERA_CHUNK;

struct NodeCamera1 : public NodeBase
{
	ARN_NDD_CAMERA_CHUNK* m_camera;
};
struct NodeCamera2 : public NodeBase
{
	enum CamType { CT_ORTHO = 1, CT_PERSP = 0 };
	char* m_parentName;
	char* m_ipoName;
	ArnMatrix* m_localXform;
	CamType m_camType;
	float m_angle;
	float m_clipStart;
	float m_clipEnd;
	float m_scale;
};

struct NodeIpo1 : public NodeBase
{
	unsigned int m_ipoCount;
};

enum CurveType
{
	IPO_CONST,
	IPO_LIN,
	IPO_BEZ
};

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

struct BezTripleData
{
	float vec[3][2];
	//float alfa, weight, radius;	/* alfa: tilt in 3D View, weight: used for softbody goal weight, radius: for bevel tapering */
	//short h1, h2; 				/* h1, h2: the handle type of the two handles */
	//char f1, f2, f3, hide;		/* f1, f2, f3: used for selection status,  hide: used to indicate whether BezTriple is hidden */
};

struct CurveDataShell
{
	char* name;
	unsigned int pointCount;
	CurveType type;
	BezTripleData* points;
};

struct NodeIpo2 : public NodeBase
{
	~NodeIpo2() { delete [] m_curves; }

	char* m_parentName;
	unsigned int m_curveCount;
	CurveDataShell* m_curves;
};
struct NodeAction1 : public NodeBase
{
	unsigned int m_actionCount;
	std::vector<std::pair<const char*, std::vector<std::pair<const char*, const char*> > > > m_actions;
};

struct NodeSymLink1 : public NodeBase
{
	int dummy;
};
//////////////////////////////////////////////////////////////////////////
struct ArnBinaryFile
{
	ArnBinaryFile()
		: m_fileSize(0)
		, m_data(0)
		, m_curPos(0)
	{
	}

	unsigned int m_fileSize;
	char* m_data;
	unsigned int m_curPos;
};

struct ArnFileData
{
	char* m_fileDescriptor;
	unsigned int m_nodeCount;
	typedef std::vector<NodeBase*> NodeList;
	NodeList m_nodes;
	char* m_terminalDescriptor;

	ArnBinaryFile m_file;
};

class MeshData
{
public:
	unsigned int vertexCount;
	unsigned int faceCount;
	unsigned int materialCount;
	std::vector<std::string> matNameList;
	std::string armatureName;
	std::vector<std::string> boneMatIdxMap;
};

