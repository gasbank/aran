#include "StdAfx.h"
#include "ArnAnim.h"
#include "ArnFile.h"

ArnAnim::ArnAnim(void)
: ArnNode(NDT_RT_ANIM)
{
}

ArnAnim::~ArnAnim(void)
{
}

ArnNode* ArnAnim::createFromNodeBase(const NodeBase* nodeBase)
{
	if (nodeBase->m_ndt != NDT_ANIM1)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeAnim1* na1 = static_cast<const NodeAnim1*>(nodeBase);
	ArnAnim* node = new ArnAnim();
	node->setData(na1);

	return node;
}

void ArnAnim::setData(const NodeAnim1* na1)
{
	m_data = na1;
	setName(m_data->m_nodeName);
}