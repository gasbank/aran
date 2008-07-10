#include "StdAfx.h"
#include "ArnCamera.h"
#include "ArnFile.h"
ArnCamera::ArnCamera()
: ArnNode(NDT_CAMERA)
{
}

ArnCamera::~ArnCamera(void)
{
}

ArnNode* ArnCamera::createFromNodeBase( const NodeBase* nodeBase )
{
	if (nodeBase->m_ndt != NDT_CAMERA)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	const NodeCamera* ns = static_cast<const NodeCamera*>(nodeBase);
	ArnCamera* node = new ArnCamera();
	node->setData(ns);

	return node;
}

void ArnCamera::setData( const NodeCamera* nc )
{
	m_data = nc;
	setName(m_data->m_nodeName);
}