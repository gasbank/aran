#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeAnim1;

class ArnAnim : public ArnNode
{
public:
	ArnAnim(void);
	~ArnAnim(void);

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	
private:
	void setData(const NodeAnim1* na1);

	const NodeAnim1* m_data;
};
