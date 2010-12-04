#include "AranDx9PCH.h"
#include "ArnHierarchy.h"
#include "ArnFile.h"

ArnHierarchy::ArnHierarchy(void)
: ArnNode(NDT_RT_HIERARCHY)
{
}

ArnHierarchy::~ArnHierarchy(void)
{
}

ArnNode* ArnHierarchy::createFrom( const NodeBase* nodeBase )
{
	ArnHierarchy* node = new ArnHierarchy();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_HIERARCHY1:
			node->buildFrom(static_cast<const NodeHierarchy1*>(nodeBase));
			break;
		case NDT_HIERARCHY2:
			node->buildFrom(static_cast<const NodeHierarchy2*>(nodeBase));
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

void ArnHierarchy::buildFrom( const NodeHierarchy1* nh )
{
	m_data.resize(nh->m_frameCount);
	unsigned int i;
	for (i = 0; i < m_data.size(); ++i)
	{
		MyFrameData& data			= m_data[i];
		MyFrameDataShell& dataOrig	= nh->m_frames[i];

		data.m_frameName	= dataOrig.m_frameName;
		data.m_rootFlag		= dataOrig.m_rootFlag;
		data.m_sibling		= dataOrig.m_sibling;
		data.m_firstChild	= dataOrig.m_firstChild;
	}
}


void ArnHierarchy::buildFrom( const NodeHierarchy2* ns )
{
	m_data2.name					= ns->m_nodeName;
	m_data2.associatedMeshName		= "~ Should not see me~";
	m_data2.maxWeightsPerVertex		= 4;
	m_data2.bonesCount				= ns->m_boneCount;

	assert(m_data2.name == getName());

}

const MyFrameData& ArnHierarchy::getFrame( unsigned int idx ) const
{
	if (idx < getFrameCount())
		return m_data[idx];
	else
		throw MyError(MEE_STL_INDEX_OUT_OF_BOUNDS);
}
