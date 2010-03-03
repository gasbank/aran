/*!
 * @file BallSocketJoint.h
 * @author Geoyeob Kim
 * @date 2009
 */
#ifndef BALLSOCKETJOINT_H
#define BALLSOCKETJOINT_H

#include "AngularJoint.h"

TYPEDEF_SHARED_PTR(BallSocketJoint);
TYPEDEF_SHARED_PTR(GeneralBody);

/*!
 * @brief Ball-socket 형태의 관절 관련 함수 ODE 랩퍼
 */
class ARANPHY_API BallSocketJoint : public AngularJoint
{
public:
	static BallSocketJoint*		createFrom(const OdeSpaceContext* osc, const char* name, const GeneralBodyPtr b1, const GeneralBodyPtr b2, const ArnVec3& anchor, const ArnVec3& xAxis, const ArnVec3& yAxis, const ArnVec3& zAxis);
								~BallSocketJoint();
	virtual void				attach(GeneralBody& body1, GeneralBody& body2);
	virtual const char*			getJointTypeName() const { return "BallSocket"; }
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
protected:
	virtual void				doReset();
private:
								BallSocketJoint(const OdeSpaceContext* osc);
	dJointID					m_amotorId;
	ArnVec3						m_xAxis;
	ArnVec3						m_yAxis;
	ArnVec3						m_zAxis;
};

#endif // BALLSOCKETJOINT_H
