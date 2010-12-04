#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeAnim1;

class ArnAnim : public ArnNode
{
public:
	ArnAnim(void);
	~ArnAnim(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);
	unsigned int		getKeyCount() { return m_data.size(); }

	// *** INTERNAL USE ONLY START ***
	virtual void			interconnect(ArnNode* sceneRoot) { ArnNode::interconnect(sceneRoot); }
	// *** INTERNAL USE ONLY END ***
private:
	void				buildFrom(const NodeAnim1* na);

	std::vector<RST_DATA> m_data;
};
