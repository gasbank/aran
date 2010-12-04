#include "AranPhyPCH.h"
#include "ArnPhyBox.h"

ArnPhyBox::ArnPhyBox( const OdeSpaceContext* osc )
: GeneralBody(osc)
{
}

ArnPhyBox::~ArnPhyBox()
{
}

ArnPhyBox*
ArnPhyBox::createFrom(const OdeSpaceContext* osc, const char* name, const ArnVec3& com, const ArnVec3& size, float mass, bool fixed)
{
	ArnPhyBox* ret = new ArnPhyBox(osc);
	ret->setName(name);
	ret->setMass(mass);
	ret->setMassDistributionType(AMDT_BOX);
	ret->setMassDistributionSize(size);
	ret->setBoundingBoxType(ABBT_BOX);
	ret->setBoundingBoxSize(size);
	ret->setInitialCom(com);
	ret->setFixed(fixed);
	return ret;
}
