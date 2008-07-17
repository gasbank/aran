#include "AranPCH.h"
#include "ArnCamera.h"
#include "ArnFile.h"

ArnCamera::ArnCamera()
: ArnMovable(NDT_RT_CAMERA)
{
}

ArnCamera::~ArnCamera(void)
{
}

ArnNode* ArnCamera::createFrom( const NodeBase* nodeBase )
{
	ArnCamera* node = new ArnCamera();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_CAMERA1:
			node->buildFrom(static_cast<const NodeCamera1*>(nodeBase));
			break;
		case NDT_CAMERA2:
			node->buildFrom(static_cast<const NodeCamera2*>(nodeBase));
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

void ArnCamera::buildFrom( const NodeCamera1* nc )
{
	m_cameraData = *nc->m_camera;
}

void ArnCamera::buildFrom( const NodeCamera2* nc )
{
	setParentName(nc->m_parentName);
	setLocalXform(*nc->m_localXform);
	setIpoName(nc->m_ipoName);
	m_cameraData.nearClip		= nc->m_clipStart;
	m_cameraData.farClip		= nc->m_clipEnd;
	m_cameraData.lookAtVector	= POINT3FLOAT::ZERO;
	m_cameraData.pos			= POINT3FLOAT::ZERO;
	m_cameraData.rot			= POINT4FLOAT::ZERO;
	m_cameraData.targetPos		= POINT3FLOAT::ZERO;
	m_cameraData.upVector		= POINT3FLOAT::ZERO;
}

void ArnCamera::interconnect( ArnNode* sceneRoot )
{
	setIpo(getIpoName());

	ArnNode::interconnect(sceneRoot);
}

