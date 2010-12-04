#include "AranPCH.h"
#include "version.h"
#include "AranApi.h"

long ArnGetBuildCount()
{
	return AutoVersion::BUILDS_COUNT;
}
