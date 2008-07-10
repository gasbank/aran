#pragma once
#include "ArnNode.h"
struct NodeBase;
struct NodeAnim1;
class ArnAnim1 : public ArnNode
{
public:
	ArnAnim1(void);
	~ArnAnim1(void);

	static ArnNode* createFromNodeBase(const NodeBase* nodeBase);
	
private:
	void setData(const NodeAnim1* na1);

	const NodeAnim1* m_data;
};
