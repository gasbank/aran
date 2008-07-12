#include "AranPCH.h"
#include "ArnSkeleton.h"
#include "ArnFile.h"

ArnSkeleton::ArnSkeleton()
: ArnNode(NDT_RT_SKELETON)
{
}

ArnSkeleton::~ArnSkeleton(void)
{
}

ArnNode* ArnSkeleton::createFrom( const NodeBase* nodeBase )
{
	ArnSkeleton* node = new ArnSkeleton();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_SKELETON1:
			node->buildFrom(static_cast<const NodeSkeleton1*>(nodeBase));
			break;
		case NDT_SKELETON2:
			node->buildFrom(static_cast<const NodeSkeleton2*>(nodeBase));
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
}

void ArnSkeleton::buildFrom( const NodeSkeleton1* ns )
{
	m_data.name					= ns->m_nodeName;
	m_data.associatedMeshName	= ns->m_associatedMeshName;
	m_data.maxWeightsPerVertex	= ns->m_maxWeightsPerVertex;
	m_data.bonesCount			= ns->m_boneCount;

	assert(m_data.name == getName());

	if (strncmp(getName(), "Skeleton-", 9) == 0)
		setParentName(getName() + 9); // implicit parent
	else
		throw MyError(MEE_SKELETON1NODE_CORRUPTED);
}

void ArnSkeleton::buildFrom( const NodeSkeleton2* ns )
{
	throw MyError(MEE_NOT_IMPLEMENTED_YET);
}