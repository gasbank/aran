#include "AranPCH.h"
#include "ArnNodeFactory.h"
#include "ArnMesh.h"
#include "ArnSkeleton.h"
#include "ArnHierarchy.h"
#include "ArnLight.h"
#include "ArnCamera.h"
#include "ArnFile.h"
#include "ArnAnim.h"
#include "ArnBone.h"
#include "ArnMaterial.h"

ArnNode* ArnNodeFactory::createFromNodeBase( const NodeBase* nodeBase )
{
	ArnNode* node = 0;
	switch (nodeBase->m_ndt)
	{
	case NDT_MESH2:
	case NDT_MESH3:
		node = ArnMesh::createFrom(nodeBase);
		break;
	case NDT_SKELETON1:
		node = ArnSkeleton::createFrom(nodeBase);
		break;
	case NDT_HIERARCHY1:
		node = ArnHierarchy::createFrom(nodeBase);
		break;
	case NDT_LIGHT1:
	case NDT_LIGHT2:
		node = ArnLight::createFrom(nodeBase);
		break;
	case NDT_CAMERA1:
	case NDT_CAMERA2:
		node = ArnCamera::createFrom(nodeBase);
		break;
	case NDT_ANIM1:
		node = ArnAnim::createFrom(nodeBase);
		break;
	case NDT_BONE1:
		node = ArnBone::createFrom(nodeBase);
		break;
	case NDT_MATERIAL1:
	case NDT_MATERIAL2:
		node = ArnMaterial::createFrom(nodeBase);
		break;
	default:
		// unidentified node, maybe corrupted or unsupported; skip the node
		throw MyError(MEE_UNDEFINED_ERROR);
	}

	return node;
}