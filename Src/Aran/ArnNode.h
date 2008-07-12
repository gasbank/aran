#pragma once
#include "ArnObject.h"

class ArnNode : public ArnObject
{
public:
	typedef std::list<ArnNode*> ChildrenList;

	ArnNode(NODE_DATA_TYPE type);
	virtual ~ArnNode(void);

	ArnNode*				getParent() { return m_parent; }
	const STRING&			getParentName() const { return m_parentName; }
	const char*				getName() const { return m_name.c_str(); }
	void					setName(const char* name) { m_name = name; }
	void					attachChild(ArnNode* child);
	void					detachChild(ArnNode* child);
	void					deleteAllChildren();
	const D3DXMATRIX&		getLocalXform() const { return m_localXform; }
	void					setLocalXform(const D3DXMATRIX& localXform) { m_localXform = localXform; }
	ArnNode*				getLastNode();
	ArnNode*				getNodeByName(const STRING& name);
	ArnNode*				getNodeAt(unsigned int idx);

	// *** INTERNAL USE ONLY START ***
	void					setParent(ArnNode* node) { m_parent = node; }
	void					detachParent() { if (m_parent) m_parent->detachChild(this); }
	// *** INTERNAL USE ONLY END ***
protected:
	void					setParentName(const char* name) { m_parentName = name; }

private:
	STRING					m_name;
	ArnNode*				m_parent;
	STRING					m_parentName;
	ChildrenList			m_children;
	D3DXMATRIX				m_localXform;
};
