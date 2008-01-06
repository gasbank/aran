// ModelEditorDoc.cpp : implementation of the CModelEditorDoc class
//

#include "stdafx.h"
#include "ModelEditor.h"

#include "ModelEditorDoc.h"
#include "MainFrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CModelEditorDoc

IMPLEMENT_DYNCREATE(CModelEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CModelEditorDoc, CDocument)
END_MESSAGE_MAP()


// CModelEditorDoc construction/destruction

CModelEditorDoc::CModelEditorDoc(): m_pMR(NULL)
{
	// TODO: add one-time construction code here
}

CModelEditorDoc::~CModelEditorDoc()
{
	SAFE_DELETE(m_pMR);
}

BOOL CModelEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	// No 'New' document support. Just stop rendering;
	CMainFrame* mf = (CMainFrame*)AfxGetMainWnd();
	if ( mf != NULL )
	{
		CArnView* arnView = (CArnView*)mf->GetActiveView();
		if ( arnView != NULL )
		{
			arnView->m_bIsOpeningFile = TRUE;
		}
	}

	// TODO: clear all GUI filling


	SAFE_DELETE(m_pMR);

	return TRUE;
}




// CModelEditorDoc serialization

void CModelEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CModelEditorDoc diagnostics

#ifdef _DEBUG
void CModelEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CModelEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CModelEditorDoc commands

BOOL CModelEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	//AfxMessageBox(lpszPathName);

	CMainFrame* mf = (CMainFrame*)AfxGetMainWnd();
//	CArnView* arnView = (CArnView*)mf->m_pArnView;
	CArnView* arnView = (CArnView*)mf->GetActiveView();
	
	arnView->m_bIsOpeningFile = TRUE;
	while (arnView->m_bIsNowRendering == TRUE); // sync since m_pMR will be referenced by VideoMan module
	

	SAFE_DELETE(m_pMR);
	m_pMR = new ModelReader;

	m_pMR->SetDev(arnView->m_videoMan.GetDev());
	
	/*m_pMR->Initialize(arnView->m_videoMan.GetDev(), ARN_VDD::ARN_VDD_FVF, NULL,
		lpszPathName, arnView->m_videoMan.GetAnimationController(), TRUE);*/

	m_pMR->Initialize(arnView->m_videoMan.GetDev(), ARN_VDD::ARN_VDD_FVF, NULL,
		lpszPathName, NULL);
	
	ARN_NDD_CAMERA_CHUNK* pCamChunk = m_pMR->GetFirstCamera();
	if ( pCamChunk != NULL )
	{
		arnView->m_videoMan.SetCamera( pCamChunk );
	}

	arnView->m_videoMan.SetDrawingModelAtEditor(m_pMR);
	
	
	arnView->m_bIsOpeningFile = FALSE; // ARN file opening finished. It is ok to start rendering in VideoMan

	LV_ITEM lvItem;
	int i, j;
	if (arnView != NULL)
	{
		arnView->m_listNodes.SetRedraw(FALSE);
		arnView->m_listNodes.DeleteAllItems();

		size_t nodeHeadersSize = m_pMR->GetArnNodeHeadersSize();
		for (i = 0; i < (int)nodeHeadersSize; i++)
		{
			lvItem.iItem = i;
			lvItem.iImage = i%3;
			lvItem.iIndent = 0;

			// Type
			lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
			lvItem.iSubItem = 0;
			switch (m_pMR->GetArnNodeHeader(i).ndt)
			{
#define CASE_ENUM_TO_STRING(e,sz) case e: {sz = _T(#e); }
				CASE_ENUM_TO_STRING(NDT_MESH1, lvItem.pszText)
					break;
				CASE_ENUM_TO_STRING(NDT_MESH2, lvItem.pszText)
					break;
				CASE_ENUM_TO_STRING(NDT_SKELETON, lvItem.pszText)
					lvItem.iIndent++;
					break;
				CASE_ENUM_TO_STRING(NDT_HIERARCHY, lvItem.pszText)
					break;
				CASE_ENUM_TO_STRING(NDT_LIGHT, lvItem.pszText)
					break;
				CASE_ENUM_TO_STRING(NDT_CAMERA, lvItem.pszText)
					break;
#undef CASE_ENUM_TO_STRING
			}
			arnView->m_listNodes.InsertItem(&lvItem);

			// Name
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 1;
			static wchar_t wc[128];
			size_t length = m_pMR->GetArnNodeHeader(i).uniqueName.length();
			for (j = 0; j < (int)length; j++)
				wc[j] = std::cin.widen(m_pMR->GetArnNodeHeader(i).uniqueName.c_str()[j]);
			wc[length] = L'\0';

			//mbtowc(wc, m_pMR->GetArnNodeHeader(i).uniqueName.c_str(), m_pMR->GetArnNodeHeader(i).uniqueName.length());
			lvItem.pszText = wc;
			arnView->m_listNodes.SetItem(&lvItem);

			// Chunk Start
			lvItem.iSubItem = 2;
			CString cstr;
			
			cstr.Format(_T("%d"), m_pMR->GetArnNodeHeader(i).chunkStartPos);
			lvItem.pszText = (LPTSTR)cstr.GetString();
			arnView->m_listNodes.SetItem(&lvItem);

			// Chunk Size
			lvItem.iSubItem = 3;
			cstr.Format(_T("%d"), m_pMR->GetArnNodeHeader(i).chunkSize);
			lvItem.pszText = (LPTSTR)cstr.GetString();
			arnView->m_listNodes.SetItem(&lvItem);
		}

		arnView->m_listNodes.SetRedraw(TRUE);
		arnView->m_listNodes.Invalidate();
	}

	return TRUE;
}

BOOL CModelEditorDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::OnSaveDocument(lpszPathName);
}
