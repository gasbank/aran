#pragma once

#include "ViewTree.h"

class CMaterialView : public CDockablePane
{
public:
	CMaterialView(void);
	virtual ~CMaterialView(void);

	// Overrides
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()

	CViewTree		m_tree;
	CImageList		m_treeImages;

private:
	void FillDummyData();
};
