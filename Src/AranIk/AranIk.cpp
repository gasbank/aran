#include "AranIkPCH.h"
#include "AranIk.h"
#include "ArnIkSolver.h"
#include "ArnSkeleton.h"

void
ArnCreateArnIkSolversOnSceneGraph( std::vector<ArnIkSolver*>& ikSolvers, ArnSceneGraphPtr sg )
{
	assert(ikSolvers.size() == 0);
	foreach(ArnNode* node, sg->getChildren())
	{
		if (node->getType() == NDT_RT_SKELETON)
		{
			ArnSkeleton* skel = static_cast<ArnSkeleton*>(node);
			ArnIkSolver* ikSolver = ArnIkSolver::createFrom(skel);
			assert(ikSolver);
			skel->setVisible(false);
			ikSolvers.push_back(ikSolver);
		}
	}
}
