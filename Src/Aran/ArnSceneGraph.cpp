#include "StdAfx.h"
#include "ArnSceneGraph.h"
#include "ArnFile.h"
#include "ArnContainer.h"
#include "ArnNodeFactory.h"
#include "ArnHierarchy.h"
#include "ArnBone.h"

ArnSceneGraph::ArnSceneGraph( const ArnFileData& afd )
: m_afd(afd), m_sceneRoot(0)
{
	if (strcmp(afd.m_fileDescriptor, "ARN20") == 0)
		m_exportVersion = EV_ARN20;
	else if (strcmp(afd.m_fileDescriptor, "ARN25") == 0)
		m_exportVersion = EV_ARN25;
	else
		throw MyError(MEE_UNSUPPORTED_ARNFILE);

	m_sceneRoot = new ArnContainer();
	unsigned int i;
	// Create a ArnNode from NodeBase and construct a scene graph
	// according to the implicit or explicit hierarchy definition.
	for (i = 0; i < afd.m_nodes.size(); ++i)
	{
		ArnNode* node = ArnNodeFactory::createFromNodeBase(afd.m_nodes[i]);

		if (node->getType() == NDT_RT_BONE && node->getParentName().length() == 0)
		{
			ArnNode* lastToplevelNode = m_sceneRoot->getLastNode();
			if (lastToplevelNode->getType() == NDT_RT_MESH && lastToplevelNode->getLastNode()->getType() == NDT_RT_SKELETON)
				lastToplevelNode->getLastNode()->attachChild(node);
			else
				throw MyError(MEE_UNDEFINED_ERROR);
		}
		else
		{
			if (node->getParentName().length() > 0)
			{
				ArnNode* parentNode = m_sceneRoot->getNodeByName(node->getParentName());
				if (parentNode)
					parentNode->attachChild(node);
				else
					throw MyError(MEE_NODE_NOT_FOUND);
			}
			else
			{
				m_sceneRoot->attachChild(node); // Child of scene root
			}
		}
	}

	if (m_exportVersion == EV_ARN20)
		postprocessingARN20();
}
ArnSceneGraph::~ArnSceneGraph(void)
{
	delete m_sceneRoot;
}

ArnSceneGraph* ArnSceneGraph::createFrom( const ArnFileData& afd )
{
	return new ArnSceneGraph(afd);
}

void ArnSceneGraph::postprocessingARN20()
{
	// Build bone hierarchy according to NDT_RT_HIERARCHY data
	// which is placed at the end of scene graph root nodes.
	if (m_sceneRoot->getLastNode()->getType() != NDT_RT_HIERARCHY)
		throw MyError(MEE_UNDEFINED_ERROR);

	ArnHierarchy* hierNode = static_cast<ArnHierarchy*>(m_sceneRoot->getLastNode());
	unsigned int frameCount = hierNode->getFrameCount();
	unsigned int i;
	// Find the root frame.
	for (i = 0; i < frameCount; ++i)
	{
		if (hierNode->getFrame(i).m_rootFlag == TRUE)
			break;
	}
	const MyFrameData& rootFrame = hierNode->getFrame(i);
	assert(rootFrame.m_sibling == -1);
	ArnBone* rootFrameNode = static_cast<ArnBone*>(m_sceneRoot->getNodeByName(rootFrame.m_frameName));
	ArnNode* skelNode = rootFrameNode->getParent();
	rootFrameNode->setFrameData(&rootFrame);

	buildBoneHierarchy(hierNode, skelNode, rootFrameNode);
}

void ArnSceneGraph::buildBoneHierarchy( ArnHierarchy* hierNode, ArnNode* skelNode, ArnBone* parentBoneNode )
{
	int firstChildIdx = parentBoneNode->getFrameData()->m_firstChild;
	int siblingIdx = parentBoneNode->getFrameData()->m_sibling;

	if (firstChildIdx > -1)
	{
		const MyFrameData& firstChildFrame = hierNode->getFrame((unsigned int)firstChildIdx);
		ArnBone* childBoneNode = static_cast<ArnBone*>(skelNode->getNodeByName(firstChildFrame.m_frameName));
		if (childBoneNode)
		{
			childBoneNode->setFrameData(&firstChildFrame);
			parentBoneNode->attachChild(childBoneNode);

			buildBoneHierarchy(hierNode, skelNode, childBoneNode);
		}
	}
	if (siblingIdx > -1)
	{
		const MyFrameData& siblingFrame = hierNode->getFrame((unsigned int)siblingIdx);
		ArnBone* siblingBoneNode = static_cast<ArnBone*>(skelNode->getNodeByName(siblingFrame.m_frameName));
		if (siblingBoneNode)
		{
			siblingBoneNode->setFrameData(&siblingFrame);
			parentBoneNode->getParent()->attachChild(siblingBoneNode);

			buildBoneHierarchy(hierNode, skelNode, siblingBoneNode);
		}
	}
}