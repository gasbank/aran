
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window

class CClassView;

class COutputList : public CListBox
{
// Construction
public:
	COutputList();

// Implementation
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();
	afx_msg void OnFindInSceneGraph();

	DECLARE_MESSAGE_MAP()
public:
	CClassView* m_wndClassView;
protected:
private:
	HTREEITEM m_prevFound;
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();

// Attributes
protected:
	CFont m_Font;

	CMFCTabCtrl	m_wndTabs;

	COutputList m_wndOutputBuild;
	COutputList m_wndOutputDebug;
	COutputList m_wndOutputFind;

protected:
	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	void insertNodeName(const CString& name, HTREEITEM treeItem);
	void setWndClassView(CClassView* val) { m_wndClassView = val; m_wndOutputBuild.m_wndClassView = val; }
	CClassView* getWndClassView() { return m_wndClassView; }
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFindInSceneGraph();

	DECLARE_MESSAGE_MAP()

private:
	
	CClassView* m_wndClassView;
	
};

