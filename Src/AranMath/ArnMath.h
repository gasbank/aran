#pragma once

class ArnVec3;
class ArnColorValue4f;
class ArnQuat;
class ArnMatrix;
class ArnMesh;
class ArnGenericBuffer;
struct ArnViewportData;
const static float COMPARE_EPSILON = 1e-4f;

ARANMATH_API ArnVec3			ArnQuatToEuler(const ArnQuat* quat);
ARANMATH_API ArnQuat			ArnEulerToQuat(const ArnVec3* vec3);
ARANMATH_API ArnVec3			ArnVec3RadToDeg(const ArnVec3* vec3);
ARANMATH_API float				ArnVec3Length(const ArnVec3* vec3);
ARANMATH_API DWORD				ArnFloat4ColorToDword(const ArnColorValue4f* cv);
ARANMATH_API void				ArnQuatToAxisAngle(ArnVec3* axis, float* angle, const ArnQuat* q);
ARANMATH_API ArnMatrix*			ArnMatrixTransformation(ArnMatrix* pOut, const ArnVec3* pScalingCenter,
								   const ArnQuat* pScalingRotation, const ArnVec3* pScaling,
								   const ArnVec3* pRotationCenter, const ArnQuat* pRotation,
								   const ArnVec3* pTranslation);
ARANMATH_API HRESULT				ArnMatrixDecompose( ArnVec3* pOutScale, ArnQuat* pOutRotation, ArnVec3* pOutTranslation, const ArnMatrix* pM );
ARANMATH_API void				ArnMatrixRotationQuaternion(ArnMatrix* mat, const ArnQuat* quat);
ARANMATH_API void				ArnVec3TransformNormal(ArnVec3* out, ArnVec3* vec, ArnMatrix* mat);

// Calculate inverse of matrix.  Inversion my fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
ARANMATH_API ArnMatrix*			ArnMatrixInverse( ArnMatrix *pOut, FLOAT *pDeterminant, CONST ArnMatrix *pM );

// Rotation about arbitrary axis.
ARANMATH_API ArnQuat*			ArnQuaternionRotationAxis( ArnQuat *pOut, CONST ArnVec3 *pV, FLOAT Angle );

// Build a quaternion from a rotation matrix.
ARANMATH_API ArnQuat*			ArnQuaternionRotationMatrix( ArnQuat *pOut, CONST ArnMatrix *pM);

// Build a lookat matrix. (left-handed)
ARANMATH_API ArnMatrix*			ArnMatrixLookAt( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp, bool rightHanded );
ARANMATH_API ArnMatrix*			ArnMatrixLookAtLH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp );
ARANMATH_API ArnMatrix*			ArnMatrixLookAtRH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp );

// Build a perspective projection matrix. (left-handed)
// TODO: Duplicated functions
ARANMATH_API ArnMatrix*			ArnMatrixPerspectiveFovLH( ArnMatrix *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );
ARANMATH_API ArnMatrix*			ArnMatrixPerspectiveYFov(ArnMatrix* out, float yFov, float aspect, float nearClip, float farClip, bool rightHanded);

// Project vector from object space into screen space
ARANMATH_API ArnVec3*			ArnVec3Project( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport,
									const ArnMatrix* pProjection, const ArnMatrix* pView, const ArnMatrix* pWorld);
ARANMATH_API ArnVec3*			ArnVec3Unproject( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport, const ArnMatrix* pProjection, const ArnMatrix* pModelview );


ARANMATH_API void				ArnVec3Normalize(ArnVec3* out, const ArnVec3* v3);
ARANMATH_API float				ArnVec3NormalizeSelf( ArnVec3* inout );

ARANMATH_API ArnMatrix*			ArnMatrixRotationAxis(ArnMatrix* pOut, const ArnVec3* pV, float Angle );

// Transform (x, y, z, 1) by matrix, project result back into w=1.
ARANMATH_API ArnVec3*			ArnVec3TransformCoord( ArnVec3* pOut, const ArnVec3* pV, const ArnMatrix* pM );

// Build an ortho projection matrix. (left-handed)
ARANMATH_API ArnMatrix*			ArnMatrixOrthoLH( ArnMatrix* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

ARANMATH_API void ArnGetViewportMatrix(ArnMatrix* out, const ArnViewportData* pViewport);
ARANMATH_API void ArnExtractFrustumPlanes(float planes[6][4], const ArnMatrix* modelview, const ArnMatrix* projection);
ARANMATH_API void ArnGetFrustumCorners(ArnVec3 corners[8], float planes[6][4]);

template<typename V1, typename V2, typename V3> V1* ArnVec3Lerp( V1* pOut, const V2* pV1, const V3* pV2, float s )
{
	pOut->x = pV1->x + s * (pV2->x - pV1->x);
	pOut->y = pV1->y + s * (pV2->y - pV1->y);
	pOut->z = pV1->z + s * (pV2->z - pV1->z);
	return pOut;
}

ARANMATH_API int ArnIntersectTriangle(float* t, float* u, float* v, const ArnVec3* orig, const ArnVec3* dir, const ArnVec3 verts[3]);
ARANMATH_API void ArnMakePickRay(ArnVec3* origin, ArnVec3* direction, float scrX, float scrY, const ArnMatrix* view, const ArnMatrix* projection, const ArnMatrix* viewport);

#ifdef ARANMATH_EXPORTS
void ArnCmlMatToArnMat(ArnMatrix* out, const cml_mat44* cmlout);
#endif

#include "ArnMath.inl"
