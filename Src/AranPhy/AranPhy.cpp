#include "AranPhyPCH.h"
#include "AranPhy.h"

int ArnInitializePhysics()
{
	dInitODE();
	return 0;
}

int ArnCleanupPhysics()
{
	dCloseODE();
	return 0;
}