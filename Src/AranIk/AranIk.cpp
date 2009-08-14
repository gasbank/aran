#include "AranIkPCH.h"
#include "AranIk.h"
#include "ArnIkSolver.h"
#include "ArnSkeleton.h"

void
CreateArnIkSolver(ArnIkSolver** solver, const ArnSkeleton* skel)
{
	assert(*solver == 0);
	*solver = ArnIkSolver::createFrom(skel);
}

void
ArnCreateArnIkSolversOnSceneGraph( std::vector<ArnIkSolver*>& ikSolvers, ArnSceneGraphPtr sg )
{
	assert(ikSolvers.size() == 0);
	foreach(ArnNode* node, sg->getChildren())
	{
		if (node->getType() == NDT_RT_SKELETON)
		{
			ArnSkeleton* skel = static_cast<ArnSkeleton*>(node);
			ArnIkSolver* ikSolver = 0;
			CreateArnIkSolver(&ikSolver, skel);
			assert(ikSolver);
			skel->setVisible(false);
			ikSolvers.push_back(ikSolver);
		}
	}
}
