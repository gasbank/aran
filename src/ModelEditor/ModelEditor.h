// ModelEditor.h : main header file for the ModelEditor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CModelEditorApp:
// See ModelEditor.cpp for the implementation of this class
//

class CModelEditorApp : public CWinApp
{
public:
	CModelEditorApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
//	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
};

extern CModelEditorApp theApp;