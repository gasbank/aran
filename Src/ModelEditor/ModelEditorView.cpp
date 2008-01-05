// ModelEditorView.cpp : implementation of the CModelEditorView class
//

#include "stdafx.h"
#include "ModelEditor.h"

#include "ModelEditorDoc.h"
#include "ModelEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CModelEditorView

IMPLEMENT_DYNCREATE(CModelEditorView, CView)

BEGIN_MESSAGE_MAP(CModelEditorView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_BN_CLICKED(100, &OnButtonDown)

END_MESSAGE_MAP()

// CModelEditorView construction/destruction


CModelEditorView::CModelEditorView()
{
	// TODO: add construction code here

}

CModelEditorView::~CModelEditorView()
{
}

BOOL CModelEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CModelEditorView drawing

void CModelEditorView::OnDraw(CDC* /*pDC*/)
{
	CModelEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CModelEditorView printing

BOOL CModelEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CModelEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CModelEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CModelEditorView diagnostics

#ifdef _DEBUG
void CModelEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CModelEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CModelEditorDoc* CModelEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CModelEditorDoc)));
	return (CModelEditorDoc*)m_pDocument;
}
#endif //_DEBUG


// CModelEditorView message handlers

void CModelEditorView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	if (m_btnTest.GetSafeHwnd() == NULL)
	{
		RECT rect = { 10, 10, 200, 50 };
		m_btnTest.Create(_T("Custom Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, this, 100);
		m_btnTest.ShowWindow(SW_SHOW);
	}

	if (m_lbTest.GetSafeHwnd() == NULL)
	{
		RECT rect1 = { 10, 100, 200, 200 };
		m_lbTest.Create(WS_CHILD | WS_VISIBLE | LBS_STANDARD, rect1, this, 200);
		m_lbTest.ShowWindow(SW_SHOW);
		m_lbTest.AddString(_T("data1"));
		m_lbTest.AddString(_T("data2"));
		m_lbTest.AddString(_T("data3"));
	}
	
	if (m_editTest.GetSafeHwnd() == NULL)
	{
		RECT rect2 = { 210, 10, 400, 100 };
		m_editTest.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_BORDER | WS_VSCROLL, rect2, this, 300);
		m_editTest.ShowWindow(SW_SHOW);
	}
}



void CModelEditorView::OnButtonDown()
{
	AfxMessageBox(_T("Button is clicked"));
}
