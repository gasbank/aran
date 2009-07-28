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
		// Note: IPO, Action and NLA should be configured in blender
		//       in order to play an animation
		if (!(obj && ipo))
		{
			std::cerr << " ** Critical error while interconnecting ArnAction : Object-Ipo mapping corruption." << std::endl;
			std::cerr << "      Object: " << p.first << "  --->  " << "IPO: " << p.second << std::endl;
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
		m_objectIpoMap[obj] = ipo;
	}
}

ArnAction* ArnAction::createFrom(ArnNode* obj, ArnIpo* ipo)
{
	ArnAction* ret = new ArnAction();
	ret->addMap(obj->getName(), ipo->getName());
	ret->m_objectIpoMap[obj] = ipo;
	return ret;
}