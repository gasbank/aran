#pragma once
#include "ArnNode.h"
struct NodeSkeleton;
struct NodeBase;

class ArnSkeleton : public ArnNode
{
public:
	ArnSkeleton(void);
	~ArnSkeleton(void);

	void setData(const NodeSkeleton* ns);

	// factory method
	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);

private:
	const NodeSkeleton* m_data;
};
