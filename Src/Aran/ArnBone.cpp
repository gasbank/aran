#include "StdAfx.h"
#include "ArnBone.h"
#include "ArnFile.h"

ArnBone::ArnBone(void)
: ArnNode(NDT_RT_BONE), m_frameData(0)
{
}

ArnBone::~ArnBone(void)
{
}

ArnNode* ArnBone::createFrom( const NodeBase* nodeBase )
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

void ArnBone::buildFrom( const NodeBone1* nb )
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