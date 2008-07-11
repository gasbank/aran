#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeBone1;

class ArnBone : public ArnNode
{
public:
	ArnBone(void);
	~ArnBone(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);

private:
	void				buildFrom(const NodeBone1* nb);
};
