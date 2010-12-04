#pragma once

struct MyFrame;
class AnimationSet;
class ArnSkinInfo;
class ArnAnimationController;
class VideoMan;
class ModelReader;
class ArnMesh;
class ArnTexture;

class ModelDrawer
{
public:
	ModelDrawer*					create(const VideoMan* videoMan, const ModelReader* modelReader);
	virtual ~ModelDrawer(void);

	virtual void					clearMembers();

	int								BuildBlendedMeshByMeshIndex(int meshIndex);
	int								BuildBoneHierarchyByMeshIndex(int meshIndex);
	HRESULT							BuildKeyframedAnimationSetOfSkeletonNodeIndex( int skeletonNodeIndex, int keyFrameStartIndex = 0, int keyFrameEndIndex = -1);
	void							UpdateBoneCombinedMatrixByMeshIndex(int meshIndex);
	void							UpdateBoneCombinedMatrixRecursive(MyFrame* startFrame, ArnMatrix& parentCombinedTransform);

	AnimationSet*					GetKeyframedAnimationSet( int animSetIndex = 0 ) const;
	MyFrame*						GetFrameRootByMeshIndex(int meshIndex);
	MyFrame*						GetFrameBySkeletonName(const char* skelName); // for testing
	ArnMesh*						GetMeshPointer(int i) const;
	ArnMesh*						GetSkinnedMeshPointer(int i) const;
	ArnSkinInfo*					GetSkinInfoPointer(int i) const;
	const SkeletonNode*				GetSkeletonNodePointer(int index) const;
	size_t							GetSkeletonNodeSize() const;
	int								GetMeshIndexBySkeletonIndex(int skelIndex) const;
	int								GetSkeletonIndexByMeshIndex(int meshIndex) const;
	const ArnMatrix*				GetTransformationMatrixByBoneName( const char* boneName ) const;
	ArnMatrix*						GetCombinedMatrixByBoneName( const char* boneName );

	HRESULT							AdvanceTime(float timeDelta) const; // redirection to AC

	DWORD							GetFVF() const;
	LPDIRECT3DVERTEXBUFFER9			GetVB() const;
	int								GetVBLength() const;

	ArnTexture*						GetTexture(int referenceIndex) const;
	const ArnMatrix*				GetAnimMatControlledByAC(int meshIndex) const;
	const ArnAnimationController*	GetAnimationController() const { return this->lpAnimationController; }

	HRESULT							ParseNDD_Anim(NODE_DATA_TYPE belongsToType, Bone* pBone); // child NDD structure

protected:
	ModelDrawer(void);

private:
	ArnAnimationController*			lpAnimationController;
};
