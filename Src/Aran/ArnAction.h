#ifndef ARNACTION_H
#define ARNACTION_H

#include "ArnNode.h"

class ArnNode;
class ArnIpo;

class ArnIpo;

class ArnAction : public ArnNode
{
public:
											ArnAction();
	virtual									~ArnAction();
	static ArnAction*						createFrom(const DOMElement* elm);
	const std::map<ArnNode*, ArnIpo*>&		getObjectIpoMap() const { return m_objectIpoMap; }
	// *** INTERNAL USE ONLY START ***
	virtual void							interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
protected:
private:
	void									addMap(const char* objName, const char* ipoName) { m_objectIpoNameMap[objName] = ipoName; }
	std::map<STRING, STRING>				m_objectIpoNameMap;
	std::map<ArnNode*, ArnIpo*>				m_objectIpoMap;
};

#endif // ARNACTION_H
