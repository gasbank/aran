
#pragma once

class ArnNode;
class ArnMesh;
class ArnCamera;
class ArnAnim;
class ArnSkeleton;
class ArnBone;
class ArnHierarchy;
class ArnMaterial;
class ArnLight;
class ArnIpo;

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
	void updateNodeProp(ArnMaterial* node);
	void updateNodeProp(ArnLight* node);
	void updateNodeProp(ArnIpo* node);
	void writePrecomputedCurvesToFile(ArnIpo* node);

	CMFCPropertyGridProperty* m_nodeBaseGroup;
	CMFCPropertyGridProperty* m_cameraGroup;
	CMFCPropertyGridProperty* m_animGroup;
	CMFCPropertyGridProperty* m_meshGroup;
	CMFCPropertyGridProperty* m_hierarchyGroup;
	CMFCPropertyGridProperty* m_skelGroup;
	CMFCPropertyGridProperty* m_boneGroup;
	CMFCPropertyGridProperty* m_materialGroup;
	CMFCPropertyGridProperty* m_lightGroup;
	CMFCPropertyGridProperty* m_ipoGroup;


	enum PROP_ENUM
	{
		PROP_BASE_NDT,
		PROP_BASE_NAME,
		PROP_BASE_PARENT,
		PROP_BASE_LX_ROT, PROP_BASE_LX_QUAT, PROP_BASE_LX_SCALING, PROP_BASE_LX_TRANS,

		PROP_CAM_TARGETPOS,
		PROP_CAM_UPVEC,
		PROP_CAM_LOOKATVEC,
		PROP_CAM_FARCLIP,
		PROP_CAM_NEARCLIP,

		PROP_ANIM_KEYCOUNT,

		PROP_MESH_VERTCOUNT,
		PROP_MESH_FACECOUNT,
		PROP_MESH_MATERIALCOUNT,

		PROP_SKEL_MAXWEIGHTSPERVERT,
		PROP_SKEL_BONECOUNT,

		PROP_BONE_OFF_ROT, PROP_BONE_OFF_QUAT, PROP_BONE_OFF_SCALING, PROP_BONE_OFF_TRANS,
		PROP_BONE_INFVERTCOUNT,

		PROP_MAT_COUNT,
		PROP_MAT_DIFFUSE, PROP_MAT_AMBIENT, PROP_MAT_SPECULAR, PROP_MAT_EMISSIVE, PROP_MAT_POWER,

		PROP_LIGHT_TYPE,
		PROP_LIGHT_DIFFUSE, PROP_LIGHT_SPECULAR, PROP_LIGHT_AMBIENT,
		PROP_LIGHT_POS,
		PROP_LIGHT_DIR,
		PROP_LIGHT_RANGE,
		PROP_LIGHT_FALLOFF,
		PROP_LIGHT_ATT0,
		PROP_LIGHT_ATT1,
		PROP_LIGHT_ATT2,
		PROP_LIGHT_THETA,
		PROP_LIGHT_PHI,

		PROP_IPO_COUNT,
		PROP_IPO_CURVECOUNT,
		PROP_IPO_CURVENAMES,
	};

	// Prop SetValue() helper methods

	void propEnumSetValue(PROP_ENUM pe, const POINT3FLOAT& p3f);
	void propEnumSetValue(PROP_ENUM pe, float f);
	void propEnumSetValue(PROP_ENUM pe, unsigned int ui);
	void propEnumSetValue(PROP_ENUM pe, CString& cstr);
	void propEnumSetValue(PROP_ENUM pe, const D3DVECTOR& d3dVec);
	void propEnumSetValue(PROP_ENUM pe, const D3DXVECTOR3& d3dVec);
	void propEnumSetValue(PROP_ENUM pe, const D3DCOLORVALUE& d3dColVal);
	void propEnumSetValue(PROP_ENUM pe, const D3DXQUATERNION& quat);
	void propEnumSetValue(PROP_ENUM pe, const STRING& arnStr);

};

