
// NodeViewerView.cpp : implementation of the CNodeViewerView class
//

#include "stdafx.h"
#include "NodeViewer.h"

#include "NodeViewerDoc.h"
#include "NodeViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNodeViewerView

IMPLEMENT_DYNCREATE(CNodeViewerView, CView)

BEGIN_MESSAGE_MAP(CNodeViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CNodeViewerView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CNodeViewerView construction/destruction

CNodeViewerView::CNodeViewerView()
{
	// TODO: add construction code here

}

CNodeViewerView::~CNodeViewerView()
{
}

BOOL CNodeViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CNodeViewerView drawing

void CNodeViewerView::OnDraw(CDC* pDC)
{
	CNodeViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	RECT rect = { 0, 0, 100, 100 };
	pDC->DrawText(_T("Hello?"), &rect, DT_CENTER);
}


// CNodeViewerView printing


void CNodeViewerView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CNodeViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CNodeViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CNodeViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CNodeViewerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CNodeViewerView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CNodeViewerView diagnostics

#ifdef _DEBUG
void CNodeViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CNodeViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CNodeViewerDoc* CNodeViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNodeViewerDoc)));
	return (CNodeViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CNodeViewerView message handlers
