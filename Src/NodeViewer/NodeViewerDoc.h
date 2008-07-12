
// NodeViewerDoc.h : interface of the CNodeViewerDoc class
//


#pragma once


class CNodeViewerDoc : public CDocument
{
protected: // create from serialization only
	CNodeViewerDoc();
	DECLARE_DYNCREATE(CNodeViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CNodeViewerDoc();
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
};


