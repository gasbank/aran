/*!
 * @file SixDofJoint.h
 * @author Geoyeob Kim
 * @date 2010
 */
#pragma once

#include "AngularJoint.h"
#include "LinearJoint.h"

TYPEDEF_SHARED_PTR(SixDofJoint);
TYPEDEF_SHARED_PTR(GeneralBody);

class ARANPHY_API SixDofJoint : public AngularJoint, public LinearJoint
{
public:
	static SixDofJoint*			createFrom(const OdeSpaceContext* osc, const char* name, const GeneralBodyPtr b1, const GeneralBodyPtr b2, const ArnVec3& anchor, const ArnVec3& xAxis, const ArnVec3& yAxis, const ArnVec3& zAxis);
								~SixDofJoint();
	virtual void				attach(GeneralBody& body1, GeneralBody& body2);
	virtual const char*			getJointTypeName() const { return "SixDof"; }
	virtual void				setAxis(int anum, const ArnVec3& axis);
	virtual void				getAxis(int anum, dVector3 result) const;
	virtual dReal				getAngle(int anum) const;
	virtual dReal				getVelocity(int anum) const;
	virtual void				setParamLoHiStop(int anum, dReal lo, dReal hi);
	virtual void				controlAddTorque(int anum, double torque);
	virtual void				setParamVelocity(int anum, dReal v);
	virtual void				setParamFMax(int anum, dReal v);
	virtual void				renderJointAxis() const;
	virtual void				updateFrame();
	virtual void				configureOdeContext(const OdeSpaceContext* osc);
	virtual void				addTorque(AxisEnum anum, float torque);
	virtual dReal				getPosition(int anum) const { return dJointGetSliderPosition(getId()); }
	virtual double				getVelocity(int anum) const { return dJointGetSliderPositionRate(getId()); }
	virtual void				setAxis(int anum, const ArnVec3& pos) { dJointSetSliderAxis(getId(), pos.x, pos.y, pos.z); }
	virtual void				getAxis(int anum, dVector3 result) const { dJointGetSliderAxis(getId(), result); }
	virtual double				getValue(int ) { return getPosition(1); }
	virtual void				setParamVelocity(int , dReal v) { dJointSetSliderParam(getId(), dParamVel, v); }
	virtual void				setParamFMax(int , dReal v) { dJointSetSliderParam(getId(), dParamFMax, v); }
	virtual void				setParamLoHiStop(int anum, dReal lo, dReal hi);
	virtual void				renderJointAxis() const;
	virtual void				updateFrame();

protected:
	virtual void				doReset();
private:
								SixDofJoint(const OdeSpaceContext* osc);
};
