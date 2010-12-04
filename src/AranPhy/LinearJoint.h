#ifndef LINEARJOINT_H
#define LINEARJOINT_H

#include "GeneralJoint.h"

class LinearJoint : public GeneralJoint
{
public:
					LinearJoint(const OdeSpaceContext* osc);
	virtual			~LinearJoint();

	virtual double	getValue(int anum) const { return getPosition(anum); }
	virtual dReal	getPosition(int anum) const = 0;
protected:
private:
};

#endif // LINEARJOINT_H
