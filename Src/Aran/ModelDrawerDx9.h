#pragma once
#include "ModelDrawer.h"

////////////////////////////////////////////////////////////////////////
#ifdef WIN32
class ModelDrawerDx9 : public ModelDrawer
{
public:
									ModelDrawerDx9(LPDIRECT3DDEVICE9 lpDev, DWORD fvf);
	virtual							~ModelDrawerDx9();
	
	virtual void					clearMembers();

	HRESULT							Initialize(LPDIRECT3DDEVICE9 lpDev, DWORD fvf, HWND hLoadingWnd, const TCHAR* fileName, LPD3DXANIMATIONCONTROLLER lpAC, const BOOL initAC = FALSE );
	void							SetDev(LPDIRECT3DDEVICE9 lpDev);
	void							SetFVF(DWORD fvf);
	void							SetLoadingWindowHandle(HWND hLoadingWnd);
	

	// (0) find lpMeshes[meshIndex] and corresponding skeletonNode element
	// (1) implement ID3DXSkinInfo using the skeletonNode element (no hierarchy included)
	// (2) call ConvertToIndexedBlendedMesh()
	// (3) return LPD3DXMESH to this->lpSkinnedMeshes
	int								BuildBlendedMeshByMeshIndex(int meshIndex);

	// connect interconnection pointers of this->hierarchy using array index reference
	int								BuildBoneHierarchyByMeshIndex(int meshIndex);

	// keyframeEndIndex means there the last frame;
	// i.e. default argument will build animation set of entire frames
	HRESULT							BuildKeyframedAnimationSetOfSkeletonNodeIndex( int skeletonNodeIndex, int keyFrameStartIndex = 0, int keyFrameEndIndex = -1);

	// update combined transformation matrix hierarchically
	void							UpdateBoneCombinedMatrixByMeshIndex(int meshIndex);
	void							UpdateBoneCombinedMatrixRecursive(MyFrame* startFrame, ArnMatrix& parentCombinedTransform);


	LPD3DXMESH						GetMeshPointer(int i) const;
	LPD3DXMESH						GetSkinnedMeshPointer(int i) const;
	LPD3DXSKININFO					GetSkinInfoPointer(int i) const;



	DWORD							GetFVF() const;
	LPDIRECT3DVERTEXBUFFER9			GetVB() const;

	LPDIRECT3DTEXTURE9				GetTexture(int referenceIndex) const;
	const ArnMatrix*				GetAnimMatControlledByAC(int meshIndex) const;

	HWND							hLoadingWnd;
private:
	LPDIRECT3DDEVICE9				lpDev;
	LPDIRECT3DVERTEXBUFFER9			lpVB;
	DWORD							fvf;
	std::vector<LPDIRECT3DTEXTURE9> textureList;


	BOOL							useLocalAC;


	// for ARN2x format
	LPD3DXMESH*						lpMeshes;
	std::vector<LPD3DXMESH>			lpSkinnedMeshes;
	std::vector<LPD3DXSKININFO>		lpSkinnedMeshesSkinInfo;
	D3DXMESHCONTAINER				meshContainer;
	D3DXFRAME						frame;

	LPD3DXKEYFRAMEDANIMATIONSET		lpKeyframedAnimationSet;
	std::vector<LPD3DXKEYFRAMEDANIMATIONSET> lpKeyframedAnimationSetList;

	ModelReader*					m_reader;

};
#endif