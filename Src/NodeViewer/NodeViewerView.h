
// NodeViewerView.h : interface of the CNodeViewerView class
//


#pragma once


class CNodeViewerView : public CView
{
protected: // create from serialization only
	CNodeViewerView();
	DECLARE_DYNCREATE(CNodeViewerView)

// Attributes
public:
	CNodeViewerDoc* GetDocument() const;

// Operations
public:
	void updateSceneGraph();
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CNodeViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	CString m_drawUpdatedText;
};

#ifndef _DEBUG  // debug version in NodeViewerView.cpp
inline CNodeViewerDoc* CNodeViewerView::GetDocument() const
   { return reinterpret_cast<CNodeViewerDoc*>(m_pDocument); }
#endif

