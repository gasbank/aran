#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeIpo1;
struct NodeIpo2;

class ArnIpo : public ArnNode
{
public:
	ArnIpo(void);
	~ArnIpo(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);

private:
	void				buildFrom(const NodeIpo1* ni);
	void				buildFrom(const NodeIpo2* ni);

	unsigned int m_ipoCount;
	unsigned int m_curveCount;
	//std::vector<CurveDataShell> m_curves;
};
