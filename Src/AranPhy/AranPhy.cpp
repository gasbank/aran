#include "AranPhyPCH.h"
#include "AranPhy.h"

static bool gs_bAranPhyInitialized = false;

int ArnInitializePhysics()
{
	if (gs_bAranPhyInitialized == false)
	{
		dInitODE();
		gs_bAranPhyInitialized = true;
		return 0;
	}
	else
	{
		// Already initialized.
		return -1;
	}
}

int ArnCleanupPhysics()
{
	if (gs_bAranPhyInitialized)
	{
		dCloseODE();
		gs_bAranPhyInitialized = false;
		return 0;
	}
	else
	{
		// Not initialized, but cleanup called.
		return -1;
	}
}
