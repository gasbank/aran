#ifndef UNIVERSALJOINT_H
#define UNIVERSALJOINT_H

#include "AngularJoint.h"

class UniversalJoint : public AngularJoint
{
public:
								UniversalJoint(const OdeSpaceContext* osc);
	virtual						~UniversalJoint();
	virtual const char*			getJointTypeName() const { return "Universal"; }
	virtual void				setAnchor(const ArnVec3& pos);
	virtual void				getAnchor(dVector3 anchor) const;
	virtual void				setAxis(int anum, const ArnVec3& axis);
	virtual void				getAxis(int anum, dVector3 result) const;
	virtual dReal				getAngle(int anum) const;
	virtual double				getVelocity(int anum) const;

	// Control
	virtual void				setParamLoHiStop(int anum, dReal lo, dReal hi);
	virtual void				controlAddTorque(int anum, double torque);

	// Joint parameters
	virtual void				setParamVelocity(int anum, dReal v);
	virtual void				setParamFMax(int anum, dReal v);

	// Rendering
	virtual void				renderJointAxis() const;
	virtual void				updateFrame();
protected:
	virtual void				doReset();
private:
};

inline void UniversalJoint::updateFrame()
{
	updateFrameInternal(1);
	updateFrameInternal(2);
}

#endif // UNIVERSALJOINT_H
