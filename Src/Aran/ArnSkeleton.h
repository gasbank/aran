#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeSkeleton1;
struct NodeHierarchy2;
class ArnAction;

class ArnSkeleton : public ArnXformable
{
public:
										~ArnSkeleton(void);
	static ArnSkeleton*					createFrom(const NodeBase* nodeBase);
	static ArnSkeleton*					createFrom(const DOMElement* elm);
	const SkeletonData&					getSkeletonData() const { return m_data; }
	void								render();
	void								configureIpos();
	void								setDefaultActionName(const char* name) { m_actionName = name; }
	const std::string&					getActionName() const { return m_actionName; }
	unsigned int						getChildBoneCount() const;
	void								setActionToNext();
	// *** INTERNAL USE ONLY START ***
	virtual void						interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
protected:
	virtual void						update(double fTime, float fElapsedTime);
private:
										ArnSkeleton(void);
	void								buildFrom(const NodeSkeleton1* ns);
	SkeletonData						m_data;
	std::string							m_actionName;
	ArnAction*							m_defaultAction;
	std::vector<std::string>			m_actionStripNames;
	std::vector<ArnAction*>				m_actionStrips;
};
