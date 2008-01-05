#pragma once


// CAnimSliderCtrl

class CAnimSliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CAnimSliderCtrl)

public:
	CAnimSliderCtrl();
	virtual ~CAnimSliderCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:

	CBrush m_brush;
	CPen m_pen;

protected:
	virtual void PreSubclassWindow();
public:
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};


