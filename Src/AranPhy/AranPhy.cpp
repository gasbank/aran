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
ArnGeneralBodyPlaneIntersection(std::vector<ArnVec3>& points, const GeneralBody& gb, const ArnPlane& plane)
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

void
ArnGeneralBodyVerticalLineIntersection( std::vector<ArnVec3>& points, const GeneralBody& gb, const float x, const float y)
{
	if (gb.getBoundingBoxType() == ABBT_BOX)
	{
		dVector3 bbSize;
		gb.getGeomSize(bbSize);
		assert(bbSize[0] && bbSize[1] && bbSize[2]);
		ArnVec3 pos(gb.getPosition()[0], gb.getPosition()[1], gb.getPosition()[2]);
		ArnQuat q(gb.getQuaternion()[1], gb.getQuaternion()[2], gb.getQuaternion()[3], gb.getQuaternion()[0]);
		ArnXformedBoxVerticalLineIntersection(points, ArnVec3(bbSize[0], bbSize[1], bbSize[2]), pos, q, x, y);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}