// ModelEditorDoc.h : interface of the CModelEditorDoc class
//


#pragma once

#include "../Aran/ModelReader.h"

class CModelEditorDoc : public CDocument
{
protected: // create from serialization only
	CModelEditorDoc();
	DECLARE_DYNCREATE(CModelEditorDoc)

// Attributes
public:
	ModelReader* m_pMR;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CModelEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};


