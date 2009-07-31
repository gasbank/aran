#pragma once
#include "ArnObject.h"

class ArnNode;
class ArnRenderableObject;

typedef std::list<ArnNode*>					ArnNodeList;

class ARAN_API ArnNode : public ArnObject
{
public:
	virtual							~ArnNode(void);
	inline ArnNode*					getParent() const;
	inline const std::string&		getParentName() const;
	inline const char*				getName() const;
	inline void						setName(const char* name);
	void							attachChild(ArnNode* child);
	void							detachChild(ArnNode* child);
	void							deleteAllChildren();
	ArnNode*						getLastNode();
	ArnNode*						getNodeByName(const std::string& name);  // Cannot be const function since it can return itself(this pointer).
	const ArnNode*					getConstNodeByName(const std::string& name) const;
	ArnNode*						getNodeAt(unsigned int idx) const;
	ArnNode*						getNodeById(unsigned int id); // Cannot be const function since it can return itself(this pointer).
	const ArnNode*					getConstNodeById(unsigned int id) const;
	inline unsigned int				getNodeCount() const;
	inline ArnNode*					getSceneRoot();
	inline const ArnNode*			getConstSceneRoot() const;
	inline const ArnNodeList&		getChildren() const;
	inline void						setParent(ArnNode* node);
	inline void						detachParent();
	virtual void					update(double fTime, float fElapsedTime);
	void							printNodeHierarchy(int depth) const;
	const ArnRenderableObject*		getRenderableObject() const; // Return a first renderable object(should be unique) from direct(depth 0) children.

	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	virtual void					interconnect(ArnNode* sceneRoot) = 0;
	//@}
protected:
									ArnNode(NODE_DATA_TYPE type);
	inline void						setParentName(const char* name);

private:
	std::string						m_name;
	ArnNode*						m_parent;
	std::string						m_parentName;
	ArnNodeList						m_children;
};

#include "ArnNode.inl"
