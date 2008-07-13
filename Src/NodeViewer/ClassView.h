
#pragma once

#include "ViewTree.h"

class COutputWnd;
class ArnSceneGraph;
class ArnNode;

class CPropertiesWnd;

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CClassView : public CDockablePane
{
public:
	CClassView();
	virtual ~CClassView();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	CClassToolBar m_wndToolBar;
	CViewTree m_wndClassView;
	
	CImageList m_ClassViewImages;
	UINT m_nCurrSort;

	void FillClassView();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClassAddMemberFunction();
	afx_msg void OnClassAddMemberVariable();
	afx_msg void OnClassDefinition();
	afx_msg void OnClassProperties();
	afx_msg void OnNewFolder();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnSort(UINT id);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	afx_msg void OnNodeProperties();

	DECLARE_MESSAGE_MAP()
public:
	void updateSceneGraph(ArnSceneGraph* sg, COutputWnd* outputWnd);
	CViewTree& getWndClassView() { return m_wndClassView; }
	CPropertiesWnd* getPropWnd() const { return m_propWnd; }
	void setPropWnd(CPropertiesWnd* val) { m_propWnd = val; }
private:
	void buildSceneGraph(ArnNode* node, HTREEITEM treeParent);
	COutputWnd* m_outputWnd;
	ArnSceneGraph* m_sg;
	CPropertiesWnd* m_propWnd;
	
};

