#ifndef BALLSOCKETJOINT_H
#define BALLSOCKETJOINT_H

#include "AngularJoint.h"

class BallSocketJoint : public AngularJoint
{
public:
							BallSocketJoint(const OdeSpaceContext* osc);
	virtual					~BallSocketJoint();

	virtual const char*		getJointTypeName() const { return "BallSocket"; }

	virtual void			setAnchor(const ArnVec3& pos)
	{
		dJointSetBallAnchor(getId(), pos.x, pos.y, pos.z);
	}

	virtual void getAnchor(dVector3 anchor) const
	{
		dJointGetBallAnchor(getId(), anchor);
	}

	virtual void setAxis(int anum, const ArnVec3& axis)
	{
		// No axis in ball-socket joint
		assert(!"Error!");
	}

	virtual void getAxis(int anum, dVector3 result) const
	{
		assert(!"Error!");
	}

	virtual dReal getAngle(int anum) const
	{
		if (anum == 1) return dJointGetUniversalAngle1(getId());
		else if (anum == 2) return dJointGetUniversalAngle2(getId());
		else assert(!"Invalid axis number.");
		return 0;
	}
	virtual dReal getVelocity(int anum) const
	{
		if (anum == 1) return dJointGetUniversalAngle1Rate(getId());
		else if (anum == 2) return dJointGetUniversalAngle2Rate(getId());
		else assert(!"Invalid axis number.");
		return 0;
	}

	// Control
	virtual void setParamLoHiStop(int anum, dReal lo, dReal hi);

	virtual void controlAddTorque(int anum, double torque)
	{
		if (anum == 1) dJointAddUniversalTorques(getId(), torque, 0);
		else if (anum == 2) dJointAddUniversalTorques(getId(), 0, torque);
	}

	// Joint parameters
	virtual void setParamVelocity(int anum, dReal v)
	{
		if (anum == 1) dJointSetUniversalParam(getId(), dParamVel1, v);
		else if (anum == 2) dJointSetUniversalParam(getId(), dParamVel2, v);
		else assert(!"Invalid axis number.");
	}

	virtual void setParamFMax(int anum, dReal v)
	{
		if (anum == 1) dJointSetUniversalParam(getId(), dParamFMax1, v);
		else if (anum == 2) dJointSetUniversalParam(getId(), dParamFMax2, v);
		else assert(!"Invalid axis number.");
	}

	// Rendering
	virtual void renderJointAxis() const;

	virtual void updateFrame();
protected:
	virtual void doReset();
private:
protected:
private:
};

#endif // BALLSOCKETJOINT_H
