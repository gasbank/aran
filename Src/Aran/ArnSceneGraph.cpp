#include "StdAfx.h"
#include "ArnSceneGraph.h"
#include "ArnFile.h"
#include "ArnContainer.h"
#include "ArnNodeFactory.h"

ArnSceneGraph::ArnSceneGraph( const ArnFileData& afd )
: m_afd(afd)
{
	m_sceneRoot = new ArnContainer();
	unsigned int i;
	for (i = 0; i < afd.m_nodeCount; ++i)
	{
		ArnNode* node = ArnNodeFactory::createFromNodeBase(afd.m_nodes[i]);
		m_sceneRoot->attachChild(node);
	}
}
ArnSceneGraph::~ArnSceneGraph(void)
{
	delete m_sceneRoot;
}
