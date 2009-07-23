#include "AranPCH.h"
#include "ModelDrawerDx9.h"
#include "ModelReader.h"
#include "ArnMath.h"

#ifdef WIN32
ModelDrawerDx9::ModelDrawerDx9( LPDIRECT3DDEVICE9 lpDev, DWORD fvf )
{

	// for skinned mesh feature
	ZeroMemory(&this->meshContainer, sizeof(meshContainer));
	ZeroMemory(&this->frame, sizeof(frame));
}

void ModelDrawerDx9::clearMembers()
{
	lpVB					= 0;
	fvf						= 0;
	hLoadingWnd				= 0;
	lpMeshes				= 0;
	lpKeyframedAnimationSet	= 0;
	lpDev					= 0;
	useLocalAC				= FALSE;

	ModelDrawer::clearMembers();
}

ModelDrawerDx9::~ModelDrawerDx9()
{
	//SAFE_RELEASE(this->lpVB);
	//int i;
	//for (i = 0; i < (int)this->textureList.size(); i++)
	//{
	//	SAFE_RELEASE(this->textureList[i]);
	//}
	//if (this->lpMeshes != 0)
	//{
	//	for (i = 0; i < this->getIndexedTotalMeshCount(); i++)
	//	{
	//		SAFE_RELEASE(this->lpMeshes[i]);
	//	}
	//	delete [] this->lpMeshes;
	//	this->lpMeshes = 0;
	//}

	//for (i = 0; i < (int)this->lpSkinnedMeshes.size(); i++)
	//{
	//	SAFE_RELEASE(this->lpSkinnedMeshes[i]);
	//}
	//this->lpSkinnedMeshes.resize(0);

	//for (i = 0; i < (int)this->lpSkinnedMeshesSkinInfo.size(); i++)
	//{
	//	SAFE_RELEASE(this->lpSkinnedMeshesSkinInfo[i]);
	//}
	//this->lpSkinnedMeshesSkinInfo.resize(0);


	//// Deallocate animation sets
	//SAFE_RELEASE(this->lpKeyframedAnimationSet);
	//for (i = 0; i < (int)this->lpKeyframedAnimationSetList.size(); i++)
	//{
	//	SAFE_RELEASE(this->lpKeyframedAnimationSetList[i]);
	//}

	//if (this->useLocalAC)
	//{
	//	SAFE_RELEASE(this->lpAnimationController);
	//}
}

//ArnIpo* ModelReader::GetKeyframedAnimationSet(int animSetIndex) const
//{
//	return this->lpKeyframedAnimationSetList[animSetIndex];
//}

LPDIRECT3DVERTEXBUFFER9 ModelDrawerDx9::GetVB() const
{
	return this->lpVB;
}

void ModelDrawerDx9::SetDev(LPDIRECT3DDEVICE9 lpDev)
{
	this->lpDev = lpDev;
}
void ModelDrawerDx9::SetFVF(DWORD fvf)
{
	this->fvf = fvf;
}
DWORD ModelDrawerDx9::GetFVF() const
{
	return this->fvf;
}
void ModelDrawerDx9::SetLoadingWindowHandle(HWND hLoadingWnd)
{
	this->hLoadingWnd = hLoadingWnd;
}



int ModelDrawerDx9::BuildBoneHierarchyByMeshIndex(int meshIndex)
{
	/*
	size_t s;
	for (s = 0; s < this->hierarchy.size(); s++)
	{
		MyFrame* currentBone = &this->hierarchy[s];

		size_t siblingIndex = currentBone->sibling;
		if (siblingIndex == -1)
		{
			currentBone->pFrameSibling = 0;
		}
		else
		{
			currentBone->pFrameSibling = &this->hierarchy[siblingIndex];
		}

		size_t firstChildIndex = currentBone->firstChild;
		if (firstChildIndex == -1)
		{
			currentBone->pFrameFirstChild = 0;
		}
		else
		{
			currentBone->pFrameFirstChild = &this->hierarchy[firstChildIndex];
		}
		currentBone->pMeshContainer = 0;
		currentBone->Name = currentBone->nameFixed;
	}
	*/

	return 0;
}



int ModelDrawerDx9::BuildBlendedMeshByMeshIndex(int meshIndex)
{
	// should Read() method called in precedence
	if (m_reader->GetExportVersion() != EV_ARN20)
	{
		return -1;
	}
	int i;
	for (i = 0; i < (int)m_reader->GetSkeletonNodeSize(); i++)
	{
		if (strcmp(m_reader->GetSkeletonNodePointer(i)->associatedMeshName, m_reader->getIndMeshNames(meshIndex).c_str()) == 0)
		{
			break;
		}
	}
	SkeletonNode* currentSkeleton = m_reader->GetSkeletonNodePointer(i);

	LPD3DXSKININFO lpSkinInfo = 0;
	int vertCount = this->lpMeshes[meshIndex]->GetNumVertices();
	size_t boneCount = currentSkeleton->bones.size();


	if (FAILED(D3DXCreateSkinInfoFVF(vertCount, MY_CUSTOM_MESH_VERTEX::MY_CUSTOM_MESH_VERTEX_FVF, (int)boneCount, &lpSkinInfo)))
	{
		MessageBox(0, _T("D3DXCreateSkinInfoFVF() failed."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}



	// Bone name, offset matrix, influence
	V_OKAY(lpSkinInfo->SetMinBoneInfluence(1e-4f));

	int j = 0;
	for (i = 0; i < (int)boneCount; i++)
	{
		Bone* bone = &currentSkeleton->bones[i];
		V_OKAY(lpSkinInfo->SetBoneName(i, bone->nameFixed.c_str()));
		V_OKAY(lpSkinInfo->SetBoneOffsetMatrix(i, bone->offsetMatrix.getConstDxPtr()));

		// optimized
		//V_OKAY(lpSkinInfo->SetBoneInfluence(i, (DWORD)bone->influencingVertexCount, &bone->indices[0], &bone->weights[0]));

		// debugging
		if (i == 0)
		{
			bone->indices.resize(vertCount);
			bone->weights.resize(vertCount);
			for (j = (int)bone->infVertexCount; j < vertCount; j++)
			{
				bone->indices[j] = bone->indices[bone->infVertexCount-1];
				bone->weights[j] = bone->weights[bone->infVertexCount-1];

				//bone->indices[j] = 0xffff;
				//bone->weights[j] = 0.0f;
			}
			V_OKAY(lpSkinInfo->SetBoneInfluence(i, vertCount, &bone->indices[0], &bone->weights[0]));
			bone->indices.resize(bone->infVertexCount);
			bone->weights.resize(bone->infVertexCount);
		}
		else
		{
			V_OKAY(lpSkinInfo->SetBoneInfluence(i, (DWORD)bone->infVertexCount, &bone->indices[0], &bone->weights[0]));
		}


	}
	DWORD maxVertexInfluence, numBoneCombinations;
	LPD3DXBUFFER lpBoneCombinations = 0;
	LPD3DXMESH lpSkinnedMesh = 0;

	LPD3DXMESH lpTempMesh = this->lpMeshes[meshIndex];
	HRESULT hr;

	D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
	ZeroMemory(declaration, sizeof(declaration));


	DWORD* pNewAdjacency = new DWORD[lpTempMesh->GetNumFaces() * 3];
	if( 0 == pNewAdjacency )
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}
	hr = lpTempMesh->GenerateAdjacency( 1e-4f, pNewAdjacency );
	if( FAILED(hr) )
		goto e_Exit;

	//V_OKAY(lpTempMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, pNewAdjacency, 0, 0, 0));


	hr = lpSkinInfo->ConvertToIndexedBlendedMesh(
		lpTempMesh,
		D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE | D3DXMESH_MANAGED, // D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE |
		(DWORD)boneCount,
		pNewAdjacency,
		0,
		0,
		0,
		&maxVertexInfluence,
		&numBoneCombinations,
		&lpBoneCombinations,
		&lpSkinnedMesh
		);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Convert failed"), _T("Error"), MB_OK | MB_ICONERROR);
		goto e_Exit;
	}
	//D3DXBONECOMBINATION* boneComb = (D3DXBONECOMBINATION*)lpBoneCombinations->GetBufferPointer();

	this->lpSkinnedMeshes.push_back(lpSkinnedMesh);
	this->lpSkinnedMeshesSkinInfo.push_back(lpSkinInfo);
	lpSkinInfo->AddRef();

	hr = S_OK;


	// Debug skinned mesh declaration here...

	//int numBytesPerVertex = lpTempMesh->GetNumBytesPerVertex();
	//int numBytesPerVertexSkinned = lpSkinnedMesh->GetNumBytesPerVertex();

	D3DVERTEXELEMENT9 declMesh[MAX_FVF_DECL_SIZE];
	ZeroMemory(declMesh, sizeof(declMesh));
	lpTempMesh->GetDeclaration(declMesh);

	D3DVERTEXELEMENT9 declMeshSkinned[MAX_FVF_DECL_SIZE];
	ZeroMemory(declMeshSkinned, sizeof(declMeshSkinned));
	lpSkinnedMesh->GetDeclaration(declMeshSkinned);


	//
	// Debug Testing... (Change specific vertex value)
	//

	//LPDIRECT3DVERTEXBUFFER9 lpTempVB = 0;
	//lpSkinnedMesh->GetVertexBuffer(&lpTempVB);
	//struct SKINNED_MESH_VERT
	//{
	//	float x, y, z;
	//	float w0, w1, w2;
	//	unsigned char i0, i1, i2, i3;
	//	float nx, ny, nz;
	//	float color;
	//	float u, v;
	//};
	//SKINNED_MESH_VERT* skinnedVert = 0;
	//lpTempVB->Lock(0, 0, (void**)&skinnedVert, 0);

	//skinnedVert[85].x = -50.0f;
	//
	////skinnedVert[85].w0 = 0.1f;
	////skinnedVert[85].w1 = 0.2f;
	////skinnedVert[85].w2 = 0.7f;

	//lpTempVB->Unlock();
	//SAFE_RELEASE(lpTempVB);

e_Exit:
	SAFE_RELEASE(lpBoneCombinations);
	SAFE_RELEASE(lpSkinInfo);
	SAFE_DELETE_ARRAY(pNewAdjacency);
	return hr;
}


LPD3DXMESH ModelDrawerDx9::GetSkinnedMeshPointer(int i) const
{
	return this->lpSkinnedMeshes[i];
}
LPD3DXSKININFO ModelDrawerDx9::GetSkinInfoPointer(int i) const
{
	return this->lpSkinnedMeshesSkinInfo[i];
}




HRESULT ModelDrawerDx9::BuildKeyframedAnimationSetOfSkeletonNodeIndex( int skeletonNodeIndex, int keyframeStartIndex, int keyframeEndIndex )
{

	ASSERTCHECK( keyframeStartIndex >= 0 );

	SkeletonNode* currentSkeleton = m_reader->GetSkeletonNodePointer(skeletonNodeIndex);
	size_t s;
	HRESULT hr;
	const int ticksPerSecond = 1;

	int totalBoneCount = 0;
	for (s = 0; s < m_reader->GetSkeletonNodeSize(); s++)
	{
		totalBoneCount += (int)m_reader->GetSkeletonNodePointer(s)->bones.size();
	}

	//
	// Create animation set if 0
	// (this animation set is dedicated to keyframed animation of bone structure)
	//
	LPD3DXKEYFRAMEDANIMATIONSET lpKfAnimSet = 0; // temp animset
	if (this->lpKeyframedAnimationSet == 0)
	{
		/*std::tstring animationSetName(_T("Default Animation Set Of "));
		animationSetName = animationSetName + this->szFileName;*/
		STRING animSetName = currentSkeleton->associatedMeshName;
		animSetName += "-Animation";

		static char animSetNameChar[128];
		sprintf_s( animSetNameChar, 128, "%s-Animation-%d", currentSkeleton->associatedMeshName, (int)this->lpKeyframedAnimationSetList.size() );

		hr = D3DXCreateKeyframedAnimationSet(animSetNameChar, ticksPerSecond, D3DXPLAY_LOOP, totalBoneCount, 0, 0, &lpKfAnimSet);
		if (hr != D3D_OK)
			return hr;
	}

	int entireKeyframesCount = (int)currentSkeleton->bones[0].keys.size(); // assume all bones have the same amount of keyframes...
	int registeringKeyframesCount = entireKeyframesCount;

	ASSERTCHECK( entireKeyframesCount >= keyframeEndIndex );
	if ( keyframeStartIndex != 0 || keyframeEndIndex != -1 )
	{
		// called by explicit animset frame range
		registeringKeyframesCount = keyframeEndIndex - keyframeStartIndex + 1;
	}
	else
	{
		// default animset frame range: entire
		keyframeEndIndex = registeringKeyframesCount - 1;
	}
	ASSERTCHECK( keyframeEndIndex >= keyframeStartIndex);



	TCHAR debugString[512];
	_stprintf_s( debugString, TCHARSIZE(debugString), _T(" - Animation Set build by keyframe range: %d to %d\n"), keyframeStartIndex, keyframeEndIndex );
	OutputDebugString( debugString );
	if (lpKfAnimSet == 0)
		throw new std::runtime_error("Animation set is 0");

	for (s = 0; s < currentSkeleton->bones.size(); s++)
	{
		Bone* currentBone = &currentSkeleton->bones[s];

		ModelReader::AllocateAsAnimationSetFormat(registeringKeyframesCount, &currentBone->keys[keyframeStartIndex],
			&currentBone->scaleKeysSize, &currentBone->rotationKeysSize, &currentBone->translationKeysSize,
			&currentBone->scaleKeys, &currentBone->rotationKeys, &currentBone->translationKeys, TRUE);

		DWORD animIndex = 0;

		hr = lpKfAnimSet->RegisterAnimationSRTKeys(currentBone->nameFixed.c_str(),
			currentBone->scaleKeysSize, currentBone->rotationKeysSize, currentBone->translationKeysSize,
			currentBone->scaleKeys->getDxPtr(), currentBone->rotationKeys->getDxPtr(), currentBone->translationKeys->getDxPtr(),
			&animIndex);

		SAFE_DELETE_ARRAY(currentBone->scaleKeys);
		SAFE_DELETE_ARRAY(currentBone->translationKeys);
		SAFE_DELETE_ARRAY(currentBone->rotationKeys);


		_stprintf_s(debugString, TCHARSIZE(debugString), _T("    - Registered animation key index: %d\n"), animIndex);
		OutputDebugString(debugString);


		if (hr != D3D_OK)
			return hr;
	}

	// animset has all matrices corresponding to each bone transformation
	this->lpKeyframedAnimationSetList.push_back( lpKfAnimSet );


	return 0;
}


void ModelDrawerDx9::UpdateBoneCombinedMatrixByMeshIndex(int meshIndex)
{
	MyFrame* frameRoot = this->GetFrameRootByMeshIndex(meshIndex);

	ArnMatrix identity;
	D3DXMatrixIdentity(identity.getDxPtr());
	UpdateBoneCombinedMatrixRecursive(frameRoot, identity);
}
void ModelDrawerDx9::UpdateBoneCombinedMatrixRecursive(MyFrame* startFrame, ArnMatrix& parentCombinedTransform)
{
	startFrame->combinedMatrix = startFrame->TransformationMatrix * parentCombinedTransform;

	if (startFrame->pFrameSibling != 0)
		this->UpdateBoneCombinedMatrixRecursive((MyFrame*)startFrame->pFrameSibling, parentCombinedTransform);

	if (startFrame->pFrameFirstChild != 0)
		this->UpdateBoneCombinedMatrixRecursive((MyFrame*)startFrame->pFrameFirstChild, startFrame->combinedMatrix);
}
LPD3DXMESH ModelDrawerDx9::GetMeshPointer(int i) const
{
	return this->lpMeshes[i];
}


LPDIRECT3DTEXTURE9 ModelDrawerDx9::GetTexture(int referenceIndex) const
{
	if ( this->textureList.size() > (size_t)referenceIndex )
		return this->textureList[referenceIndex];
	else
		return 0;
}
#endif
