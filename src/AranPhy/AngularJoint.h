#ifndef ANGULARJOINT_H
#define ANGULARJOINT_H

#include "GeneralJoint.h"

class ARANPHY_API AngularJoint : public GeneralJoint
{
public:
	virtual						~AngularJoint();
	/*! @name 관절 앵커
	관절 앵커(anchor)는 관절로 이어진 두 물체가 어떤 점을 중심으로
	회전할 수 있는지를 나타냅니다.
	*/
	//@{
	const ArnVec3&				getAnchor() const { return m_anchor; }
	void						setAnchor(const ArnVec3& v) { m_anchor = v; }
	//@}
	virtual double				getValue(int anum) const { return getAngle(anum); }
	virtual dReal				getAngle(int anum) const = 0;
	void						controlAddTorqueToTargetAngle(int anum, double torque, double angle);
	virtual void				controlAddTorque(int anum, double torque) = 0;
	virtual void				render(const BasicObjects& bo) const;
protected:
								AngularJoint(const OdeSpaceContext* osc);
private:
	ArnVec3						m_anchor;
};

#endif // ANGULARJOINT_H
