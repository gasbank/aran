#include "AranPhyPCH.h"
#include "SixDofJoint.h"

SixDofJoint::SixDofJoint(const OdeSpaceContext* osc)
: AngularJoint(osc)
, LinearJoint(osc)
{
}

SixDofJoint::~SixDofJoint(void)
{
}
