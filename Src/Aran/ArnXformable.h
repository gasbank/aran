#pragma once

#include "ArnNode.h"
class ArnIpo;

class ArnXformable : public ArnNode
{
public:
	virtual ~ArnXformable(void);

	const STRING&			getIpoName() { return m_ipoName; }
	const D3DXMATRIX&		getLocalXform() const { return m_localXform; }
	D3DXMATRIX				getFinalXform() const;
protected:
	ArnXformable(NODE_DATA_TYPE ndt);

	ArnIpo*					getIpo() const { return m_ipo; }
	void					setIpo(ArnIpo* val) { m_ipo = val; }
	void					setIpo(const STRING& ipoName);
	void					setIpoName(const char* ipoName) { m_ipoName = ipoName; }
	
	void					setLocalXform(const D3DXMATRIX& localXform) { m_localXform = localXform; }

private:
	ArnIpo*					m_ipo;
	STRING					m_ipoName;
	D3DXMATRIX				m_localXform;
};
