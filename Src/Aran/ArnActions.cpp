#include "AranPCH.h"
#include "ArnActions.h"
#include "ArnFile.h"

ArnActions::ArnActions(void)
: ArnNode(NDT_RT_ACTIONS)
, m_data()
{
}

ArnActions::~ArnActions(void)
{
}

ArnActions* ArnActions::createFrom( const NodeBase* nodeBase )
{
	ArnActions* node = new ArnActions();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_ACTION1:
			node->buildFrom(static_cast<const NodeAction1*>(nodeBase));
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

void ArnActions::interconnect( ArnNode* sceneRoot )
{
	UNREFERENCED_PARAMETER(sceneRoot);
}

void ArnActions::buildFrom( const NodeAction1* ni )
{
	assert( ni->m_actions.size() == ni->m_actionCount );
	unsigned i, j;
	if ( ni->m_actionCount )
	{
		m_data.resize(ni->m_actionCount);
		for (i = 0; i < ni->m_actionCount; ++i)
		{
			m_data[i].actionName = ni->m_actions[i].first;
			const unsigned channelCount = ni->m_actions[i].second.size();
			m_data[i].channels.resize( channelCount );
			for (j = 0; j < channelCount; ++j)
			{
				m_data[i].channels[j].first = ni->m_actions[i].second[j].first; // Bone name
				m_data[i].channels[j].second = ni->m_actions[i].second[j].second; // Corresponding ipo name
			}
		}
	}
}
