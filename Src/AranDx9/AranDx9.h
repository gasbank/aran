#pragma once

class VideoMan;

ARANDX9_API D3DXMATRIX				ArnGetD3DX1(const ArnMatrix* arnMat);
ARANDX9_API D3DXVECTOR3				ArnGetD3DX2(const ArnVec3* arnVec3);
ARANDX9_API D3DXQUATERNION			ArnGetD3DX3(const ArnQuat* arnQuat);
ARANDX9_API HRESULT					arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH& mesh);
ARANDX9_API HRESULT					arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH& mesh);
ARANDX9_API HRESULT					arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPDIRECT3DVERTEXBUFFER9& d3dvb, OUT LPDIRECT3DINDEXBUFFER9& d3dib);
ARANDX9_API HRESULT					ArnCreateMeshFVF(IN DWORD NumFaces, IN DWORD NumVertices, IN DWORD FVF, IN LPDIRECT3DDEVICE9 dev, OUT LPD3DXMESH* ppMesh);
ARANDX9_API const D3DXVECTOR3*		ArnVec3GetConstDxPtr(const ArnVec3& v);
ARANDX9_API D3DXVECTOR3*			ArnVec3GetDxPtr(ArnVec3& v);
ARANDX9_API const D3DMATERIAL9*		ArnMaterialGetConstDxPtr(const ArnMaterialData& amd);
ARANDX9_API D3DMATERIAL9*			ArnMaterialGetDxPtr(ArnMaterialData& amd);
ARANDX9_API const D3DXMATRIX*		ArnMatrixGetConstDxPtr(const ArnMatrix& mat);
ARANDX9_API D3DXMATRIX*				ArnMatrixGetDxPtr(ArnMatrix& mat);
ARANDX9_API void					ArnInitializeRenderableObjectsDx9(ArnNode* node);

inline void Convert(D3DXVECTOR3 &a, const ArnVec3 &b)
{
  a[0] = b[0];
  a[1] = b[1];
  a[2] = b[2];
}

inline void Convert(ArnMatrix &a, const D3DXMATRIX &b)
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      a.m[i][j] = b.m[i][j];
}

inline void Convert(D3DXMATRIX &a, const ArnMatrix &b)
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      a.m[i][j] = b.m[i][j];
}

inline D3DXMATRIX operator * (const D3DXMATRIX &a, const ArnMatrix &b)
{
  D3DXMATRIX b2;
  Convert(b2, b);
  return a * b2;
}
