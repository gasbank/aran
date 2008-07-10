#pragma once
#include "ArnNode.h"
struct NodeBase;
class ArnNode;
struct NodeHierarchy;
class ArnHierarchy : public ArnNode
{
public:
	ArnHierarchy(void);
	~ArnHierarchy(void);

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	void setData(const NodeHierarchy* nh);
private:
	const NodeHierarchy* m_data;
};
