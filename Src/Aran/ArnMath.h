#pragma once

#include "ArnQuat.h"

const static float COMPARE_EPSILON = 1e-4f;

ArnVec3				ArnQuatToEuler(const ArnQuat* quat);
ArnQuat				ArnEulerToQuat(const ArnVec3* vec3);
ArnVec3				ArnVec3RadToDeg(const ArnVec3* vec3);
float				ArnVec3Length(const ArnVec3* vec3);
DWORD				ArnFloat4ColorToDword(const ArnColorValue4f* cv);
void				ArnQuatToAxisAngle(ArnVec3* axis, float* angle, const ArnQuat* q);
ArnMatrix*			ArnMatrixTransformation(ArnMatrix* pOut, const ArnVec3* pScalingCenter,
								   const ArnQuat* pScalingRotation, const ArnVec3* pScaling,
								   const ArnVec3* pRotationCenter, const ArnQuat* pRotation,
								   const ArnVec3* pTranslation);
HRESULT				ArnMatrixDecompose( ArnVec3* pOutScale, ArnQuat* pOutRotation, ArnVec3* pOutTranslation, const ArnMatrix* pM );
void				ArnMatrixRotationQuaternion(ArnMatrix* mat, const ArnQuat* quat);
void				ArnVec3TransformNormal(ArnVec3* out, ArnVec3* vec, ArnMatrix* mat);

// Calculate inverse of matrix.  Inversion my fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
ArnMatrix*			ArnMatrixInverse( ArnMatrix *pOut, FLOAT *pDeterminant, CONST ArnMatrix *pM );

// Rotation about arbitrary axis.
ArnQuat*			ArnQuaternionRotationAxis( ArnQuat *pOut, CONST ArnVec3 *pV, FLOAT Angle );

// Build a quaternion from a rotation matrix.
ArnQuat*			ArnQuaternionRotationMatrix( ArnQuat *pOut, CONST ArnMatrix *pM);

// Build a lookat matrix. (left-handed)
ArnMatrix*			ArnMatrixLookAt( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp, cml::Handedness handedness );
ArnMatrix*			ArnMatrixLookAtLH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp );
ArnMatrix*			ArnMatrixLookAtRH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp );

// Build a perspective projection matrix. (left-handed)
ArnMatrix*			ArnMatrixPerspectiveFovLH( ArnMatrix *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

// Project vector from object space into screen space
ArnVec3*			ArnVec3Project( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport,
									const ArnMatrix* pProjection, const ArnMatrix* pView, const ArnMatrix* pWorld);
ArnVec3*			ArnVec3Unproject( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport, const ArnMatrix* pProjection, const ArnMatrix* pModelview );


void				ArnVec3Normalize(ArnVec3* out, const ArnVec3* v3);

ArnMatrix*			ArnMatrixRotationAxis(ArnMatrix* pOut, const ArnVec3* pV, float Angle );

// Transform (x, y, z, 1) by matrix, project result back into w=1.
ArnVec3*			ArnVec3TransformCoord( ArnVec3* pOut, const ArnVec3* pV, const ArnMatrix* pM );

// Build an ortho projection matrix. (left-handed)
ArnMatrix*			ArnMatrixOrthoLH( ArnMatrix* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

void ArnGetViewportMatrix(ArnMatrix* out, const ArnViewportData* pViewport);
void ArnExtractFrustumPlanes(float planes[6][4], const ArnMatrix* modelview, const ArnMatrix* projection);
void ArnGetFrustumCorners(ArnVec3 corners[8], float planes[6][4]);

template<typename V1, typename V2, typename V3> V1* ArnVec3Lerp( V1* pOut, const V2* pV1, const V3* pV2, float s )
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	pOut->z = pV1->z + s * (pV2->z - pV1->z);
	return pOut;
}

void ArnCmlMatToArnMat(ArnMatrix* out, const cml_mat44* cmlout);

class ArnMesh;
class ArnGenericBuffer;

HRESULT ArnIntersectGl( ArnMesh* pMesh, const ArnVec3* pRayPos, const ArnVec3* pRayDir, bool* pHit, unsigned int* pFaceIndex, FLOAT* pU, FLOAT* pV, FLOAT* pDist, ArnGenericBuffer* ppAllHits, unsigned int* pCountOfHits );
int ArnIntersectTriangle(float* t, float* u, float* v, const ArnVec3* orig, const ArnVec3* dir, const ArnVec3 verts[3]);
void ArnMakePickRay(ArnVec3* origin, ArnVec3* direction, float scrX, float scrY, const ArnMatrix* view, const ArnMatrix* projection, const ArnMatrix* viewport);
#ifdef WIN32
HRESULT 
ArnIntersectDx9(
			 LPD3DXMESH pMesh,
			 const ArnVec3* pRayPos,
			 const ArnVec3* pRayDir, 
			 bool* pHit,              // True if any faces were intersected
			 DWORD* pFaceIndex,        // index of closest face intersected
			 FLOAT* pU,                // Barycentric Hit Coordinates    
			 FLOAT* pV,                // Barycentric Hit Coordinates
			 FLOAT* pDist,             // Ray-Intersection Parameter Distance
			 ArnGenericBuffer* ppAllHits,    // Array of D3DXINTERSECTINFOs for all hits (not just closest) 
			 DWORD* pCountOfHits);     // Number of entries in AllHits array
#endif

#include "ArnMath.inl"
