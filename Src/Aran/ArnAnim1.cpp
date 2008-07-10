#include "StdAfx.h"
#include "ArnAnim1.h"
#include "ArnFile.h"

ArnAnim1::ArnAnim1(void)
: ArnNode(NDT_ANIM1)
{
}

ArnAnim1::~ArnAnim1(void)
{
}


ArnNode* ArnAnim1::createFromNodeBase(const NodeBase* nodeBase)
{
	if (nodeBase->m_ndt != NDT_LIGHT)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeAnim1* na1 = static_cast<const NodeAnim1*>(nodeBase);
	ArnAnim1* node = new ArnAnim1();
	node->setData(na1);

	return node;
}

void ArnAnim1::setData(const NodeAnim1* na1)
{
	m_data = na1;
	setName(m_data->m_nodeName);
}