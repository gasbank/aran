#pragma once

class ArnAnimationController;
class ArnIpo;

// Parsing .arn file
class ModelReader
{
public:
	static ModelReader*				create(const char* fileName, const BOOL initAC = FALSE);
	virtual							~ModelReader(void);

	// Read mesh(vertices, vertex indices, texture coords, ...), materials, animations
	// ARN1x, ARN2x compatible
	int								Read(const char* fileName);

	void							clearMembers();
	void							SetFileName( const char* fileName );

	// GET methods
	int								GetTotalFaceCount() const;
	int								GetMaterialReference(int meshIndex, int faceIndex) const;
	int								GetMaterialReferenceFast(int meshIndex, int materialIndex) const;
	const ArnMaterialData*			GetMaterial(int referenceIndex) const;
	int								GetNotIndMeshCount() const;
	int								GetIndMeshCount() const;
	int								GetFaceCount(int meshIndex) const;
	int								GetMaterialCount(int meshIndex) const;
	int								GetAnimQuatSize(int meshIndex) const;
	RST_DATA*						GetAnimQuat(int meshIndex) const;
	int								GetLightCount() const;
	ArnLightData&					GetLight(int i);
	EXPORT_VERSION					GetExportVersion() const;

	ArnNodeHeader					GetArnNodeHeader(int idx);
	size_t							GetArnNodeHeadersSize();
	const char*					GetFileNameOnly();
	ARN_NDD_CAMERA_CHUNK*			GetFirstCamera();
	BOOL							IsInitialized() const;

	DWORD							GetFVF() const { ARN_THROW_REMOVE_FUNCTION_ERROR }
	void*							GetVB() const { ARN_THROW_REMOVE_FUNCTION_ERROR }
	void							AdvanceTime(float dt) { ARN_THROW_REMOVE_FUNCTION_ERROR }
	ArnAnimationController*			GetAnimationController() const { ARN_THROW_REMOVE_FUNCTION_ERROR }
	void*							GetTexture(int referenceIndex) const { ARN_THROW_REMOVE_FUNCTION_ERROR }
	void*							GetMeshPointer(int i) { ARN_THROW_REMOVE_FUNCTION_ERROR }
	void*							GetSkinnedMeshPointer(int i) const { ARN_THROW_REMOVE_FUNCTION_ERROR }
	const ArnMatrix*				GetAnimMatControlledByAC(int i) const { ARN_THROW_REMOVE_FUNCTION_ERROR }

	ArnIpo*							GetKeyframedAnimationSet( int animSetIndex = 0 ) const { ARN_THROW_NOT_IMPLEMENTED_ERROR }

	int								GetVBLength() const;
	MyFrame*						GetFrameRootByMeshIndex(int meshIndex);
	MyFrame*						GetFrameBySkeletonName(const char* skelName); // for testing
	ArnMatrix*						GetCombinedMatrixByBoneName( const char* boneName );

	const ArnMatrix*				GetTransformationMatrixByBoneName( const char* boneName ) const;

	SkeletonNode*					GetSkeletonNodePointer(int index);
	size_t							GetSkeletonNodeSize() const;


	int								GetMeshIndexBySkeletonIndex(int skelIndex) const;
	int								GetSkeletonIndexByMeshIndex(int meshIndex) const;

	int								getIndexedTotalMeshCount() const { return indTotalMeshCount; }
	const std::string&				getIndMeshNames(int idx) { return indMeshNames[idx]; }

	static int						AllocateAsAnimationSetFormat(UINT sourceArraySize, RST_DATA* sourceArray,
		UINT* pScaleSize, UINT* pRotationSize, UINT* pTranslationSize,
		ARNKEY_VECTOR3** ppScale, ARNKEY_QUATERNION** ppRotation, ARNKEY_VECTOR3** ppTranslation,
		BOOL removeDuplicates = FALSE);

private:
									ModelReader();
	int								BuildTopLevelNodeList();

	int								ParseNDD_Skeleton(int nodeHeaderIndex);
	int								ParseNDD_Mesh1(int nodeHeaderIndex);
	int								ParseNDD_Mesh2(int nodeHeaderIndex);
	int								ParseNDD_Hierarchy(int nodeHeaderIndex);
	int								ParseNDD_Light(int nodeHeaderIndex);
	int								ParseNDD_Camera(int nodeHeaderIndex);
	int								ParseNDD_Anim(NODE_DATA_TYPE belongsToType, Bone* pBone); // child NDD structure

    char                            szFileName[64];
	BOOL							initialized;

	// TODO: global or local animation controller?
	// Use local animation controller to model support instancing
	int								notIndVertTotalSize; // Vertex buffer size in bytes (not indexed)
	int								indVertTotalSize; // Vertex buffer size in bytes (indexed); cumulative value through all this->lpMeshes's VB
	int								lightCount;
	int								notIndTotalMeshCount; // not indexed mesh count
	int								indTotalMeshCount; // indexed mesh count == element count of lpMeshes
	int								skinnedMeshCount; // element count of lpSkinnedMeshes
	int								nodeCount; // the value read from the ARN file
	std::vector<ARN_MTD>			materialList;
	std::vector<int*>				materialReference;
	std::vector<int>				meshVertCount; // vertex count per mesh
	std::vector<int>				materialCount;
	int								totalFaceCount;
	std::vector<int*>				materialReferenceFast;
	std::vector<int>				animQuatSize;
	std::vector<RST_DATA*>			animQuat; // animation data per NDT_MESH1, NDT_MESH2
	std::vector<ArnMatrix>			animMatControlledByAC;
	std::vector<RST_DATA*>			lightAnimQuat; // animation data per light
	std::vector<ArnLightData>		lights;
	EXPORT_VERSION					exportVersion; // ARN10, ARN11, ARN20, ...
	std::vector<STRING>				notIndMeshNames;
	std::vector<ArnNodeHeader>		nodeHeaders;
	typedef std::map<NODE_DATA_TYPE, unsigned int> NodeTypeMap;
	NodeTypeMap						nodeTypeCounter;

	std::ifstream					fin;

	std::vector<int>				fmtOffset;
	std::vector<int>				fmtLength; // equals faceCount(=vertCount / 3)

	// ARN10, ARN11 only
	std::vector<int>				verticesOffset;
	std::vector<int>				verticesLength; // equals vertCount * Vertex(fvf) Size


	// Camera
	std::vector<ARN_NDD_CAMERA_CHUNK> cameraNodes;


	int								hierarchySize;
	std::vector<MyFrame>			hierarchy;


	std::vector<SkeletonNode>		skeletonNode;

	void*							lpDev; // DO NOT USE THIS MEMBER


	std::vector<STRING>				indMeshNames;

	ArnIpo*							lpKeyframedAnimationSet;
	ArnAnimationController*			lpAnimationController;

};


