#include "AranIkPCH.h"
#include "AranIk.h"
#include "ArnIkSolver.h"
#include "ArnSkeleton.h"

void CreateArnIkSolver(ArnIkSolver** solver, const ArnSkeleton* skel)
{
	assert(*solver == 0);
	*solver = ArnIkSolver::createFrom(skel);
}
