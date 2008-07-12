
// NodeViewerDoc.cpp : implementation of the CNodeViewerDoc class
//

#include "stdafx.h"
#include "NodeViewer.h"

#include "NodeViewerDoc.h"

#include "ArnSceneGraph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNodeViewerDoc

IMPLEMENT_DYNCREATE(CNodeViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CNodeViewerDoc, CDocument)
END_MESSAGE_MAP()


// CNodeViewerDoc construction/destruction

CNodeViewerDoc::CNodeViewerDoc()
{
	// TODO: add one-time construction code here
	m_simpleSG = 0;
	m_afd.m_fileDescriptor = 0;
}

CNodeViewerDoc::~CNodeViewerDoc()
{
	release_arnfile(m_afd);
	delete m_simpleSG;
}

BOOL CNodeViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CNodeViewerDoc serialization

void CNodeViewerDoc::Serialize(CArchive& ar)
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


// CNodeViewerDoc diagnostics

#ifdef _DEBUG
void CNodeViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNodeViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CNodeViewerDoc commands

BOOL CNodeViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	
	load_arnfile(lpszPathName, m_afd);
	m_simpleSG = new ArnSceneGraph(m_afd);

	return TRUE;
}

void CNodeViewerDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	release_arnfile(m_afd);

	CDocument::OnCloseDocument();
}
