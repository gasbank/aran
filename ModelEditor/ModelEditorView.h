// ModelEditorView.h : interface of the CModelEditorView class
//


#pragma once

class CModelEditorView : public CView
{
protected: // create from serialization only
	CModelEditorView();
	DECLARE_DYNCREATE(CModelEditorView)

// Attributes
public:
	CModelEditorDoc* GetDocument() const;

// Operations
public:

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
	virtual ~CModelEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
protected:
	CButton m_btnTest;
	CListBox m_lbTest;
	CEdit m_editTest;

	afx_msg void OnButtonDown();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

};

#ifndef _DEBUG  // debug version in ModelEditorView.cpp
inline CModelEditorDoc* CModelEditorView::GetDocument() const
   { return reinterpret_cast<CModelEditorDoc*>(m_pDocument); }
#endif

