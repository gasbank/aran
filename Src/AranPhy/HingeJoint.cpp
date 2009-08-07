#include "AranPhyPCH.h"
#include "HingeJoint.h"

HingeJoint::HingeJoint(const OdeSpaceContext* osc)
: AngularJoint(osc)
{
	//ctor
	setId(dJointCreateHinge(osc->world, 0 /* Joint group ID */));
}

HingeJoint::~HingeJoint()
{
	//dtor
}

void HingeJoint::doReset()
{
	setParamVelocity(1, 0);
	setParamFMax(1, 0);
}

void HingeJoint::renderJointAxis() const
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
	/*const double scale = 0.25;
	dVector3 anchor, axis1;
	getAnchor(anchor);
	getAxis(1, axis1);
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	glBegin(GL_LINES);
	glVertex3dv(anchor);
	glVertex3d(anchor[0] + scale * axis1[0], anchor[1] + scale * axis1[1], anchor[2] + scale * axis1[2]);
	glEnd();
	glEnable(GL_LIGHTING);*/
}

void HingeJoint::getAxis(int anum, dVector3 result) const
{
	switch(anum)
	{
		case 1:	dJointGetHingeAxis(getId(), result); break;
		default: dAASSERT(false); break;
	}
}

void HingeJoint::setParamLoHiStop(int anum, dReal lo, dReal hi)
{
	assert(anum == 1);
	double cur = getValue(anum);
	if (cur >= lo && cur <= hi)
	{
		dJointSetHingeParam(getId(), dParamLoStop, lo);
		dJointSetHingeParam(getId(), dParamHiStop, hi);
	}
	else
	{
		fprintf(stderr, "New LoHi params must include the current value of joints.\n");
		abort();
	}
}
