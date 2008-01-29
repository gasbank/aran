#pragma once

#include "../VideoLib/VideoMan.h"
#include "DirectXStatic.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "animsliderctrl.h"
// CArnView form view


class CArnView : public CFormView
{
	DECLARE_DYNCREATE(CArnView)

protected:
	CArnView();           // protected constructor used by dynamic creation
	virtual ~CArnView();

public:
	enum { IDD = IDD_ARNVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	BOOL m_bReady, m_bRunningThread, m_bIsNowRendering;
	BOOL m_bIsOpeningFile;
	int m_iSelAnimGroupID, m_iSelAnimItemIndex;
	
	CImageList* m_pSmallImage;

	VideoMan m_videoMan;
	CButton m_testButton;
	CDirectXStatic m_dxStatic;


public:
	void Render();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewtestArnview();
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	// Node List in ARN file
	CListCtrl m_listNodes;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnLvnItemchangedNodelist(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_listAnim;
	afx_msg void OnLvnItemchangedAnimlist(NMHDR *pNMHDR, LRESULT *pResult);
	
	CEdit m_editSelGroupName;
	
	CString m_cstrSelGroupName;

	CString m_cstrTestField;
	afx_msg void OnBnClickedBtnChangeGroupName();
	CButton m_btnChangeGroupName;
	afx_msg void OnBnClickedBtnSplitAnim();
	int m_iAnimTime;
	CAnimSliderCtrl m_animSliderTime;
	
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	CTabCtrl m_tcTest;
	afx_msg void OnBnClickedButton1();
};


