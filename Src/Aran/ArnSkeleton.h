#pragma once
#include "ArnNode.h"

struct NodeSkeleton1;
struct NodeBase;

class ArnSkeleton : public ArnNode
{
public:
	ArnSkeleton(void);
	~ArnSkeleton(void);

	void setData(const NodeSkeleton1* ns);

	// factory method
	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);

private:
	const NodeSkeleton1* m_data;
};
