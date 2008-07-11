#pragma once
#include "ArnObject.h"
#include <set>

struct NodeBase;

class ArnNode : public ArnObject
{
public:
	typedef std::set<ArnNode*> ChildrenSet;

	ArnNode(NODE_DATA_TYPE type);
	virtual ~ArnNode(void);

	ArnNode*				getParent() { return m_parent; }
	void					setParent(ArnNode* node) { m_parent = node; }
	const char*				getName() const { return m_name.c_str(); }
	void					setName(const char* name) { m_name = name; }
	void					attachChild(ArnNode* child);
	void					deleteAllChildren();
	const D3DXMATRIX&		getLocalXform() const { return m_localXform; }
	void					setLocalXform(const D3DXMATRIX& localXform) { m_localXform = localXform; }
private:
	STRING			m_name;
	ArnNode*		m_parent;
	ChildrenSet		m_children;
	D3DXMATRIX		m_localXform;
};
