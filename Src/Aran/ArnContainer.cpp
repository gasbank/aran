#include "AranPCH.h"
#include "ArnContainer.h"

ArnContainer::ArnContainer(void)
: ArnNode(NDT_RT_CONTAINER)
{
}

ArnContainer::~ArnContainer(void)
{
}

ArnContainerPtr
ArnContainer::createFromEmpty()
{
	ArnContainerPtr retPtr(new ArnContainer());
	return retPtr;
}
