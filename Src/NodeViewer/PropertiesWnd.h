
#pragma once

class ArnNode;
class ArnMesh;
class ArnCamera;
class ArnAnim;
class ArnSkeleton;
class ArnBone;
class ArnHierarchy;

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

protected:
	CFont m_fntPropList;
	CComboBox m_wndObjectCombo;
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;

// Implementation
public:
	virtual ~CPropertiesWnd();
	void updateNodeProp(ArnNode* node);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();

private:
	void hideAllPropGroup();
	void updateNodeProp(ArnMesh* node);
	void updateNodeProp(ArnCamera* node);
	void updateNodeProp(ArnAnim* node);
	void updateNodeProp(ArnSkeleton* node);
	void updateNodeProp(ArnBone* node);
	void updateNodeProp(ArnHierarchy* node);

	CMFCPropertyGridProperty* m_nodeBaseGroup;
	CMFCPropertyGridProperty* m_cameraGroup;
	CMFCPropertyGridProperty* m_animGroup;
	CMFCPropertyGridProperty* m_meshGroup;
	CMFCPropertyGridProperty* m_hierarchyGroup;
	CMFCPropertyGridProperty* m_skelGroup;
	CMFCPropertyGridProperty* m_boneGroup;
};

