#include "AranPhyPCH.h"
#include "BallSocketJoint.h"

BallSocketJoint::BallSocketJoint(const OdeSpaceContext* osc)
: AngularJoint(osc)
{
	//ctor
	setId(dJointCreateBall(osc->world, 0 /* Joint group ID */));
}

BallSocketJoint::~BallSocketJoint()
{
	//dtor
}

void BallSocketJoint::setParamLoHiStop(int anum, dReal lo, dReal hi)
{
}

void BallSocketJoint::renderJointAxis() const
{
}

void BallSocketJoint::updateFrame()
{
}

void BallSocketJoint::doReset()
{
}
