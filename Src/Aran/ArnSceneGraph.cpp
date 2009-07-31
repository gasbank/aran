#include "AranPCH.h"
#include "ArnSceneGraph.h"
#include "ArnContainer.h"
#include "ArnBone.h"
#include "ArnMesh.h"
#include "ArnBinaryChunk.h"
#include "ArnMaterial.h"
#include "ArnLight.h"
#include "ArnSkeleton.h"

ArnSceneGraph::ArnSceneGraph()
: ArnNode(NDT_RT_SCENEGRAPH)
, m_bRendererObjectInited(false)
, m_exportVersion(EV_ARN30)
, m_binaryChunk(0)
{
}

ArnSceneGraph::~ArnSceneGraph(void)
{
	delete m_binaryChunk;
}

ArnSceneGraph* ArnSceneGraph::createFromEmptySceneGraph()
{
	ArnSceneGraph* sg = new ArnSceneGraph();
	return sg;
}

void ArnSceneGraph::attachToRoot(ArnNode* node)
{
	attachChild(node);
}

void ArnSceneGraph::render()
{
	foreach (const ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_MESH)
		{
			ArnMesh* mesh = (ArnMesh*)node;
			mesh->render();
		}
		else if (node->getType() == NDT_RT_SKELETON)
		{
			ArnSkeleton* skel = (ArnSkeleton*)node;
			skel->render();
		}
	}
}

void ArnSceneGraph::interconnect(ArnNode* sceneRoot)
{
	ArnNode::interconnect(sceneRoot);
}

void ArnSceneGraph::initRendererObjects()
{
	foreach (ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_MESH)
		{
			ArnMesh* mesh = static_cast<ArnMesh*>(node);
			mesh->initRendererObject();
			//mesh->configureIpo();
		}
		else if (node->getType() == NDT_RT_MATERIAL)
		{
			ArnMaterial* mtrl = static_cast<ArnMaterial*>(node);
			mtrl->initRendererObject();
		}
		else if (node->getType() == NDT_RT_SKELETON)
		{
			ArnSkeleton* skel = static_cast<ArnSkeleton*>(node);
			skel->configureIpos();
		}
	}
	m_bRendererObjectInited = true;
}

ArnNode* ArnSceneGraph::findFirstNodeOfType( NODE_DATA_TYPE ndt )
{
	foreach (ArnNode* node, getChildren())
	{
		if (node->getType() == ndt)
		{
			return node;
		}
	}
	return 0;
}
