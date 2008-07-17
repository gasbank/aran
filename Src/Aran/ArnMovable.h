#pragma once

#include "ArnNode.h"
class ArnIpo;

class ArnMovable : public ArnNode
{
public:
	virtual ~ArnMovable(void);
	const STRING&		getIpoName() { return m_ipoName; }

protected:
	ArnMovable(NODE_DATA_TYPE ndt);

	ArnIpo*				getIpo() const { return m_ipo; }
	void				setIpo(ArnIpo* val) { m_ipo = val; }
	void				setIpo(const STRING& ipoName);
	void				setIpoName(const char* ipoName) { m_ipoName = ipoName; }

private:
	ArnIpo*				m_ipo;
	STRING				m_ipoName;
};
