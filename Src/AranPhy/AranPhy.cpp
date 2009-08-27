#include "AranPhyPCH.h"
#include "AranPhy.h"
#include "GeneralBody.h"
#include "ArnPlane.h"
#include "ArnIntersection.h"

static bool gs_bAranPhyInitialized = false;

int
ArnInitializePhysics()
{
	if (gs_bAranPhyInitialized == false)
	{
		dInitODE();
		gs_bAranPhyInitialized = true;
		return 0;
	}
	else
	{
		// Already initialized.
		return -1;
	}
}

int
ArnCleanupPhysics()
{
	if (gs_bAranPhyInitialized)
	{
		dCloseODE();
		gs_bAranPhyInitialized = false;
		return 0;
	}
	else
	{
		// Not initialized, but cleanup called.
		return -1;
	}
}

void
ArnLineSegmentPlaneIntersection(std::list<ArnVec3>& points, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& plane)
{
	ArnVec3 isect;
	int isectCount;
	isectCount = intersect3D_SegmentPlane(isect, p0, p1, plane);
	if (isectCount == 1)
	{
		points.push_back(isect);
	}
	else if (isectCount == 2)
	{
		points.push_back(p0);
		points.push_back(p1);
	}
}

void
ArnBoxPlaneIntersection(std::list<ArnVec3>& points, const ArnVec3 p[8], const ArnPlane& plane)
{
	ArnLineSegmentPlaneIntersection(points, p[0], p[1], plane);
	ArnLineSegmentPlaneIntersection(points, p[4], p[5], plane);
	ArnLineSegmentPlaneIntersection(points, p[6], p[7], plane);
	ArnLineSegmentPlaneIntersection(points, p[2], p[3], plane);

	ArnLineSegmentPlaneIntersection(points, p[0], p[2], plane);
	ArnLineSegmentPlaneIntersection(points, p[4], p[6], plane);
	ArnLineSegmentPlaneIntersection(points, p[5], p[7], plane);
	ArnLineSegmentPlaneIntersection(points, p[1], p[3], plane);

	ArnLineSegmentPlaneIntersection(points, p[0], p[4], plane);
	ArnLineSegmentPlaneIntersection(points, p[2], p[6], plane);
	ArnLineSegmentPlaneIntersection(points, p[3], p[7], plane);
	ArnLineSegmentPlaneIntersection(points, p[1], p[5], plane);
}

void
ArnXformedBoxPlaneIntersection(std::list<ArnVec3>& points, const ArnVec3& boxSize, const ArnVec3& boxPos, const ArnQuat& q, const ArnPlane& plane)
{
	const float x = boxSize[0] / 2;
	const float y = boxSize[1] / 2;
	const float z = boxSize[2] / 2;

	ArnVec3 p[8];
	p[0].set(  x,  y,  z);
	p[1].set(  x,  y, -z);
	p[2].set(  x, -y,  z);
	p[3].set(  x, -y, -z);
	p[4].set( -x,  y,  z);
	p[5].set( -x,  y, -z);
	p[6].set( -x, -y,  z);
	p[7].set( -x, -y, -z);
	for (int i = 0; i < 8; ++i)
	{
		ArnMatrix rotMat;
		q.getRotationMatrix(&rotMat);
		ArnVec3 tp;
		ArnVec3TransformCoord(&tp, &p[i], &rotMat);
		p[i] = tp;
		p[i] += boxPos;
	}
	ArnBoxPlaneIntersection(points, p, plane);
}

void
ArnGeneralBodyPlaneIntersection(std::list<ArnVec3>& points, const GeneralBody& gb, const ArnPlane& plane)
{
	if (gb.getBoundingBoxType() == ABBT_BOX)
	{
		dVector3 bbSize;
		gb.getGeomSize(bbSize);
		assert(bbSize[0] && bbSize[1] && bbSize[2]);
		ArnVec3 pos(gb.getPosition()[0], gb.getPosition()[1], gb.getPosition()[2]);
		ArnQuat q(gb.getQuaternion()[1], gb.getQuaternion()[2], gb.getQuaternion()[3], gb.getQuaternion()[0]);
		ArnXformedBoxPlaneIntersection(points, ArnVec3(bbSize[0], bbSize[1], bbSize[2]), pos, q, plane);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}
