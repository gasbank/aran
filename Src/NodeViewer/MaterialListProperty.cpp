// MaterialListProperty.cpp : implementation file
//

#include "NodeViewerPCH.h"
#include "NodeViewer.h"
#include "MaterialListProperty.h"

#define AFX_PROP_HAS_LIST 0x0001
#define AFX_PROP_HAS_BUTTON 0x0002
#define AFX_PROP_HAS_SPIN 0x0004


// CMaterialListProperty
CMaterialListProperty::CMaterialListProperty( const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr /*= NULL*/, DWORD_PTR dwData /*= 0*/, LPCTSTR lpszEditMask /*= NULL*/, LPCTSTR lpszEditTemplate /*= NULL*/, LPCTSTR lpszValidChars /*= NULL*/ )
: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
}
CMaterialListProperty::~CMaterialListProperty()
{
}


// CMaterialListProperty member functions

void CMaterialListProperty::OnClickButton(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));
	m_bButtonIsDown = TRUE;
	Redraw();

	/*if (dlg.DoModal() == IDOK)
	{
		
	}*/
	AfxMessageBox(_T("Prop button clicked"));

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}

	m_bButtonIsDown = FALSE;
	Redraw();
}