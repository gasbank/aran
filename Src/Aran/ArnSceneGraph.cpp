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
, m_exportVersion(EV_ARN30)
, m_binaryChunk(0)
{
}

ArnSceneGraph::~ArnSceneGraph(void)
{
	delete m_binaryChunk;
}

ArnSceneGraph*
ArnSceneGraph::createFromEmptySceneGraph()
{
	ArnSceneGraph* sg = new ArnSceneGraph();
	return sg;
}

void
ArnSceneGraph::attachToRoot(ArnNode* node)
{
	attachChild(node);
}

void
ArnSceneGraph::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

void
ArnSceneGraph::interconnect(ArnNode* sceneRoot)
{
	ArnNode::interconnect(sceneRoot);
}

void
ArnSceneGraph::initRendererObjects()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

ArnNode*
ArnSceneGraph::findFirstNodeOfType( NODE_DATA_TYPE ndt )
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
