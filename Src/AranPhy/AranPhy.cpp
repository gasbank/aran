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

static void
ArnxGeneralBodyPlaneIntersectionSub(std::list<ArnVec3>& points, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& plane)
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
ArnGeneralBodyPlaneIntersection(std::list<ArnVec3>& points, const GeneralBody& gb, const ArnPlane& plane)
{
	assert(gb.getBoundingBoxType() == ABBT_BOX);
	dVector3 bbSize;
	gb.getGeomSize(bbSize);
	assert(bbSize[0] && bbSize[1] && bbSize[2]);
	float x = bbSize[0] / 2;
	float y = bbSize[1] / 2;
	float z = bbSize[2] / 2;

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
		ArnQuat q(gb.getQuaternion()[1], gb.getQuaternion()[2], gb.getQuaternion()[3], gb.getQuaternion()[0]);
		q.getRotationMatrix(&rotMat);
		ArnVec3 tp;
		ArnVec3TransformCoord(&tp, &p[i], &rotMat);
		p[i] = tp;

		p[i].x += gb.getPosition()[0];
		p[i].y += gb.getPosition()[1];
		p[i].z += gb.getPosition()[2];
	}

	ArnxGeneralBodyPlaneIntersectionSub(points, p[0], p[1], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[4], p[5], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[6], p[7], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[2], p[3], plane);

	ArnxGeneralBodyPlaneIntersectionSub(points, p[0], p[2], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[4], p[6], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[5], p[7], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[1], p[3], plane);

	ArnxGeneralBodyPlaneIntersectionSub(points, p[0], p[4], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[2], p[6], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[3], p[7], plane);
	ArnxGeneralBodyPlaneIntersectionSub(points, p[1], p[5], plane);
}
