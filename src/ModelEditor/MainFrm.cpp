// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ModelEditor.h"

#include "MainFrm.h"

#include "ModelEditorDoc.h"
#include "ModelEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEWTEST_ARNVIEW, &CMainFrame::OnViewtestArnview)
	ON_COMMAND(ID_VIEWTEST_FIRSTVIEW, &CMainFrame::OnViewtestFirstview)
	ON_UPDATE_COMMAND_UI(ID_VIEWTEST_ARNVIEW, &CMainFrame::OnUpdateViewtestArnview)
	ON_UPDATE_COMMAND_UI(ID_VIEWTEST_FIRSTVIEW, &CMainFrame::OnUpdateViewtestFirstview)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pArnView = NULL;
	m_pFirstView = NULL;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);



	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers




void CMainFrame::OnViewtestArnview()
{
	if (GetActiveView() == m_pArnView)
		return;

	if (m_pArnView == NULL)
	{
		CRuntimeClass* pClass;
		pClass = RUNTIME_CLASS(CArnView);
		CCreateContext context;
		context.m_pCurrentDoc = GetActiveDocument();
		context.m_pNewViewClass = pClass;
		m_pArnView = STATIC_DOWNCAST(CView, CreateView(&context));

		m_pArnView->OnInitialUpdate();

	}
	
	
	GetActiveView()->ShowWindow(SW_HIDE);
	m_pFirstView = GetActiveView();
	SetActiveView(m_pArnView);
	m_pArnView->ShowWindow(TRUE);
	RecalcLayout();
}

void CMainFrame::OnViewtestFirstview()
{
	if (GetActiveView() == m_pFirstView || m_pFirstView == NULL)
		return;

	GetActiveView()->ShowWindow(SW_HIDE);	
	SetActiveView(m_pFirstView);
	m_pFirstView->ShowWindow(TRUE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewtestArnview(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateViewtestFirstview(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//pCmdUI->Enable(FALSE);
}
