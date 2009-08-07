#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "BallSocketJoint.h"

BallSocketJoint::BallSocketJoint(const OdeSpaceContext* osc)
: AngularJoint(osc)
, m_amotorId(0)
{
}

BallSocketJoint::~BallSocketJoint()
{
	dJointDestroy(m_amotorId);
}

BallSocketJoint*
BallSocketJoint::createFrom( const OdeSpaceContext* osc, const char* name, const GeneralBodyPtr& b1, const GeneralBodyPtr& b2, const ArnVec3& anchor, const ArnVec3& xAxis, const ArnVec3& yAxis, const ArnVec3& zAxis)
{
	BallSocketJoint* bsj = new BallSocketJoint(osc);
	bsj->setBody1(b1);
	bsj->setBody2(b2);
	bsj->setName(name);
	bsj->setAnchor(anchor);
	bsj->m_xAxis = xAxis;
	bsj->m_yAxis = yAxis;
	bsj->m_zAxis = zAxis;
	return bsj;
}

void
BallSocketJoint::setParamLoHiStop(int anum, dReal lo, dReal hi)
{
	int loStopEnum = 0;
	int hiStopEnum = 0;
	if (anum == 1) // X-axis
	{
		loStopEnum = dParamLoStop;
		hiStopEnum = dParamHiStop;
	}
	else if (anum == 2)
	{
		loStopEnum = dParamLoStop2;
		hiStopEnum = dParamHiStop2;
	}
	else if (anum == 3)
	{
		loStopEnum = dParamLoStop3;
		hiStopEnum = dParamHiStop3;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	dJointSetBallParam(getId(), loStopEnum, lo);
	dJointSetBallParam(getId(), hiStopEnum, hi);
	dJointSetAMotorParam(m_amotorId, loStopEnum, lo);
	dJointSetAMotorParam(m_amotorId, hiStopEnum, hi);
}

void
BallSocketJoint::renderJointAxis() const
{
}

void
BallSocketJoint::updateFrame()
{
}

void
BallSocketJoint::doReset()
{
}

void
BallSocketJoint::setAxis( int anum, const ArnVec3& axis )
{
	// No axis in ball-socket joint
	assert(!"Error!");
}

void
BallSocketJoint::getAxis( int anum, dVector3 result ) const
{
	assert(!"Error!");
}

dReal
BallSocketJoint::getAngle( int anum ) const
{
	if (anum == 1) return dJointGetUniversalAngle1(getId());
	else if (anum == 2) return dJointGetUniversalAngle2(getId());
	else assert(!"Invalid axis number.");
	return 0;
}

dReal
BallSocketJoint::getVelocity( int anum ) const
{
	if (anum == 1) return dJointGetUniversalAngle1Rate(getId());
	else if (anum == 2) return dJointGetUniversalAngle2Rate(getId());
	else assert(!"Invalid axis number.");
	return 0;
}

void
BallSocketJoint::controlAddTorque( int anum, double torque )
{
	if (anum == 1) dJointAddUniversalTorques(getId(), torque, 0);
	else if (anum == 2) dJointAddUniversalTorques(getId(), 0, torque);
}

void
BallSocketJoint::setParamVelocity( int anum, dReal v )
{
	if (anum == 1) dJointSetUniversalParam(getId(), dParamVel1, v);
	else if (anum == 2) dJointSetUniversalParam(getId(), dParamVel2, v);
	else assert(!"Invalid axis number.");
}

void
BallSocketJoint::setParamFMax( int anum, dReal v )
{
	if (anum == 1) dJointSetUniversalParam(getId(), dParamFMax1, v);
	else if (anum == 2) dJointSetUniversalParam(getId(), dParamFMax2, v);
	else assert(!"Invalid axis number.");
}

void BallSocketJoint::attach( GeneralBody& body1, GeneralBody& body2 )
{
	GeneralJoint::attach(body1, body2);
	dJointAttach(m_amotorId, body1.getBodyId(), body2.getBodyId());
}

void BallSocketJoint::configureOdeContext( const OdeSpaceContext* osc )
{
	assert(!getId() && !m_amotorId);
	assert(getBody1()->getBodyId() && getBody2()->getBodyId());
	setId(dJointCreateBall(osc->world, 0));
	dJointAttach(getId(), getBody1()->getBodyId(), getBody2()->getBodyId());
	dJointSetBallAnchor(getId(), getAnchor().x, getAnchor().y, getAnchor().z);
	m_amotorId = dJointCreateAMotor(osc->world, 0);
	dJointAttach(m_amotorId, getBody1()->getBodyId(), getBody2()->getBodyId());
	dJointSetAMotorMode(m_amotorId, dAMotorEuler);
	dJointSetAMotorNumAxes(m_amotorId, 3);
	dJointSetAMotorAxis(m_amotorId, 0 /* X-AXIS */, 1 /* Anchored to the first body */, m_xAxis.x, m_xAxis.y, m_xAxis.z);
	//dJointSetAMotorAxis(m_amotorId, 1 /* Y-AXIS */, 1 /* Anchored to the first body */, m_yAxis.x, m_yAxis.y, m_yAxis.z);
	dJointSetAMotorAxis(m_amotorId, 2 /* Z-AXIS */, 2 /* Anchored to the second body */, m_zAxis.x, m_zAxis.y, m_zAxis.z);
	
	//dJointSetAMotorAngle(m_amotorId, 0 /* X-AXIS */, 0);
	//dJointSetAMotorAngle(m_amotorId, 1 /* Y-AXIS */, 0);
	//dJointSetAMotorAngle(m_amotorId, 2 /* Z-AXIS */, 0);

}
