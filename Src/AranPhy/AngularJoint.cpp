#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "AngularJoint.h"

AngularJoint::AngularJoint(const OdeSpaceContext* osc)
: GeneralJoint(osc)
{
	//ctor
}

AngularJoint::~AngularJoint()
{
	//dtor
}

void AngularJoint::controlAddTorqueToTargetAngle(int anum, double torque, double angle)
{
	double angleDiff = angle - getAngle(anum);
	controlAddTorque(anum, torque * angleDiff);
}

void AngularJoint::render( const BasicObjects& bo ) const
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

//////////////////////////////////////////////////////////////////////////
