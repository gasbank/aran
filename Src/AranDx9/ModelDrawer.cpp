#include "AranPCH.h"
#include "ModelDrawer.h"

ModelDrawer::ModelDrawer(void)
{
}

ModelDrawer::~ModelDrawer(void)
{
}

void ModelDrawer::clearMembers()
{
	lpAnimationController	= 0;
	//hierarchySize			= 0;
}

MyFrame* ModelDrawer::GetFrameRootByMeshIndex( int meshIndex )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
