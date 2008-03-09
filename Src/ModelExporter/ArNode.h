#pragma once
#include "arobject.h"

class ArNode :
	public ArObject
{
public:
	enum Type {
		eArMesh, eArCamera, eArLight
	};
	ArNode(void);
	ArNode(const char* name) : ArObject(name) { }
	virtual ~ArNode(void);

	ArMatrix4 getWorldMatrix() const { return m_worldMatrix; }
	void setWorldMatrix(const ArMatrix4& val) { m_worldMatrix = val; }

	const ArNode* getParent() const { return m_parent; }
	void setParent(ArNode* const val);

	// *** INTERNAL USE ONLY ***
	void addChild(ArNode* const val);
private:
	ArMatrix4 m_worldMatrix;	
	ArNode* m_parent;
	std::list<ArNode*> m_children;
};
