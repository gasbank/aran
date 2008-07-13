#include "StdAfx.h"
#include "MaterialView.h"
#include "MainFrm.h"
#include "Resource.h"
#include "NodeViewer.h"

CMaterialView::CMaterialView(void)
{
}

CMaterialView::~CMaterialView(void)
{
}
BEGIN_MESSAGE_MAP(CMaterialView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////


int CMaterialView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_tree.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	OnChangeVisualStyle();
	
	FillDummyData();
	return 0;
}

void CMaterialView::OnChangeVisualStyle()
{
	m_treeImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_MATERIAL_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_treeImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_treeImages.Add(&bmp, RGB(255, 0, 0));

	m_tree.SetImageList(&m_treeImages, TVSIL_NORMAL);
}


void CMaterialView::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();

}

void CMaterialView::OnContextMenu( CWnd* pWnd, CPoint point )
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_tree;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_MATERIAL);

	CMenu* pSubMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSubMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

BOOL CMaterialView::PreTranslateMessage( MSG* pMsg )
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CMaterialView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	CRect rectClient;
	GetClientRect(rectClient);
	int cyTlb = 0;
	m_tree.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CMaterialView::FillDummyData()
{
	HTREEITEM hRoot;
	hRoot = m_tree.InsertItem(_T("ARN Scene Graph Root"), 0, 0);
	m_tree.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	hRoot = m_tree.InsertItem(_T("ARN Scene Graph Root"), 0, 0);

	hRoot = m_tree.InsertItem(_T("ARN Scene Graph Root"), 0, 0);
	m_tree.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
}