#pragma once

class ArnMatrix;
class ArnMesh;


// Aran library compartment for LPD3DXSKININFO
class ArnSkinInfo
{
public:
	ArnSkinInfo(void);
	~ArnSkinInfo(void);

	HRESULT SetBoneName(DWORD boneIndex, const char* boneName) { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	HRESULT SetBoneOffsetMatrix(DWORD boneIndex, const ArnMatrix* mat) { ARN_THROW_NOT_IMPLEMENTED_ERROR }

	HRESULT SetBoneInfluence(DWORD boneIndex, DWORD numInfluences, const DWORD* boneVertices, const float* boneWeights) { ARN_THROW_NOT_IMPLEMENTED_ERROR }

	HRESULT ConvertToIndexedBlendedMesh(
		LPD3DXMESH pMesh,
		DWORD Options,
		DWORD paletteSize,
		CONST DWORD* pAdjacencyIn,
		DWORD* pAdjacencyOut,
		DWORD* pFaceRemap,
		void* ppVertexRemap,
		DWORD* pMaxVertexInfl,
		DWORD* pNumBoneCombinations,
		void* ppBoneCombinationTable,
		LPD3DXMESH* ppMesh) { ARN_THROW_NOT_IMPLEMENTED_ERROR }

	int GetNumBones() const { ARN_THROW_NOT_IMPLEMENTED_ERROR }
};
