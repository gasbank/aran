// AnimSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ModelEditor.h"
#include "AnimSliderCtrl.h"
#include "ArnView.h"

// CAnimSliderCtrl

IMPLEMENT_DYNAMIC(CAnimSliderCtrl, CSliderCtrl)

CAnimSliderCtrl::CAnimSliderCtrl()
{
	m_brush.CreateSolidBrush( RGB(255,0,0) );
	m_pen.CreatePen( PS_SOLID, 1, RGB(128,128,128) ); // dark gray
}

CAnimSliderCtrl::~CAnimSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CAnimSliderCtrl, CSliderCtrl)

	ON_WM_DRAWITEM()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CAnimSliderCtrl::OnNMCustomdraw)
END_MESSAGE_MAP()



// CAnimSliderCtrl message handlers

void CAnimSliderCtrl::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	//ModifyStyle(0, BS_OWNERDRAW);

	CSliderCtrl::PreSubclassWindow();
}

void CAnimSliderCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	//CDC* pDC   = CDC::FromHandle(lpDrawItemStruct->hDC);
	//CRect rect = lpDrawItemStruct->rcItem;
	//UINT state = lpDrawItemStruct->itemState;
	//rect.DeflateRect( CSize(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE)));
	//pDC->FillSolidRect(rect, RGB(255, 255, 0)); // yellow

	CSliderCtrl::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CAnimSliderCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here

	switch ( pNMCD->dwDrawStage )
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW ;
		break;
	case CDDS_ITEMPREPAINT:
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		if ( pNMCD->dwItemSpec == TBCD_THUMB )
		{
			CDC* pDC = CDC::FromHandle( pNMCD->hdc );
			pDC->SelectObject( m_brush );
			pDC->SelectObject( m_pen );
			pDC->Ellipse( &(pNMCD->rc) );
			pDC->Detach();
			*pResult = CDRF_SKIPDEFAULT;
		}
		break;
	default:
		break;
	}

	/**pResult = 0;
	*pResult |= CDRF_NOTIFYPOSTPAINT;
	*pResult |= CDRF_NOTIFYITEMDRAW;
	*pResult |= CDRF_NOTIFYSUBITEMDRAW;*/
}
