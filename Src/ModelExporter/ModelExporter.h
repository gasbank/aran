#pragma once
#define DEBUG


#include <fstream>

#include <windows.h>
#include <tchar.h>

#include "Main.h"
#include "impexp.h"
#include "Remap.h"
#include "types.h"



// Vertex Data Definition (VDD) for ARN format
// (Applied format: ARN10, ARN20)
struct Custom_FVF
{
	Point3 vertex, normal; // Point3 is 3ds Max 9 type
	int color; // vertex color (may not be used)
	float u, v;
};


struct FULL_VERTEX {
	int origVert;
	Point3 pos;
	Point3 normal;
	Point3 tangent;
	Point3 binormal;
	Point3 uvw;
	Point4 color;
};


struct BoneHierarchy
{
	BoneHierarchy(char* boneName, size_t sibling, size_t firstChild, BOOL isRootBone)
	{
		this->boneName = boneName;
		this->sibling = sibling;
		this->firstChild = firstChild;
		this->isRootBone = isRootBone;
		
	}
	char* boneName;
	BOOL isRootBone;

	// std::vector index
	size_t sibling, firstChild;
};

//  build a table from vertex to array of bones
class BoneWeight
{
public:
	int index;
	float weight;
	bool operator == (BoneWeight const& o) const
	{
		return o.weight == weight;
	}
	bool operator < (BoneWeight const& o) const
	{
		//  sort biggest first
		return o.weight < weight;
	}
};

class ModelExporter : public SceneExport
{
private:
	char fileDescriptor[32]; // ARN10, ARN20, ...

	BOOL isFirstTimeInit;
	BOOL isExportSuccessful;
	char exportTime[128];
	
	Interface* coreInterface;
	TimeValue coreFrame;

	std::string exportName; // export file name
	IGameScene* game;

	std::ofstream fout;
	int rootMaterialCount;
	int nodeCount;

	int currentMeshCondensedVerticesCount;
	Remap currentMeshRemap;
	
	//DWORD numTotalFacesPerMesh, numTotalVerticesPerMesh;

	GMatrix maxToDx;
	int numberOfBonesInfluencingMax;

	// Bone Hierarchy
	std::vector<BoneHierarchy> boneHierarchy;
	int currentBoneDepth;
	int currentBoneParentIndex;
	

public:
	ModelExporter(void);
	~ModelExporter(void);


	

	const static int ffffffff = -1;
	const static int MAX_BONE_WEIGHTS_PER_VERTEX = 40;				// Temp maximum; Refer to MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING
	const static int MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING = 4;	// Effective maximum

	int				ExtCount();					// Number of extensions supported
	const TCHAR*	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR*	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR*	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR*	AuthorName();				// ASCII Author name
	const TCHAR*	CopyrightMessage();			// ASCII Copyright message
	const TCHAR*	OtherMessage1();			// Other message #1
	const TCHAR*	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	BOOL SupportsOptions(int ext, DWORD options);
	int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

	HRESULT StartExporting();
	HRESULT FinalizeExporting();

	int ExportNode(IGameNode* node); // Node Exporting Entry

	HRESULT ExportNDD_Mesh(IGameNode* node); // NDD: Mesh
	HRESULT ExportNDD_Mesh3( IGameNode* node ); // NDT_MESH3
	HRESULT ExportNDD_BoneHierarchy(); // NDD: Hierarchy
	HRESULT ExportNDD_SkinningData(IGameNode* meshNodeToBeSkinned); // NDD: Skeleton(Bones)

	int ExportMeshARN10(IGameNode* node);
	
	int ExportMeshARN20(IGameNode* node);
	//HRESULT ExportMeshARN20_Build( IGameNode* node );

	int ExportMeshARN30( IGameNode* node ); // NDT_MESH3

	HRESULT BuildSkinningData( IGameNode* meshNodeToBeSkinned, std::vector<ARN_VDD_S>& arnVDDS );
	int FillARN_MTDWithIGameMaterial( ARN_MTD& cm, IGameMaterial* igmat, TCHAR* matName );

	int ExportNDD_Camera(IGameNode* node);
	int ExportLight(IGameNode* node, IGameObject* obj);

	BoneHierarchy* findBoneHierarchyByNode(IGameNode* node);
	int BuildNDD_BoneHierarchy(IGameNode* node);
	

	size_t FindBoneIndexByName(const char* boneName);
	size_t FindLastSiblingBoneIndexByIndex(size_t idx);

	int ExportNDD_Anim(IGameNode* node);
	

	

	void PrintChildMaterialInfo(IGameMaterial* igm, int depth);

	static Point3 TransformVector(GMatrix const& gm, Point3 const& p);
	static Point3 TransformVector4(GMatrix const& gm, Point4 const& p);
	static int Point3ToIntColor(Point3& p3);
	

	DWORD AddCondensedVertex(std::vector<FULL_VERTEX>& vec, std::vector<std::pair<float, DWORD> >& acc, FULL_VERTEX const& fv);

	IGameScene* GetGame() const;
	
};
