#include "AranDx9PCH.h"
#include "ArnSkeletonDx9.h"

ArnSkeletonDx9::ArnSkeletonDx9(void)
{
}

ArnSkeletonDx9::~ArnSkeletonDx9(void)
{
}


ArnSkeletonDx9*
ArnSkeletonDx9::createFrom( const NodeBase* nodeBase )
{
	/*
	ArnSkeletonDx9* node = new ArnSkeleton();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_SKELETON1:
			node->buildFrom(static_cast<const NodeSkeleton1*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

ArnSkeletonDx9 *ArnSkeletonDx9::createFrom( const ArnSkeleton *skel )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
void
ArnSkeletonDx9::buildFrom( const NodeSkeleton1* ns )
{
	/*
	m_data.name					= ns->m_nodeName;
	m_data.associatedMeshName	= ns->m_associatedMeshName;
	m_data.maxWeightsPerVertex	= ns->m_maxWeightsPerVertex;
	m_data.bonesCount			= ns->m_boneCount;

	assert(m_data.name == getName());

	if (strncmp(getName(), "Skeleton-", 9) == 0)
		setParentName(getName() + 9); // implicit parent
	else
		throw MyError(MEE_SKELETON1NODE_CORRUPTED);
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}
