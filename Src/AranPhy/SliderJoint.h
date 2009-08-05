#ifndef SLIDERJOINT_H
#define SLIDERJOINT_H

#include "LinearJoint.h"

class SliderJoint : public LinearJoint
{
public:
					SliderJoint(const OdeSpaceContext* osc);
	virtual			~SliderJoint();

	virtual const char*		getJointTypeName() const { return "Slider"; }

	virtual dReal	getPosition(int anum) const { return dJointGetSliderPosition(getId()); }
	virtual double	getVelocity(int anum) const { return dJointGetSliderPositionRate(getId()); }

	virtual void	setAxis(int anum, const ArnVec3& pos) { dJointSetSliderAxis(getId(), pos.x, pos.y, pos.z); }
	virtual void	getAxis(int anum, dVector3 result) const { dJointGetSliderAxis(getId(), result); }

	virtual double	getValue(int ) { return getPosition(1); }

	virtual void	setParamVelocity(int , dReal v) { dJointSetSliderParam(getId(), dParamVel, v); }
	virtual void	setParamFMax(int , dReal v) { dJointSetSliderParam(getId(), dParamFMax, v); }

	virtual void	setParamLoHiStop(int anum, dReal lo, dReal hi);
	virtual void	renderJointAxis() const;

	virtual void	updateFrame();

protected:
	virtual void	doReset();
private:
};

inline void SliderJoint::updateFrame()
{
	updateFrameInternal(1);
}

#endif // SLIDERJOINT_H
