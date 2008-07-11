#include "StdAfx.h"
#include "ArnCamera.h"
#include "ArnFile.h"
ArnCamera::ArnCamera()
: ArnNode(NDT_RT_CAMERA)
{
}

ArnCamera::~ArnCamera(void)
{
}

ArnNode* ArnCamera::createFrom( const NodeBase* nodeBase )
{
	ArnCamera* node = new ArnCamera();

	switch (nodeBase->m_ndt)
	{
	case NDT_CAMERA1:
		node->buildFrom(static_cast<const NodeCamera1*>(nodeBase));
		break;
	case NDT_CAMERA2:
		node->buildFrom(static_cast<const NodeCamera2*>(nodeBase));
		break;
	default:
		delete node;
		throw MyError(MEE_UNDEFINED_ERROR);
	}
	return node;
}

void ArnCamera::buildFrom( const NodeCamera1* nc )
{

}

void ArnCamera::buildFrom( const NodeCamera2* nc )
{

}