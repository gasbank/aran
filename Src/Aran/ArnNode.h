#pragma once
#include "ArnObject.h"

class ArnNode : public ArnObject
{
public:
	typedef std::list<ArnNode*>		ChildrenList;

									ArnNode(NODE_DATA_TYPE type);
	virtual							~ArnNode(void);

	inline ArnNode*					getParent() const;
	inline const STRING&			getParentName() const;
	inline const char*				getName() const;
	inline void						setName(const char* name);

	void							attachChild(ArnNode* child);
	void							detachChild(ArnNode* child);
	void							deleteAllChildren();
	ArnNode*						getLastNode();
	ArnNode*						getNodeByName(const STRING& name) const;
	ArnNode*						getNodeAt(unsigned int idx);
	ArnNode*						getNodeById(unsigned int id);
	inline unsigned int				getNodeCount() const;
	inline ArnNode*					getSceneRoot();
	inline const ChildrenList&		getChildren() const;
	inline void						setParent(ArnNode* node);
	inline void						detachParent();


	virtual void					update(double fTime, float fElapsedTime);

	void							printNodeHierarchy(int depth) const;
	// *** INTERNAL USE ONLY START ***
	virtual void					interconnect(ArnNode* sceneRoot) = 0;

	// *** INTERNAL USE ONLY END ***
protected:
	inline void						setParentName(const char* name);

private:
	STRING							m_name;
	ArnNode*						m_parent;
	STRING							m_parentName;
	ChildrenList					m_children;
};

#include "ArnNode.inl"
