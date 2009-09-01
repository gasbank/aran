#include "AranPCH.h"
#include <float.h>
#include "ArnBone.h"
#include "ArnMath.h"
#include "ArnSkeleton.h"

ArnBone::ArnBone(void)
: ArnXformable(NDT_RT_BONE)
, m_frameData()
, m_roll(0)
, m_boneLength(0)
{
	clearRotLimit(AXIS_X);
	clearRotLimit(AXIS_Y);
	clearRotLimit(AXIS_Z);
}

ArnBone::~ArnBone(void)
{
}

ArnBone*
ArnBone::createFrom(const float length, const float roll)
{
	ArnBone* bone = new ArnBone();
	bone->m_boneLength = length;
	bone->m_roll = roll;
	return bone;
}

ArnBone*
ArnBone::createFrom( const NodeBase* nodeBase )
{
	ArnBone* node = new ArnBone();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_BONE1:
			node->buildFrom(static_cast<const NodeBone1*>(nodeBase));
			break;
		case NDT_BONE2:
			node->buildFrom(static_cast<const NodeBone2*>(nodeBase));
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

void
ArnBone::buildFrom( const NodeBone1* nb )
{
	m_data.nameFixed		= nb->m_nodeName;
	m_data.offsetMatrix		= *nb->m_offsetMatrix;
	m_data.infVertexCount	= nb->m_infVertexCount;
	unsigned int i;
	m_data.indices.resize(m_data.infVertexCount, 0);
	m_data.weights.resize(m_data.infVertexCount, 0);
	for (i = 0; i < m_data.infVertexCount; ++i)
	{
		m_data.indices[i] = nb->m_vertexIndices[i];
		m_data.weights[i] = nb->m_weights[i];
	}
}

void
ArnBone::buildFrom( const NodeBone2* nb )
{
	m_data.nameFixed		= nb->m_nodeName;
	m_data.offsetMatrix		= *nb->m_offsetMatrix;

	/*m_data.infVertexCount	= nb->m_infVertCount;
	unsigned int i;
	m_data.indices.resize(m_data.infVertexCount, 0);
	m_data.weights.resize(m_data.infVertexCount, 0);
	for (i = 0; i < m_data.infVertexCount; ++i)
	{
		m_data.indices[i] = nb->m_indWeightArray[i].ind;
		m_data.weights[i] = nb->m_indWeightArray[i].weight;
	}*/

	// If bone name is the same as its armature, this bone is root.
	setParentName( nb->m_parentBoneName );
}

unsigned int
ArnBone::getChildBoneCount(bool bRecursive) const
{
	unsigned int ret = 0;
	foreach(const ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_BONE)
		{
			ret += 1 + (bRecursive ? static_cast<const ArnBone*>(node)->getChildBoneCount(true) : 0);
		}
	}
	return ret;
}

void
ArnBone::setRotLimit( AxisEnum axis, float minimum, float maximum )
{
	assert(minimum <= maximum);
	if (axis == AXIS_X)
	{
		m_rotLimit[0][0] = minimum;
		m_rotLimit[0][1] = maximum;
	}
	else if (axis == AXIS_Y)
	{
		m_rotLimit[1][0] = minimum;
		m_rotLimit[1][1] = maximum;
	}
	else if (axis == AXIS_Z)
	{
		m_rotLimit[2][0] = minimum;
		m_rotLimit[2][1] = maximum;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

void
ArnBone::clearRotLimit( AxisEnum axis )
{
	if (axis == AXIS_X)
	{
		m_rotLimit[0][0] = -FLT_MAX;
		m_rotLimit[0][1] =  FLT_MAX;
	}
	else if (axis == AXIS_Y)
	{
		m_rotLimit[1][0] = -FLT_MAX;
		m_rotLimit[1][1] =  FLT_MAX;
	}
	else if (axis == AXIS_Z)
	{
		m_rotLimit[2][0] = -FLT_MAX;
		m_rotLimit[2][1] =  FLT_MAX;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

void
ArnBone::getRotLimit( AxisEnum axis, float& minimum, float& maximum ) const
{
	if (axis == AXIS_X)
	{
		minimum = m_rotLimit[0][0];
		maximum = m_rotLimit[0][1];
	}
	else if (axis == AXIS_Y)
	{
		minimum = m_rotLimit[1][0];
		maximum = m_rotLimit[1][1];
	}
	else if (axis == AXIS_Z)
	{
		minimum = m_rotLimit[2][0];
		maximum = m_rotLimit[2][1];
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

ArnBone* ArnBone::getFirstChildBone() const
{
	assert(getChildBoneCount(false) >= 1);
	foreach (ArnNode* n, getChildren())
	{
		if (n->getType() == NDT_RT_BONE)
			return static_cast<ArnBone*>(n);
	}
	ARN_THROW_UNEXPECTED_CASE_ERROR
}

ArnSkeleton*
ArnBone::getParentSkeleton() const
{
	ArnNode* node = getParent();
	while (node && node->getType() != NDT_RT_SKELETON)
	{
		node = node->getParent();
	}
	return static_cast<ArnSkeleton*>(node);
}

const ArnMatrix&
ArnBone::getAutoLocalXform() const
{
	if (getParentSkeleton()->getAnimCtrl())
	{
		assert(isAnimLocalXformDirty() == false);
		return getAnimLocalXform();
	}
	else
	{
		assert(isLocalXformDirty() == false);
		return getLocalXform();
	}
}
