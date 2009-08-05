#ifndef ANGULARJOINT_H
#define ANGULARJOINT_H

#include "GeneralJoint.h"

class AngularJoint : public GeneralJoint
{
public:
					AngularJoint(const OdeSpaceContext* osc);
	virtual			~AngularJoint();

	virtual void	setAnchor(const ArnVec3& pos) = 0;
	virtual void	getAnchor(dVector3 anchor) const = 0;

	virtual double	getValue(int anum) const { return getAngle(anum); }
	virtual dReal	getAngle(int anum) const = 0;

	// Control
	void			controlAddTorqueToTargetAngle(int anum, double torque, double angle);
	virtual void	controlAddTorque(int anum, double torque) = 0;

	virtual void	render(const BasicObjects& bo) const;
protected:
private:
};

#endif // ANGULARJOINT_H
