#include "AranPhyPCH.h"
#include "UniversalJoint.h"

UniversalJoint::UniversalJoint(const OdeSpaceContext* osc)
: AngularJoint(osc)
{
	//ctor
	setId(dJointCreateUniversal(osc->world, 0 /* Joint group ID */));
}

UniversalJoint::~UniversalJoint()
{
	//dtor
}

void UniversalJoint::doReset()
{
	setParamVelocity(1, 0);
	setParamVelocity(2, 0);
	setParamFMax(1, 0);
	setParamFMax(2, 0);
}


void UniversalJoint::renderJointAxis() const
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
	/*const double scale = 0.25;
	dVector3 anchor, axis1, axis2;
	getAnchor(anchor);
	getAxis(1, axis1);
	getAxis(2, axis2);

	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);

	glBegin(GL_LINES);
	glVertex3dv(anchor);
	glVertex3d(anchor[0] + scale * axis1[0], anchor[1] + scale * axis1[1], anchor[2] + scale * axis1[2]);
	glEnd();

	glBegin(GL_LINES);
	glVertex3dv(anchor);
	glVertex3d(anchor[0] + scale * axis2[0], anchor[1] + scale * axis2[1], anchor[2] + scale * axis2[2]);
	glEnd();

	glEnable(GL_LIGHTING);*/
}

void UniversalJoint::getAxis(int anum, dVector3 result) const
{
	switch(anum)
	{
		case 1:	dJointGetUniversalAxis1(getId(), result); break;
		case 2:	dJointGetUniversalAxis2(getId(), result); break;
		default: dAASSERT(false); break;
	}
}

void UniversalJoint::setParamLoHiStop(int anum, dReal lo, dReal hi)
{
	double cur = getValue(anum);
	if (anum == 1)
	{
		if (cur >= lo && cur <= hi)
		{
			dJointSetUniversalParam(getId(), dParamLoStop, lo);
			dJointSetUniversalParam(getId(), dParamHiStop, hi);
		}
		else
		{
			fprintf(stderr, "New LoHi params must include the current value of joints.\n");
			abort();
		}
	}
	else if (anum == 2)
	{
		if (cur >= lo && cur <= hi)
		{
			dJointSetUniversalParam(getId(), dParamLoStop2, lo);
			dJointSetUniversalParam(getId(), dParamHiStop2, hi);
		}
		else
		{
			fprintf(stderr, "New LoHi params must include the current value of joints.\n");
			abort();
		}
	}
	else assert(!"Invalid axis number.");
}

void UniversalJoint::setAnchor( const ArnVec3& pos )
{
	dJointSetUniversalAnchor(getId(), pos.x, pos.y, pos.z);
}

void UniversalJoint::getAnchor( dVector3 anchor ) const
{
	dJointGetUniversalAnchor(getId(), anchor);
}

void UniversalJoint::setAxis( int anum, const ArnVec3& axis )
{
	if (anum == 1) dJointSetUniversalAxis1(getId(), axis.x, axis.y, axis.z);
	else if (anum == 2) dJointSetUniversalAxis2(getId(), axis.x, axis.y, axis.z);
	else assert(!"Invalid axis number.");
}

dReal UniversalJoint::getAngle( int anum ) const
{
	if (anum == 1) return dJointGetUniversalAngle1(getId());
	else if (anum == 2) return dJointGetUniversalAngle2(getId());
	else assert(!"Invalid axis number.");
	return 0; // not reach here
}

double UniversalJoint::getVelocity( int anum ) const
{
	if (anum == 1) return dJointGetUniversalAngle1Rate(getId());
	else if (anum == 2) return dJointGetUniversalAngle2Rate(getId());
	else assert(!"Invalid axis number.");
	return 0; // not reach here
}

void UniversalJoint::controlAddTorque( int anum, double torque )
{
	if (anum == 1) dJointAddUniversalTorques(getId(), torque, 0);
	else if (anum == 2) dJointAddUniversalTorques(getId(), 0, torque);
}

void UniversalJoint::setParamVelocity( int anum, dReal v )
{
	if (anum == 1) dJointSetUniversalParam(getId(), dParamVel1, v);
	else if (anum == 2) dJointSetUniversalParam(getId(), dParamVel2, v);
	else assert(!"Invalid axis number.");
}

void UniversalJoint::setParamFMax( int anum, dReal v )
{
	if (anum == 1) dJointSetUniversalParam(getId(), dParamFMax1, v);
	else if (anum == 2) dJointSetUniversalParam(getId(), dParamFMax2, v);
	else assert(!"Invalid axis number.");
}