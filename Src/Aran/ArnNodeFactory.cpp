#include "StdAfx.h"
#include "ArnNodeFactory.h"
#include "ArnMesh.h"
#include "ArnSkeleton.h"
#include "ArnHierarchy.h"
#include "ArnLight.h"
#include "ArnCamera.h"
#include "ArnFile.h"
#include "ArnAnim1.h"

ArnNode* ArnNodeFactory::createFromNodeBase( const NodeBase* nodeBase )
{
	ArnNode* node = 0;
	switch (nodeBase->m_ndt)
	{
	case NDT_MESH1:
	case NDT_MESH2:
	case NDT_MESH3:
		node = ArnMesh::createFromNodeBase(nodeBase);
		break;
	case NDT_SKELETON:
		node = ArnSkeleton::createFromNodeBase(nodeBase);
		break;
	case NDT_HIERARCHY:
		node = ArnHierarchy::createFromNodeBase(nodeBase);
		break;
	case NDT_LIGHT:
		node = ArnLight::createFromNodeBase(nodeBase);
		break;
	case NDT_CAMERA:
		node = ArnCamera::createFromNodeBase(nodeBase);
		break;
	case NDT_ANIM1:
		node = ArnAnim1::createFromNodeBase(nodeBase);
		break;
	default: // unidentified node, maybe corrupted or unsupported; skip the node
		throw MyError(MEE_UNDEFINED_ERROR);
	}

	return node;
}