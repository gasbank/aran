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
	
private:
	void				buildFrom(const NodeAnim1* na);

	std::vector<RST_DATA> m_data;
};
