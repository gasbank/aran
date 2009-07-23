#include "AranMathPCH.h"
#include "Decompose.h"
#include "ArnQuat.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnMatrix.h"
#include "ArnColorValue4f.h"
#include "ArnViewportData.h"
#include "ArnMath.h"

// Floating Point Library Specific
static const float	EPSILON						= 0.0001f;		// error tolerance for check
static const int	FLOAT_DECIMAL_TOLERANCE		= 3;			// decimal places for float rounding

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

DWORD
ArnFloat4ColorToDword( const ArnColorValue4f* cv )
{
	return ((int)(0) << 24)
		| ((int)(cv->b * 255) << 16)
		| ((int)(cv->g * 255) <<  8)
		| ((int)(cv->r * 255) <<  0);
}

ArnQuat
ArnEulerToQuat( const ArnVec3* vec3 )
{
	typedef cml::quaternion<float, cml::fixed<>, cml::scalar_first, cml::positive_cross> quaternion_type;
	quaternion_type cmlq;
	quaternion_rotation_euler(cmlq, vec3->x, vec3->y, vec3->z, cml::euler_order_xyz);
	return ArnQuat(cmlq[1], cmlq[2], cmlq[3], cmlq[0]);
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

HRESULT
ArnMatrixDecompose( ArnVec3* pOutScale, ArnQuat* pOutRotation, ArnVec3* pOutTranslation, const ArnMatrix* pM )
{
	HMatrix hm;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			#ifdef WIN32
			hm[i][j] = pM->m[j][i]; // D3D uses row-wise index on transform matrices
			#else
			hm[i][j] = pM->m[i][j]; // OpenGL uses column-wise index on transform matrices
			#endif
		}
	}
	AffineParts ap;
	decomp_affine(hm, &ap);
	pOutTranslation->x = ap.t.x;
	pOutTranslation->y = ap.t.y;
	pOutTranslation->z = ap.t.z;
	pOutRotation->x = ap.q.x;
	pOutRotation->y = ap.q.y;
	pOutRotation->z = ap.q.z;
	pOutRotation->w = ap.q.w;
	pOutScale->x = ap.k.x;
	pOutScale->y = ap.k.y;
	pOutScale->z = ap.k.z;
	return S_OK;
}

void
ArnVec3TransformNormal(ArnVec3* out, ArnVec3* vec, ArnMatrix* mat)
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

// Calculate inverse of matrix.  Inversion may fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
ArnMatrix*
ArnMatrixInverse( ArnMatrix *pOut, FLOAT *pDeterminant, CONST ArnMatrix *pM )
{
	assert(pDeterminant == 0);
	cml::matrix44d_c cmlmat;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			cmlmat(i, j) = pOut->m[i][j];
	cmlmat = cmlmat.inverse();
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pOut->m[i][j] = float(cmlmat(i, j));
	return pOut;
}

// Rotation about arbitrary axis.
ArnQuat*
ArnQuaternionRotationAxis( ArnQuat *pOut, CONST ArnVec3 *pV, FLOAT Angle )
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
ArnQuaternionRotationMatrix( ArnQuat *pOut, CONST ArnMatrix *pM)
{
	cml_mat44 cmlmat;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			cmlmat(i, j) = pM->m[i][j];
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
	typedef cml::matrix<double, cml::fixed<3,3>, cml::col_basis> matrix_d3;
	matrix_d3 cmlmat;
	typedef cml::quaternion<double, cml::fixed<>, cml::scalar_first, cml::positive_cross> quaternion_type;
	quaternion_type cmlq(quat->w, quat->x, quat->y, quat->z);
	matrix_rotation_quaternion(cmlmat, cmlq);
	ArnMatrixIdentity(mat);
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			mat->m[i][j] = cmlmat(i, j);
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
	cml::matrix_viewport(viewport, float(pViewport->X), float(pViewport->X + pViewport->Width), float(pViewport->Y), float(pViewport->Y + pViewport->Height), cml::z_clip_zero, float(pViewport->MinZ), float(pViewport->MaxZ));
	cml_vec3 projected(cml::project_point(model, view, proj, viewport, point));

	pOut->x = projected[0];
	pOut->y = projected[1];
	pOut->z = projected[2];
	return pOut;
}

ArnVec3*
ArnVec3Unproject( ArnVec3* pOut, const ArnVec3* pV, const ArnViewportData* pViewport, const ArnMatrix* pProjection, const ArnMatrix* pModelview )
{
	cml_mat44 modelview, proj, viewport;
	cml_vec3 point(pV->x, pV->y, pV->z); // Point in the screen-space to be unprojected to world-space.
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			modelview(i, j) = pModelview->m[i][j];
			proj(i, j) = pProjection->m[i][j];
		}
	}
	cml::matrix_viewport(viewport, float(pViewport->X), float(pViewport->X + pViewport->Width), float(pViewport->Y), float(pViewport->Y + pViewport->Height), cml::z_clip_zero, float(pViewport->MinZ), float(pViewport->MaxZ));
	cml_vec3 unprojected(cml::unproject_point(modelview, proj, viewport, point));

	pOut->x = unprojected[0];
	pOut->y = unprojected[1];
	pOut->z = unprojected[2];
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
			pOut->m[i][j] = cmlmat(i, j);
	return pOut;
}

ArnVec3*
ArnVec3TransformCoord( ArnVec3* pOut, const ArnVec3* pV, const ArnMatrix* pM )
{
	ArnVec4 v4(*pV, 1);
	ArnVec4 v4Out;
	v4Out = (*pM) * (v4);
	pOut->x = v4Out.x;
	pOut->y = v4Out.y;
	pOut->z = v4Out.z;
	return pOut;
}

ArnMatrix*
ArnMatrixOrthoLH( ArnMatrix* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf )
{
	cml_mat44 cmlmat;
	cml::matrix_orthographic_LH(cmlmat, -h/2, h/2, -w/2, w/2, zn, zf, cml::z_clip_zero);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pOut->m[i][j] = cmlmat(i, j);
	return pOut;
}


// Build a lookat matrix.
ArnMatrix* ArnMatrixLookAt( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp, bool rightHanded )
{
	cml_mat44 cmlmat;
	cml_vec3 eye(pEye->x, pEye->y, pEye->z);
	cml_vec3 target(pAt->x, pAt->y, pAt->z);
	cml_vec3 up(pUp->x, pUp->y, pUp->z);
	if (rightHanded)
		cml::matrix_look_at(cmlmat, eye, target, up, cml::right_handed);
	else
		cml::matrix_look_at(cmlmat, eye, target, up, cml::left_handed);
	for (int i = 0; i < 4; ++i) // row
		for (int j = 0; j < 4; ++j) // column
			pOut->m[i][j] = cmlmat(j, i);
	return pOut;
}
// Equivalent to D3DXMatrixLookAtRH()
ArnMatrix* ArnMatrixLookAtRH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp )
{
	return ArnMatrixLookAt(pOut, pEye, pAt, pUp, cml::right_handed);
}
// Equivalent to D3DXMatrixLookAtLH()
ArnMatrix*
ArnMatrixLookAtLH( ArnMatrix *pOut, CONST ArnVec3 *pEye, CONST ArnVec3 *pAt, CONST ArnVec3 *pUp )
{
	return ArnMatrixLookAt(pOut, pEye, pAt, pUp, cml::left_handed);
}

// Build a perspective projection matrix. (left-handed)
ArnMatrix*
ArnMatrixPerspectiveFovLH( ArnMatrix *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

void
ArnCmlMatToArnMat(ArnMatrix* out, const cml_mat44* cmlmat)
{
	for (int i = 0; i < 4; ++i) // Row
		for (int j = 0; j < 4; ++j) // Column
			out->m[i][j] = (*cmlmat)(j, i);
}


void ArnGetViewportMatrix(ArnMatrix* out, const ArnViewportData* pViewport)
{
	cml_mat44 viewport;
	cml::matrix_viewport(viewport, float(pViewport->X), float(pViewport->X + pViewport->Width), float(pViewport->Y), float(pViewport->Y + pViewport->Height), cml::z_clip_zero, float(pViewport->MinZ), float(pViewport->MaxZ));
	for (int i = 0; i < 4; ++i) // Row
		for (int j = 0; j < 4; ++j) // Column
			out->m[i][j] = viewport(j, i);
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
ArnMakePickRay(ArnVec3* origin, ArnVec3* direction, float scrX, float scrY, const ArnMatrix* view, const ArnMatrix* projection, const ArnMatrix* viewport)
{
	cml_mat44 cmlview, cmlproj, cmlviewport;
	for (int i = 0; i < 4; ++i) // Row
	{
		for (int j = 0; j < 4; ++j) // Column
		{
			cmlview(j, i) = view->m[i][j];
			cmlproj(j, i) = projection->m[i][j];
			cmlviewport(j, i) = viewport->m[i][j];
		}
	}
	cml_vec3 cmlorg, cmldir;
	cml::make_pick_ray(scrX, scrY, cmlview, cmlproj, cmlviewport, cmlorg, cmldir);
	origin->x = cmlorg[0];
	origin->y = cmlorg[1];
	origin->z = cmlorg[2];
	direction->x = cmldir[0];
	direction->y = cmldir[1];
	direction->z = cmldir[2];
}

void
ArnExtractFrustumPlanes(float planes[6][4], const ArnMatrix* modelview, const ArnMatrix* projection)
{
	cml_mat44 cmlmodelview, cmlproj;
	for (int i = 0; i < 4; ++i) // Row
	{
		for (int j = 0; j < 4; ++j) // Column
		{
			cmlmodelview(j, i) = modelview->m[i][j];
			cmlproj(j, i) = projection->m[i][j];
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

ArnMatrix*
ArnMatrixPerspectiveYFov(ArnMatrix* out, float yFov, float aspect, float nearClip, float farClip, bool rightHanded)
{
	cml_mat44 cmlout;
	cml::Handedness handedness;
	if (rightHanded)
		handedness = cml::right_handed;
	else
		handedness = cml::left_handed;

	cml::matrix_perspective_yfov(cmlout, yFov, aspect, nearClip, farClip, handedness, cml::z_clip_zero);
	ArnCmlMatToArnMat(out, &cmlout);
	return out;
}

