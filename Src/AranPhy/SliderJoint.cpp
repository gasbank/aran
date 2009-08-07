#include "AranPhyPCH.h"
#include "SliderJoint.h"

SliderJoint::SliderJoint(const OdeSpaceContext* osc)
: LinearJoint(osc)
{
	//ctor
	setId(dJointCreateSlider(osc->world, 0 /* Joint group ID */));
}

SliderJoint::~SliderJoint()
{
	//dtor
}

void SliderJoint::doReset()
{
	setParamVelocity(1, 0);
	setParamFMax(1, 0);
}

void SliderJoint::renderJointAxis() const
{
}

void SliderJoint::setParamLoHiStop(int anum, dReal lo, dReal hi)
{
	assert(anum == 1);
	double cur = getValue(anum);
	if (cur >= lo && cur <= hi)
	{
		dJointSetSliderParam(getId(), dParamLoStop, lo);
		dJointSetSliderParam(getId(), dParamHiStop, hi);
	}
	else
	{
		fprintf(stderr, "New LoHi params must include the current value of joints.\n");
		abort();
	}
}

