#pragma once

#include "ArnNode.h"
class ArnIpo;

class ArnXformable : public ArnNode
{
public:
	virtual						~ArnXformable(void);

	const STRING&				getIpoName() const { return m_ipoName; }
	const D3DXMATRIX&			getFinalLocalXform() { m_finalLocalXform = m_animLocalXform * m_localXform; return m_finalLocalXform; }
	D3DXMATRIX					getFinalXform();
protected:
								ArnXformable(NODE_DATA_TYPE ndt);

	ArnIpo*						getIpo() const { return m_ipo; }
	void						setIpo(ArnIpo* val) { m_ipo = val; }
	void						setIpo(const STRING& ipoName);
	void						setIpoName(const char* ipoName) { m_ipoName = ipoName; }
	void						setLocalXform(const D3DXMATRIX& localXform) { m_localXform = localXform; }
	virtual void				update(double fTime, float fElapsedTime);
	// *** INTERNAL USE ONLY START ***
	void						configureAnimCtrl();
	// *** INTERNAL USE ONLY END ***
private:
	ArnIpo*						m_ipo;
	STRING						m_ipoName;
	D3DXMATRIX					m_localXform;
	D3DXMATRIX					m_animLocalXform;
	D3DXMATRIX					m_finalLocalXform;

	LPD3DXANIMATIONCONTROLLER	m_d3dxAnimCtrl;
};
