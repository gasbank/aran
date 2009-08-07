#ifndef HINGEJOINT_H
#define HINGEJOINT_H

#include "AngularJoint.h"

class HingeJoint : public AngularJoint
{
public:
							HingeJoint(const OdeSpaceContext* osc);
	virtual					~HingeJoint();
	virtual const char*		getJointTypeName() const { return "Hinge"; }
	virtual void			setAnchor(const ArnVec3& pos) { dJointSetHingeAnchor(getId(), pos.x, pos.y, pos.z); }
	virtual void			getAnchor(dVector3 anchor) const { dJointGetHingeAnchor(getId(), anchor); }
	virtual void			setAxis(int anum, const ArnVec3& axis);
	virtual void			getAxis(int anum, dVector3 result) const;
	virtual dReal			getAngle(int ) const { return dJointGetHingeAngle(getId()); }
	virtual double			getVelocity(int ) const { return dJointGetHingeAngleRate(getId()); }

	// Control
	virtual void			setParamVelocity(int , dReal v) { dJointSetHingeParam(getId(), dParamVel, v); }
	virtual void			setParamFMax(int , dReal v) { dJointSetHingeParam(getId(), dParamFMax, v); }
	virtual void			controlAddTorque(int , double torque) { dJointAddHingeTorque(getId(), torque); }

	virtual void			setParamLoHiStop(int anum, dReal lo, dReal hi);
	virtual void			renderJointAxis() const;
	virtual void			updateFrame();
protected:
	virtual void			doReset();
private:
};

inline void	HingeJoint::setAxis(int anum, const ArnVec3& axis)
{
	assert(anum == 1);
	dJointSetHingeAxis(getId(), axis.x, axis.y, axis.z);
}

inline void HingeJoint::updateFrame()
{
	updateFrameInternal(1);
}


#endif // HINGEJOINT_H
