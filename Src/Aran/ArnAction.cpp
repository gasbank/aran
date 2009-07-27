#include "AranPCH.h"
#include "ArnAction.h"
#include "ArnXformable.h"
#include "ArnIpo.h"

ArnAction::ArnAction()
: ArnNode(NDT_ACTION2)
, m_objectIpoNameMap()
, m_objectIpoMap()
{
	//ctor
}

ArnAction::~ArnAction()
{
	//dtor
}

void
ArnAction::interconnect(ArnNode* sceneRoot)
{
	m_objectIpoMap.clear();
	typedef std::pair<STRING, STRING> StringStringPair;
	foreach(StringStringPair p, m_objectIpoNameMap)
	{
		ArnXformable* obj = dynamic_cast<ArnXformable*>(sceneRoot->getNodeByName(p.first));
		ArnIpo* ipo = dynamic_cast<ArnIpo*>(sceneRoot->getNodeByName(p.second));
		assert(obj && ipo);
		m_objectIpoMap[obj] = ipo;
	}
}
