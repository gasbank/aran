#include "AranDx9PCH.h"
#include "ArnSkinInfo.h"

ArnSkinInfo::ArnSkinInfo(void)
{
}

ArnSkinInfo::~ArnSkinInfo(void)
{
}

HRESULT ConvertToIndexedBlendedMesh( ArnMesh* pMesh, DWORD Options, DWORD paletteSize,
									CONST DWORD *pAdjacencyIn, DWORD* pAdjacencyOut,
									DWORD* pFaceRemap, void* ppVertexRemap, DWORD* pMaxVertexInfl,
									DWORD* pNumBoneCombinations, void* ppBoneCombinationTable, ArnMesh* ppMesh )
{
	return S_OK;
}
