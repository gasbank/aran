#include "AranMathPCH.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnColorValue4f.h"
#include "ArnViewportData.h"
#include "ArnMath.h"

// Floating Point Library Specific
static const float	EPSILON						= 0.0001f;		// error tolerance for check
static const int	FLOAT_DECIMAL_TOLERANCE		= 3;			// decimal places for float rounding

static void ArnCmlMatToArnMat(ArnMatrix* out, const cml_mat44* cmlout);

ArnVec3
ArnQuatToEuler( const ArnQuat* quat )
{
	cml_quat q(quat->w, quat->x, quat->y, quat->z);
	double rx, ry, rz;
	quaternion_to_euler(q, rx, ry, rz, cml::euler_order_xyz);
	return ArnVec3(float(rx), float(ry), float(rz));
}

ArnVec3
ArnVec3RadToDeg( const ArnVec3* vec3 )
{
	return ArnVec3((float)ArnToDegree(vec3->x), (float)ArnToDegree(vec3->y), (float)ArnToDegree(vec3->z));
}

ArnQuat
ArnEulerToQuat( const ArnVec3* vec3 )
{
	cml_quat cmlq;
	cml::quaternion_rotation_euler(cmlq, vec3->x, vec3->y, vec3->z, cml::euler_order_xyz);
	return ArnQuat(cmlq[1], cmlq[2], cmlq[3], cmlq[0]);
}

DWORD
ArnFloat4ColorToDword( const ArnColorValue4f* cv )
{
	return ((int)(0) << 24)
		| ((int)(cv->b * 255) << 16)
		| ((int)(cv->g * 255) <<  8)
		| ((int)(cv->r * 255) <<  0);
}

ArnMatrix*
ArnMatrixTransformation(
			ArnMatrix* pOut,
			const ArnVec3* pScalingCenter,
			const ArnQuat* pScalingRotation,
			const ArnVec3* pScaling,
			const ArnVec3* pRotationCenter,
			const ArnQuat* pRotation,
			const ArnVec3* pTranslation )
{
	assert(pOut);

	// These parameters are not supported yet.
	// You should set these values to 0.
	assert(pScalingCenter == 0);
	assert(pScalingRotation == 0);
	assert(pRotationCenter == 0);

	ArnMatrix mat;
	if (pTranslation)
	{
		ArnMatrixTranslation(&mat, pTranslation->x, pTranslation->y, pTranslation->z);
		*pOut = mat;
	}
	else
	{
		ArnMatrixIdentity(pOut);
	}

	if (pRotation)
	{
		ArnMatrixRotationQuaternion(&mat, pRotation);
		*pOut *= mat;
	}

	if (pScaling)
	{
		ArnMatrixScaling(&mat, pScaling->x, pScaling->y, pScaling->z);
		*pOut *= mat;
	}
	return pOut;
}

// The input transform is assumed to consist of a scaling, a rotation, and a translation,
// applied in the order scale->rotation->translation.
HRESULT
ArnMatrixDecompose( ArnVec3* pOutScale, ArnQuat* pOutRotation, ArnVec3* pOutTranslation, const ArnMatrix* pM )
{
	cml_mat44 cmlmat;
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			cmlmat.set_basis_element(j, i, pM->m[i][j]);
	cml_vec3 cmlTrans;
	cml_quat cmlRot;
	float sx, sy, sz;
	cml::matrix_decompose_SRT(cmlmat, sx, sy, sz, cmlRot, cmlTrans);
	pOutTranslation->x = cmlTrans[0];
	pOutTranslation->y = cmlTrans[1];
	pOutTranslation->z = cmlTrans[2];
	pOutRotation->w = cmlRot[0];
	pOutRotation->x = cmlRot[1];
	pOutRotation->y = cmlRot[2];
	pOutRotation->z = cmlRot[3];
	pOutScale->x = sx;
	pOutScale->y = sy;
	pOutScale->z = sz;
	return S_OK;
}

ArnVec3*
ArnVec3TransformNormal(ArnVec3* out, const ArnVec3* vec, const ArnMatrix* mat)
{
	out = out;
	vec = vec;
	mat = mat;
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

// Calculate inverse of matrix.  Inversion may fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
ArnMatrix*
ArnMatrixInverse( ArnMatrix *pOut, float *pDeterminant, const ArnMatrix *pM )
{
	assert(pDeterminant == 0);
	cml_mat44 cmlmat;
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			cmlmat.set_basis_element(j, i, pM->m[i][j]);
	cmlmat = cmlmat.inverse();
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			pOut->m[i][j] = float(cmlmat.basis_element(j, i));
	return pOut;
}

// Rotation about arbitrary axis.
ArnQuat*
ArnQuaternionRotationAxis( ArnQuat *pOut, const ArnVec3 *pV, float Angle )
{
	assert(ArnVec3GetLength(*pV) == 1);
	float s = sinf(Angle / 2);
	pOut->w = cos(Angle / 2);
	pOut->x = pV->x * s;
	pOut->y = pV->y * s;
	pOut->z = pV->z * s;
	return pOut;
}

// Build a quaternion from a rotation matrix.
ArnQuat*
ArnQuaternionRotationMatrix( ArnQuat *pOut, const ArnMatrix *pM)
{
	cml_mat44 cmlmat;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			cmlmat.set_basis_element(j, i, pM->m[i][j]);
	cml_quat cmlquat;
	quaternion_rotation_matrix(cmlquat, cmlmat);
	pOut->w = cmlquat[0];
	pOut->x = cmlquat[1];
	pOut->y = cmlquat[2];
	pOut->z = cmlquat[3];
	return pOut;
}


void
ArnQuatToAxisAngle(ArnVec3* axis, float* angle, const ArnQuat* q)
{
	cml_vec3 caxis;
	float cang;
	cml_quat cq(q->w, q->x, q->y, q->z);
	quaternion_to_axis_angle(cq, caxis, cang);
	axis->x = caxis[0];
	axis->y = caxis[1];
	axis->z = caxis[2];
	*angle = cang;
}

void
ArnMatrixRotationQuaternion(ArnMatrix* mat, const ArnQuat* quat)
{
	cml_mat44 cmlmat;
	cml_quat cmlq(quat->w, quat->x, quat->y, quat->z);
	cml::matrix_rotation_quaternion(cmlmat, cmlq);
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			mat->m[i][j] = cmlmat.basis_element(j, i);
}

float
ArnVec3Length( const ArnVec3* vec3 )
{
	return sqrtf(vec3->x * vec3->x + vec3->y * vec3->y + vec3->z * vec3->z);
}

ArnVec3*
ArnVec3Project( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport, const ArnMatrix* pProjection, const ArnMatrix* pView, const ArnMatrix* pWorld )
{
	cml_mat44 model, view, proj, viewport;
	cml_vec3 point(pV->x, pV->y, pV->z); // Point in the world-space to be projected to screen-space.
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			model(i, j) = pWorld->m[i][j];
			view(i, j) = pView->m[i][j];
			proj(i, j) = pProjection->m[i][j];
		}
	}
	/*
	The left-bottom corner of monitor is the origin.

	(+Y)
	A
	|
	|
	+------> (+X)
	0

	*/
	cml::matrix_viewport(viewport, float(pViewport->X), float(pViewport->X + pViewport->Width), float(pViewport->Y), float(pViewport->Y + pViewport->Height), cml::z_clip_neg_one, float(pViewport->MinZ), float(pViewport->MaxZ));
	cml_vec3 projected(cml::project_point(model, view, proj, viewport, point));

	pOut->x = projected[0];
	pOut->y = projected[1];
	pOut->z = projected[2];
	return pOut;
}

ArnVec3*
ArnVec3Unproject( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* avd, const ArnMatrix* pProjection, const ArnMatrix* pModelview )
{
	ArnVec3 in;
	in.x = (pV->x - avd->X) * 2 / (avd->X + avd->Width) - 1.0f;
	in.y = (pV->y - avd->Y) * 2 / (avd->Y + avd->Height) - 1.0f;
	in.z = 2 * pV->z - 1.0f;
	ArnMatrix A = *pProjection * *pModelview;
	ArnMatrixInverse(&A, 0, &A);
	ArnVec3TransformCoord(pOut, &in, &A);
	return pOut;
}

void
ArnVec3Normalize( ArnVec3* out, const ArnVec3* v3 )
{
	*out = *v3 / ArnVec3Length(v3);
}

float
ArnVec3NormalizeSelf( ArnVec3* inout )
{
	float len = ArnVec3Length(inout);
	*inout = *inout / len;
	return len;
}

ArnMatrix*
ArnMatrixRotationAxis( ArnMatrix* pOut, const ArnVec3* pV, float Angle )
{
	cml_vec3 cmlvec(pV->x, pV->y, pV->z);
	cml_mat44 cmlmat; // 4x4 double column-wise matrix
	cml::matrix_rotation_axis_angle(cmlmat, cmlvec, Angle);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pOut->m[i][j] = cmlmat.basis_element(j, i);
	return pOut;
}

ArnVec3*
ArnVec3TransformCoord( ArnVec3* pOut, const ArnVec3* pV, const ArnMatrix* pM )
{
	ArnVec4 v4(*pV, 1);
	ArnVec4 v4Out;
	v4Out = (*pM) * (v4);
	pOut->x = v4Out.x / v4Out.w;
	pOut->y = v4Out.y / v4Out.w;
	pOut->z = v4Out.z / v4Out.w;
	return pOut;
}

ArnMatrix*
ArnMatrixOrthoLH( ArnMatrix* pOut, float w, float h, float zn, float zf )
{
	cml_mat44 cmlmat;
	cml::matrix_orthographic_LH(cmlmat, -h/2, h/2, -w/2, w/2, zn, zf, cml::z_clip_neg_one);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pOut->m[i][j] = cmlmat.basis_element(j, i);
	return pOut;
}

// Build a lookat matrix.
ArnMatrix*
ArnMatrixLookAt( ArnMatrix *pOut, const ArnVec3 *pEye, const ArnVec3 *pAt, const ArnVec3 *pUp, bool rightHanded )
{
	cml_mat44 cmlmat;
	cml_vec3 eye(pEye->x, pEye->y, pEye->z);
	cml_vec3 target(pAt->x, pAt->y, pAt->z);
	cml_vec3 up(pUp->x, pUp->y, pUp->z);
	cml::Handedness handedness;
	if (rightHanded)
		handedness = cml::right_handed;
	else
		handedness = cml::left_handed;
	cml::matrix_look_at(cmlmat, eye, target, up, handedness);
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			pOut->m[i][j] = cmlmat.basis_element(j, i);
	return pOut;
}
// Equivalent to D3DXMatrixLookAtRH()
ArnMatrix*
ArnMatrixLookAtRH( ArnMatrix *pOut, const ArnVec3 *pEye, const ArnVec3 *pAt, const ArnVec3 *pUp )
{
	return ArnMatrixLookAt(pOut, pEye, pAt, pUp, true);
}

// Equivalent to D3DXMatrixLookAtLH()
ArnMatrix*
ArnMatrixLookAtLH( ArnMatrix *pOut, const ArnVec3 *pEye, const ArnVec3 *pAt, const ArnVec3 *pUp )
{
	return ArnMatrixLookAt(pOut, pEye, pAt, pUp, false);
}

// Build a perspective projection matrix. (left-handed)
ArnMatrix*
ArnMatrixPerspectiveFovLH( ArnMatrix *pOut, float fovy, float Aspect, float zn, float zf )
{
	return ArnMatrixPerspectiveYFov(pOut, fovy, Aspect, zn, zf, false);
}

ArnMatrix*
ArnMatrixPerspectiveYFov(ArnMatrix* out, float yFov, float aspect, float nearClip, float farClip, bool rightHanded)
{
	cml_mat44 cmlout;
	cml::Handedness handedness;
	if (rightHanded)
		handedness = cml::right_handed;
	else
		handedness = cml::left_handed;

	cml::matrix_perspective_yfov(cmlout, yFov, aspect, nearClip, farClip, handedness, cml::z_clip_neg_one);
	ArnCmlMatToArnMat(out, &cmlout);
	return out;
}

void
ArnCmlMatToArnMat(ArnMatrix* out, const cml_mat44* cmlmat)
{
	for (int i = 0; i < 4; ++i) // Row
		for (int j = 0; j < 4; ++j) // Column
			out->m[i][j] = cmlmat->basis_element(j, i);
}

void
ArnGetViewportMatrix(ArnMatrix* out, const ArnViewportData* pViewport)
{
	cml_mat44 viewport;
	cml::matrix_viewport(viewport, float(pViewport->X), float(pViewport->X + pViewport->Width), float(pViewport->Y), float(pViewport->Y + pViewport->Height), cml::z_clip_neg_one, float(pViewport->MinZ), float(pViewport->MaxZ));
	for (int i = 0; i < 4; ++i) // Row
		for (int j = 0; j < 4; ++j) // Column
			out->m[i][j] = viewport.basis_element(j, i);
}

#define TEST_CULL

int
ArnIntersectTriangle(float* t, float* u, float* v, const ArnVec3* orig, const ArnVec3* dir, const ArnVec3 verts[3])
{
	ArnVec3 edge1, edge2, tvec, pvec, qvec;
	float det, inv_det;
	edge1 = verts[1] - verts[0];
	edge2 = verts[2] - verts[0];
	pvec = ArnVec3GetCrossProduct(*dir, edge2);
	det = ArnVec3Dot(&edge1, &pvec);
#ifdef TEST_CULL
	if (det < EPSILON)
		return 0;
	tvec = *orig - verts[0];
	*u = ArnVec3Dot(&tvec, &pvec);
	if (*u < 0 || *u > det)
		return 0;
	qvec = ArnVec3GetCrossProduct(tvec, edge1);
	*v = ArnVec3Dot(dir, &qvec);
	if (*v < 0 || *u + *v > det)
		return 0;
	*t = ArnVec3Dot(&edge2, &qvec);
	inv_det = 1.0f / det;
	*t *= inv_det;
	*u *= inv_det;
	*v *= inv_det;
#else
	if (det > -EPSILON && det < EPSILON)
		return 0;
	inv_det = 1.0f / det;
	tvec = *orig - verts[0];
	*u = ArnVec3Dot(&tvec, &pvec) * inv_det;
	if (*u < 0 || *u > 1.0f)
		return 0;
	qvec = ArnVec3GetCrossProduct(tvec, edge1);
	*v = ArnVec3Dot(dir, &qvec) * inv_det;
	if (*v < 0 || *u + *v > 1.0f)
		return 0;
	*t = ArnVec3Dot(&edge2, &qvec) * inv_det;
#endif
	return 1;
}

void
ArnMakePickRay(ArnVec3* origin, ArnVec3* direction, float scrX, float scrY, const ArnMatrix* view, const ArnMatrix* projection, const ArnViewportData* avd)
{
	ArnVec3 scrNear(scrX, scrY, 0);
	ArnVec3 scrFar(scrX, scrY, 1.0f);
	ArnVec3Unproject(origin, &scrNear, avd, projection, view);
	ArnVec3Unproject(direction, &scrFar, avd, projection, view);
	*direction = *direction - *origin;
	ArnVec3NormalizeSelf(direction);
}

void
ArnExtractFrustumPlanes(float planes[6][4], const ArnMatrix* modelview, const ArnMatrix* projection)
{
	cml_mat44 cmlmodelview, cmlproj;
	for (int i = 0; i < 4; ++i) // Row
	{
		for (int j = 0; j < 4; ++j) // Column
		{
			cmlmodelview.set_basis_element(j, i, modelview->m[i][j]);
			cmlproj.set_basis_element(j, i, projection->m[i][j]);
		}
	}
	cml::extract_frustum_planes(cmlmodelview, cmlproj, planes, cml::z_clip_neg_one, false);
}

void
ArnGetFrustumCorners(ArnVec3 corners[8], float planes[6][4])
{
	cml_vec3 cmlcorners[8];
	cml::get_frustum_corners(planes, cmlcorners);
	for (int i = 0; i < 8; ++i)
	{
		corners[i].x = cmlcorners[i][0];
		corners[i].y = cmlcorners[i][1];
		corners[i].z = cmlcorners[i][2];
	}
}

ArnVec4*
ArnVec3Transform(ArnVec4* out, const ArnVec3* vec, const ArnMatrix* mat)
{
	out->x = mat->m[0][0] * vec->x + mat->m[0][1] * vec->y + mat->m[0][2] * vec->z + mat->m[0][3];
	out->y = mat->m[1][0] * vec->x + mat->m[1][1] * vec->y + mat->m[1][2] * vec->z + mat->m[1][3];
	out->z = mat->m[2][0] * vec->x + mat->m[2][1] * vec->y + mat->m[2][2] * vec->z + mat->m[2][3];
	out->w = mat->m[3][0] * vec->x + mat->m[3][1] * vec->y + mat->m[3][2] * vec->z + mat->m[3][3];
	return out;
}

