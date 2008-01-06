// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ArnView.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

	CView* m_pFirstView;
	CView* m_pArnView;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewtestArnview();
	afx_msg void OnViewtestFirstview();
	afx_msg void OnUpdateViewtestArnview(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewtestFirstview(CCmdUI *pCmdUI);
};


