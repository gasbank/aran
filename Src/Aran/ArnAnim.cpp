#include "AranPCH.h"
#include "ArnAnim.h"
#include "ArnFile.h"

ArnAnim::ArnAnim(void)
: ArnNode(NDT_RT_ANIM)
{
}

ArnAnim::~ArnAnim(void)
{
}

ArnNode* ArnAnim::createFrom(const NodeBase* nodeBase)
{
	ArnAnim* node = new ArnAnim();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_ANIM1:
			node->buildFrom(static_cast<const NodeAnim1*>(nodeBase));
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

void ArnAnim::buildFrom( const NodeAnim1* na )
{
	m_data.resize(na->m_keyCount);
	unsigned int i;
	for (i = 0; i < m_data.size(); ++i)
	{
		m_data[i] = na->m_rstKeys[i];
	}
	if (strncmp(getName(), "Anim-", 5) == 0)
		setParentName(getName() + 5); // implicit parent
	else
		throw MyError(MEE_ANIM1NODE_CORRUPTED);
}