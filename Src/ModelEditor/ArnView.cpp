// ArnView.cpp : implementation file
//

#include "stdafx.h"
#include "ModelEditor.h"
#include "ArnView.h"
#include "ModelEditorDoc.h"

// Rendering thread

UINT __cdecl RenderingThreadProc(LPVOID lParam)
{
	CArnView* pView;
	pView = (CArnView*)lParam;

	while (pView->m_bReady && pView->m_bRunningThread)
	{
		pView->Render();
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


// CArnView

IMPLEMENT_DYNCREATE(CArnView, CFormView)

CArnView::CArnView()
	: CFormView(CArnView::IDD), m_bReady(FALSE), m_bRunningThread(FALSE), m_bIsNowRendering(FALSE), m_bIsOpeningFile(FALSE)
	
	, m_cstrSelGroupName(_T(""))

	, m_cstrTestField(_T(""))
	, m_iAnimTime(0)
{
	
}

CArnView::~CArnView()
{
	m_pSmallImage->DeleteImageList();
	SAFE_DELETE(m_pSmallImage);
}

void CArnView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NODELIST, m_listNodes);
	DDX_Control(pDX, IDC_ANIMLIST, m_listAnim);
	DDX_Control(pDX, IDC_EDIT_SEL_GROUP_NAME, m_editSelGroupName);
	DDX_Text(pDX, IDC_EDIT1, m_cstrTestField);
	DDV_MaxChars(pDX, m_cstrTestField, 128);
	DDX_Control(pDX, IDC_BTN_CHANGE_GROUP_NAME, m_btnChangeGroupName);
	DDX_Slider(pDX, IDC_SLIDER_ANIM_TIME, m_iAnimTime);
	DDX_Control(pDX, IDC_SLIDER_ANIM_TIME, m_animSliderTime);
	DDX_Control(pDX, IDC_TAB1, m_tcTest);
}

BEGIN_MESSAGE_MAP(CArnView, CFormView)

	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT1, &CArnView::OnEnChangeEdit1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NODELIST, &CArnView::OnLvnItemchangedNodelist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ANIMLIST, &CArnView::OnLvnItemchangedAnimlist)
	ON_BN_CLICKED(IDC_BTN_CHANGE_GROUP_NAME, &CArnView::OnBnClickedBtnChangeGroupName)
	ON_BN_CLICKED(IDC_BTN_SPLIT_ANIM, &CArnView::OnBnClickedBtnSplitAnim)
	ON_WM_HSCROLL()

	ON_BN_CLICKED(IDC_BUTTON1, &CArnView::OnBnClickedButton1)
END_MESSAGE_MAP()


// CArnView diagnostics

#ifdef _DEBUG
void CArnView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CArnView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG



void CArnView::Render()
{
	if (!m_bIsOpeningFile)
	{
		m_bIsNowRendering = TRUE;
		m_videoMan.DrawAtEditor(m_bReady, m_bRunningThread);
		m_bIsNowRendering = FALSE;
	}
}

void CArnView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	
	if (m_dxStatic.GetSafeHwnd() == NULL)
	{
		// Nodes list icon registration
		m_pSmallImage = new CImageList;

		m_pSmallImage->Create(16, 16, ILC_COLOR4, 3, 3);
		CBitmap cBitmap;
		cBitmap.LoadBitmap(IDB_REDCIRCLE);
		m_pSmallImage->Add(&cBitmap, RGB(255, 255, 255));
		cBitmap.DeleteObject();
		cBitmap.LoadBitmap(IDB_GREENCIRCLE);
		m_pSmallImage->Add(&cBitmap, RGB(255, 255, 255));
		cBitmap.DeleteObject();
		cBitmap.LoadBitmap(IDB_BLUECIRCLE);
		m_pSmallImage->Add(&cBitmap, RGB(255, 255, 255));
		cBitmap.DeleteObject();

		m_listNodes.SetImageList(m_pSmallImage, LVSIL_SMALL);


		// Direct3D area init
		RECT rect1 = { 500, 10, 500 + 400, 10 + 300 };
		m_dxStatic.Create(_T("TestView"), SS_NOTIFY | SS_SUNKEN | WS_BORDER, rect1, this);
		m_dxStatic.ShowWindow(SW_SHOW);

		m_videoMan.SetHwnd(m_dxStatic.GetSafeHwnd());
		m_videoMan.SetWindowSize(400, 300);
		m_videoMan.InitD3D(TRUE);
		m_videoMan.InitAnimationController();
		m_videoMan.InitMainCamera();
		m_videoMan.InitLight();
		m_videoMan.InitShader();
		m_videoMan.InitModelsAtEditor();
		// all rendering devices and data are prepared! okay to run the thread
		m_bReady = TRUE;
		// run thread initially
		m_bRunningThread = TRUE;
		AfxBeginThread(RenderingThreadProc, (LPVOID)this);


		int i;
		LV_COLUMN lvColumn;
		// init node list columns
		TCHAR* columnName[4] = { _T("Type"), _T("Name"), _T("Chunk Offset"), _T("Chunk Size") };
		int columnWidth[4] = { 130, 130, 80, 80 };
		
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		for (i = 0; i < 4; i++)
		{
			lvColumn.pszText = columnName[i];
			lvColumn.iSubItem = i;
			lvColumn.cx = columnWidth[i];
			m_listNodes.InsertColumn(i, &lvColumn);
		}
		m_listNodes.SetExtendedStyle(LVS_EX_FULLROWSELECT);

		// init animation list columns
		TCHAR* columnName2[5] = { _T("Seq"), _T("Time"), _T("Rotation"), _T("Scale"), _T("Translation") };
		int columnWidth2[5] = { 60, 80, 160, 140, 180 };

		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		for (i = 0; i < 5; i++)
		{
			lvColumn.pszText = columnName2[i];
			lvColumn.iSubItem = i;
			lvColumn.cx = columnWidth2[i];
			m_listAnim.InsertColumn(i, &lvColumn);
		}
		m_listAnim.SetExtendedStyle(LVS_EX_FULLROWSELECT);

		// init animation list groups
		m_listAnim.EnableGroupView(TRUE);

		LVGROUP grp;
		ZeroMemory(&grp, sizeof(grp));
		grp.cbSize = sizeof(LVGROUP);
		grp.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
		grp.uAlign = LVGA_HEADER_LEFT;

		TCHAR label[128];
		_tcscpy_s(label, 128, _T("Group 0"));
		grp.pszHeader = label; //_T("Group 0");
		grp.cchHeader = 128; //_tcslen(grp.pszHeader);
		grp.iGroupId = 0;
		m_listAnim.InsertGroup(grp.iGroupId, &grp);

		m_editSelGroupName.EnableWindow(FALSE);
		m_btnChangeGroupName.EnableWindow(FALSE);
	}
}


void CArnView::OnDestroy()
{
	CFormView::OnDestroy();

	// TODO: Add your message handler code here

	m_bReady = FALSE;
	m_bRunningThread = FALSE;

	// should wait while D3D ends its rendering routine
	while (TRUE)
	{
		if (m_bIsNowRendering == FALSE) break;
	}
}

void CArnView::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	GetDocument()->SetModifiedFlag(TRUE);
}

void CArnView::OnLvnItemchangedNodelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	NM_LISTVIEW* p = (NM_LISTVIEW*)pNMHDR;
	
	if (p->uOldState == 0)
	{
		int selected = p->iItem;

		CModelEditorDoc* pDoc = (CModelEditorDoc*)GetDocument();
		ModelReader* pMR = pDoc->m_pMR;

		static wchar_t nodeName[128];
		size_t length = pMR->GetArnNodeHeader(selected).uniqueName.length();
		int j;
		for (j = 0; j < (int)length; j++)
			nodeName[j] = std::cin.widen(pMR->GetArnNodeHeader(selected).uniqueName.c_str()[j]);
		nodeName[length] = L'\0';

		//AfxMessageBox(nodeName);

		RST_DATA* pRST = pMR->GetAnimQuat(selected);
		if (pRST == NULL)
		{
			m_listAnim.DeleteAllItems();
			return; // does not have anim data
		}

		int rstSize = pMR->GetAnimQuatSize(selected);

		LV_ITEM lvItem;
		ZeroMemory(&lvItem, sizeof(lvItem));

		int i;

		m_listAnim.SetRedraw(FALSE);
		m_listAnim.DeleteAllItems();

		CString cstr;

		for (i = 0; i < rstSize; i++)
		{
			lvItem.iItem = i;

			// Sequence
			lvItem.mask = LVIF_TEXT | LVIF_GROUPID;
			lvItem.iGroupId = 0;
			lvItem.iSubItem = 0;
			cstr.Format(_T("%d"), i);
			lvItem.pszText = (LPWSTR)cstr.GetString();
			m_listAnim.InsertItem(&lvItem);
			
			// Time
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 1;
			cstr.Format(_T("%d"), i*4800);
			lvItem.pszText = (LPWSTR)cstr.GetString();
			m_listAnim.SetItem(&lvItem);

			// Rotation
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 2;
			cstr.Format(_T("(%.3f, %.3f, %.3f; %.3f)"), pRST->x, pRST->y, pRST->z, pRST->w);
			lvItem.pszText = (LPWSTR)cstr.GetString();
			m_listAnim.SetItem(&lvItem);

			// Scale
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 3;
			cstr.Format(_T("(%.3f, %.3f, %.3f)"), pRST->sx, pRST->sy, pRST->sz);
			lvItem.pszText = (LPWSTR)cstr.GetString();
			m_listAnim.SetItem(&lvItem);

			// Translation
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 4;
			cstr.Format(_T("(%.3f, %.3f, %.3f)"), pRST->tx, pRST->ty, pRST->tz);
			lvItem.pszText = (LPWSTR)cstr.GetString();
			m_listAnim.SetItem(&lvItem);

			pRST++;
		}

		pRST = NULL;
		m_listAnim.SetRedraw(TRUE);
		m_listAnim.Invalidate();

		m_animSliderTime.SetRange(0, rstSize-1, TRUE);

	}

	*pResult = 0;
}




void CArnView::OnLvnItemchangedAnimlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* p = (NM_LISTVIEW*)pNMHDR;

	if (p->uOldState == 0)
	{
		int selected = p->iItem;

		TCHAR szBuffer[1024];
		DWORD cchBuf(1024);
		LV_ITEM item;
		ZeroMemory(&item, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_GROUPID;
		item.iItem = selected;
		item.iSubItem = 0;
		item.pszText = szBuffer;
		item.cchTextMax = cchBuf;
		m_listAnim.GetItem(&item);
		
		LVGROUP grp;
		ZeroMemory(&grp, sizeof(grp));
		grp.cbSize = sizeof(grp);
		grp.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
		grp.pszHeader = szBuffer;
		grp.cchHeader = cchBuf;
		m_listAnim.GetGroupInfo(item.iGroupId, &grp);

		
		m_editSelGroupName.SetWindowText(grp.pszHeader);

		m_cstrTestField = _T("Group selected");

		m_editSelGroupName.EnableWindow(TRUE);
		m_btnChangeGroupName.EnableWindow(TRUE);
		
		m_iSelAnimGroupID = item.iGroupId;
		m_iSelAnimItemIndex = selected;

	}
	else
	{
		m_editSelGroupName.EnableWindow(FALSE);
		m_btnChangeGroupName.EnableWindow(FALSE);

		m_iSelAnimGroupID = -1;
	}

	*pResult = 0;
}

void CArnView::OnBnClickedBtnChangeGroupName()
{
	// TODO: Add your control notification handler code here
	
	LVGROUP grp;
	ZeroMemory(&grp, sizeof(grp));
	grp.cbSize = sizeof(grp);
	grp.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	grp.pszHeader = NULL;
	grp.cchHeader = 0;
	m_listAnim.GetGroupInfo(m_iSelAnimGroupID, &grp);

	CString cstr;
	m_editSelGroupName.GetWindowText(cstr);
	grp.pszHeader = (LPTSTR)cstr.GetString();
	grp.cchHeader = (int)_tcslen(grp.pszHeader);

	m_listAnim.SetRedraw(FALSE);
	//m_listAnim.RemoveGroup(m_iSelAnimGroupID);
	//m_listAnim.InsertGroup(m_iSelAnimGroupID, &grp);
	m_listAnim.SetGroupInfo(m_iSelAnimGroupID, &grp);

	m_listAnim.SetRedraw(TRUE);
}

void CArnView::OnBnClickedBtnSplitAnim()
{
	// TODO: Add your control notification handler code here
	LVGROUP grp;
	ZeroMemory(&grp, sizeof(grp));
	grp.cbSize = sizeof(LVGROUP);
	grp.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	grp.uAlign = LVGA_HEADER_LEFT;

	int i = 0;
	while (m_listAnim.HasGroup(i) == TRUE)
	{
		i++;
	}
	grp.iGroupId = i;

	CString cstr;
	cstr.Format(_T("Group %d"), grp.iGroupId);
	grp.pszHeader = (LPTSTR)cstr.GetString(); //_T("Group 0");
	grp.cchHeader = (int)_tcslen(grp.pszHeader);
	
	m_listAnim.SetRedraw(FALSE);
	m_listAnim.InsertGroup(grp.iGroupId, &grp);


	TCHAR szBuffer[1024];
	DWORD cchBuf(1024);
	LV_ITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_GROUPID;
	item.iSubItem = 0;
	item.pszText = szBuffer;
	item.cchTextMax = cchBuf;

	for (i = m_iSelAnimItemIndex; i < m_listAnim.GetItemCount(); i++)
	{
		item.iItem = i;
		m_listAnim.GetItem(&item);
		item.iGroupId = grp.iGroupId;
		m_listAnim.SetItem(&item);
	}

	m_listAnim.SetRedraw(TRUE);
	m_listAnim.Invalidate();
}

//void CArnView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	// TODO: Add your message handler code here and/or call default
//
//	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
//}


void CArnView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	// Animation slider shoots WM_HSCROLL when set by TBS_BOTTOM or TBS_TOP
	m_videoMan.SetCurrentFrameIndex(m_animSliderTime.GetPos());

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CArnView::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	
}
