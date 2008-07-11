#pragma once
#include "ArnNode.h"
struct NodeBase;
class ArnNode;
struct NodeHierarchy1;
class ArnHierarchy : public ArnNode
{
public:
	ArnHierarchy(void);
	~ArnHierarchy(void);

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	void setData(const NodeHierarchy1* nh);
private:
	const NodeHierarchy1* m_data;
};
