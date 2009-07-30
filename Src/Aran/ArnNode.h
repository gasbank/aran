#pragma once
#include "ArnObject.h"

class ArnNode;

typedef std::list<ArnNode*>					ArnNodeList;

class ARAN_API ArnNode : public ArnObject
{
public:
									ArnNode(NODE_DATA_TYPE type);
	virtual							~ArnNode(void);
	inline ArnNode*					getParent() const;
	inline const std::string&			getParentName() const;
	inline const char*				getName() const;
	inline void						setName(const char* name);
	void							attachChild(ArnNode* child);
	void							detachChild(ArnNode* child);
	void							deleteAllChildren();
	ArnNode*						getLastNode();
	ArnNode*						getNodeByName(const std::string& name) const;
	ArnNode*						getNodeAt(unsigned int idx);
	ArnNode*						getNodeById(unsigned int id);
	inline unsigned int				getNodeCount() const;
	inline ArnNode*					getSceneRoot();
	inline const ArnNodeList&		getChildren() const;
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
	std::string							m_name;
	ArnNode*						m_parent;
	std::string							m_parentName;
	ArnNodeList						m_children;
};

#include "ArnNode.inl"
