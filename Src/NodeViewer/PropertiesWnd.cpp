
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "NodeViewer.h"
//////////////////////////////////////////////////////////////////////////
#include "ArnNode.h"
#include "ArnContainer.h"
#include "ArnMesh.h"
#include "ArnSkeleton.h"
#include "ArnHierarchy.h"
#include "ArnLight.h"
#include "ArnCamera.h"
#include "ArnAnim.h"
#include "ArnBone.h"
#include "ArnMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)

ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndObjectCombo.GetWindowRect(&rectCombo);

	int cyCmb = rectCombo.Size().cy;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties Window"));
	m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	m_wndObjectCombo.SetCurSel(0);

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	//////////////////////////////////////////////////////////////////////////

	m_nodeBaseGroup = new CMFCPropertyGridProperty(_T("NodeBase"));

	CMFCPropertyGridProperty* pPropNDT = new CMFCPropertyGridProperty(_T("NODE_DATA_TYPE"), _T("NDT_RT_CONTAINER"), _T("ARN Node data type enum"));
	pPropNDT->AddOption(_T("NDT_RT_CONTAINER"));
	pPropNDT->AddOption(_T("NDT_RT_MESH"));
	pPropNDT->AddOption(_T("NDT_RT_CAMERA"));
	pPropNDT->AddOption(_T("NDT_RT_LIGHT"));
	pPropNDT->AddOption(_T("NDT_RT_ANIM"));
	pPropNDT->AddOption(_T("NDT_RT_MATERIAL"));
	pPropNDT->AddOption(_T("NDT_RT_HIERARCHY"));
	pPropNDT->AddOption(_T("NDT_RT_SKELETON"));
	pPropNDT->AddOption(_T("NDT_RT_BONE"));
	pPropNDT->AllowEdit(FALSE);
	//pPropNDT->Enable(FALSE);
	pPropNDT->SetData((DWORD_PTR)"NDT");
	m_nodeBaseGroup->AddSubItem(pPropNDT);

	m_nodeBaseGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Name"), (_variant_t) _T("Node Name"), _T("Node unique name"), (DWORD_PTR)"NAME"));
	m_nodeBaseGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Parent"), (_variant_t) _T("Parent Name"), _T("Current node's parent name"), (DWORD_PTR)"PARENT NAME"));
	
	CMFCPropertyGridProperty* pProp = 0;
	
	m_wndPropList.AddProperty(m_nodeBaseGroup);

	CMFCPropertyGridProperty* localXformProp = new CMFCPropertyGridProperty(_T("Local Transform"));
	m_nodeBaseGroup->AddSubItem(localXformProp);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Deg)"), (_variant_t) _T("(0, 0, 0)"), _T("Local rotation in Euler form (Unit: Degrees)"), (DWORD_PTR)"LX ROTATION");
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Scaling"), (_variant_t) _T("(0, 0, 0)"), _T("Local scaling"), (DWORD_PTR)"LX SCALING");
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Translation"), (_variant_t) _T("(0, 0, 0)"), _T("Local translation"), (DWORD_PTR)"LX TRANSLATION");
	localXformProp->AddSubItem(pProp);

	//m_wndPropList.AddProperty(localXformProp);
	

	/*pProp = new CMFCPropertyGridProperty(_T("Border"), _T("Dialog Frame"), _T("One of: None, Thin, Resizable, or Dialog Frame"));
	pProp->AddOption(_T("None"));
	pProp->AddOption(_T("Thin"));
	pProp->AddOption(_T("Resizable"));
	pProp->AddOption(_T("Dialog Frame"));
	pProp->AllowEdit(FALSE);

	nodeBaseGroup->AddSubItem(pProp);
	nodeBaseGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Caption"), (_variant_t) _T("About"), _T("Specifies the text that will be displayed in the window's title bar")));*/

	
	//////////////////////////////////////////////////////////////////////
	
	m_cameraGroup = new CMFCPropertyGridProperty(_T("Camera"));

	pProp = new CMFCPropertyGridProperty( _T("Target Position"), (_variant_t) _T("(0, 0, 0)"), _T("Camera target position"), (DWORD_PTR)"CAM TARGETPOS");
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Up Vector"), (_variant_t) _T("(0, 0, 0)"), _T("Camera up vector"), (DWORD_PTR)"CAM UPVECTOR");
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("LookAt Vector"), (_variant_t) _T("(0, 0, 0)"), _T("Camera look at vector"), (DWORD_PTR)"CAM LOOKATVECTOR");
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Far Clip"), (_variant_t) 0.0f, _T("Camera far clip"), (DWORD_PTR)"CAM FARCLIP");
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Near Clip"), (_variant_t) 0.0f, _T("Camera near clip"), (DWORD_PTR)"CAM NEARCLIP");
	m_cameraGroup->AddSubItem(pProp);

	/*LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, _T("Arial"));

	m_cameraGroup->AddSubItem(new CMFCPropertyGridFontProperty(_T("Font"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("Specifies the default font for the window")));
	m_cameraGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Use System Font"), (_variant_t) true, _T("Specifies that the window uses MS Shell Dlg font")));*/

	m_wndPropList.AddProperty(m_cameraGroup);
	m_cameraGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_animGroup = new CMFCPropertyGridProperty(_T("Animation"));
	
	/*pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("Application"));
	pProp->Enable(FALSE);
	m_animGroup->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	m_animGroup->AddSubItem(pColorProp);

	static TCHAR BASED_CODE szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	m_animGroup->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));

	m_animGroup->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));*/

	pProp = new CMFCPropertyGridProperty( _T("Key Count"), (_variant_t)(unsigned int) 0, _T("Animation RST key-frame count"), (DWORD_PTR)"ANIM KEYCOUNT");
	m_animGroup->AddSubItem(pProp);


	m_wndPropList.AddProperty(m_animGroup);
	m_animGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_meshGroup = new CMFCPropertyGridProperty(_T("Mesh"));

	pProp = new CMFCPropertyGridProperty( _T("Vertex Count"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), (DWORD_PTR)"MESH VERTCOUNT");
	m_meshGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Face Count"), (_variant_t)(unsigned int) 0, _T("Mesh face count"), (DWORD_PTR)"MESH FACECOUNT");
	m_meshGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Material Count"), (_variant_t)(unsigned int) 0, _T("Mesh material count"), (DWORD_PTR)"MESH MATERIALCOUNT");
	m_meshGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_meshGroup);
	m_meshGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_hierarchyGroup = new CMFCPropertyGridProperty(_T("Hierarchy"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("First sub-level"));
	m_hierarchyGroup->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("Second sub-level"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 1"), (_variant_t) _T("Value 1"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 2"), (_variant_t) _T("Value 2"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 3"), (_variant_t) _T("Value 3"), _T("This is a description")));

	//hierarchyGroup->Expand(FALSE);
	m_wndPropList.AddProperty(m_hierarchyGroup);
	m_hierarchyGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_skelGroup = new CMFCPropertyGridProperty(_T("Skeleton"));

	pProp = new CMFCPropertyGridProperty( _T("Max Weights Per Vertex"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), (DWORD_PTR)"SKEL MAXWEIGHTSPERVERT");
	m_skelGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Bone Count"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), (DWORD_PTR)"SKEL BONECOUNT");
	m_skelGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_skelGroup);
	m_skelGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_boneGroup = new CMFCPropertyGridProperty(_T("Bone"));

	CMFCPropertyGridProperty* boneOffsetMatrix = new CMFCPropertyGridProperty(_T("Offset Matrix"));
	m_boneGroup->AddSubItem(boneOffsetMatrix);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Deg)"), (_variant_t) _T("(0, 0, 0)"), _T("Rotation in Euler form (Unit: Degrees)"), (DWORD_PTR)"BONE ROTATION");
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Scaling"), (_variant_t) _T("(0, 0, 0)"), _T("Scaling"), (DWORD_PTR)"BONE SCALING");
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Translation"), (_variant_t) _T("(0, 0, 0)"), _T("Translation"), (DWORD_PTR)"BONE TRANSLATION");
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Inf Vertex Count"), (_variant_t)(unsigned int) 0, _T("The influenced vertex count by this bone"), (DWORD_PTR)"BONE INFVERTCOUNT");
	m_boneGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_boneGroup);
	m_boneGroup->Show(FALSE);

}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

void CPropertiesWnd::updateNodeProp( ArnNode* node )
{
	//CMFCPropertyGridProperty* prop = m_wndPropList.GetProperty(0);
	CMFCPropertyGridProperty* ndtProp = m_wndPropList.FindItemByData((DWORD_PTR)"NDT");
	CString ndtVal;
	hideAllPropGroup();
	switch (node->getType())
	{
	case NDT_RT_MESH:
		ndtVal = _T("NDT_RT_MESH");
		m_meshGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnMesh*>(node));
		break;
	case NDT_RT_ANIM:
		ndtVal = _T("NDT_RT_ANIM");
		m_animGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnAnim*>(node));
		break;
	case NDT_RT_SKELETON:
		ndtVal = _T("NDT_RT_SKELETON");
		m_skelGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnSkeleton*>(node));
		break;
	case NDT_RT_BONE:
		ndtVal = _T("NDT_RT_BONE");
		m_boneGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnBone*>(node));
		break;
	case NDT_RT_CAMERA:
		ndtVal = _T("NDT_RT_CAMERA");
		m_cameraGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnCamera*>(node));
		break;
	case NDT_RT_HIERARCHY:
		ndtVal = _T("NDT_RT_HIERARCHY");
		m_hierarchyGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnHierarchy*>(node));
		break;
	default:
		ndtVal = _T("NDT_RT_CONTAINER");
		break;
	}
	ndtProp->SetValue(ndtVal);

	CMFCPropertyGridProperty* ndtName = m_wndPropList.FindItemByData((DWORD_PTR)"NAME");
	ndtName->SetValue(CString(node->getName()));

	CMFCPropertyGridProperty* parentNameProp = m_wndPropList.FindItemByData((DWORD_PTR)"PARENT NAME");
	parentNameProp->SetValue(CString(node->getParentName().c_str()));
	
	const D3DXMATRIX& localXform = node->getLocalXform();
	D3DXVECTOR3 vecScaling, vecTranslation;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&vecScaling, &quat, &vecTranslation, &localXform);

	D3DXVECTOR3 euler = ArnMath::QuatToEuler(&quat);
	euler = ArnMath::Vec3RadToDeg(&euler);

	CString xformStr;
	CMFCPropertyGridProperty* localXformProp;

	xformStr.Format(_T("(%.2f %.2f %.2f)"), euler.x, euler.y, euler.z);
	localXformProp = m_wndPropList.FindItemByData((DWORD_PTR)"LX ROTATION");
	localXformProp->SetValue(xformStr);

	xformStr.Format(_T("(%.2f %.2f %.2f)"), vecScaling.x, vecScaling.y, vecScaling.z);
	localXformProp = m_wndPropList.FindItemByData((DWORD_PTR)"LX SCALING");
	localXformProp->SetValue(xformStr);

	xformStr.Format(_T("(%.2f %.2f %.2f)"), vecTranslation.x, vecTranslation.y, vecTranslation.z);
	localXformProp = m_wndPropList.FindItemByData((DWORD_PTR)"LX TRANSLATION");
	localXformProp->SetValue(xformStr);

	//////////////////////////////////////////////////////////////////////////

}

void CPropertiesWnd::updateNodeProp( ArnMesh* node )
{
	CMFCPropertyGridProperty* prop;

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"MESH VERTCOUNT");
	prop->SetValue( (_variant_t) node->getMeshData().vertexCount);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"MESH FACECOUNT");
	prop->SetValue( (_variant_t) node->getMeshData().faceCount);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"MESH MATERIALCOUNT");
	prop->SetValue( (_variant_t) node->getMeshData().materialCount);
}

void CPropertiesWnd::updateNodeProp( ArnCamera* node )
{
	CMFCPropertyGridProperty* prop;
	CString str;

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"CAM FARCLIP");
	prop->SetValue(node->getCameraData().farClip);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"CAM NEARCLIP");
	prop->SetValue(node->getCameraData().nearClip);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"CAM TARGETPOS");
	str.Format(_T("(%.2f %.2f %.2f)"),
		node->getCameraData().targetPos.x,
		node->getCameraData().targetPos.y,
		node->getCameraData().targetPos.z);
	prop->SetValue(str);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"CAM UPVECTOR");
	str.Format(_T("(%.2f %.2f %.2f)"),
		node->getCameraData().upVector.x,
		node->getCameraData().upVector.y,
		node->getCameraData().upVector.z);
	prop->SetValue(str);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"CAM LOOKATVECTOR");
	str.Format(_T("(%.2f %.2f %.2f)"),
		node->getCameraData().lookAtVector.x,
		node->getCameraData().lookAtVector.y,
		node->getCameraData().lookAtVector.z);
	prop->SetValue(str);

}

void CPropertiesWnd::updateNodeProp( ArnAnim* node )
{
	CMFCPropertyGridProperty* prop;
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"ANIM KEYCOUNT");
	prop->SetValue((_variant_t)node->getKeyCount());
}

void CPropertiesWnd::updateNodeProp( ArnSkeleton* node )
{
	CMFCPropertyGridProperty* prop;
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"SKEL MAXWEIGHTSPERVERT");
	prop->SetValue((_variant_t)node->getSkeletonData().maxWeightsPerVertex);
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"SKEL BONECOUNT");
	prop->SetValue((_variant_t)node->getSkeletonData().bonesCount);
}

void CPropertiesWnd::updateNodeProp( ArnBone* node )
{
	const D3DXMATRIX& offsetMat = node->getBoneData().offsetMatrix;
	D3DXVECTOR3 vecScaling, vecTranslation;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&vecScaling, &quat, &vecTranslation, &offsetMat);

	D3DXVECTOR3 euler = ArnMath::QuatToEuler(&quat);
	euler = ArnMath::Vec3RadToDeg(&euler);

	CString xformStr;
	CMFCPropertyGridProperty* prop;

	xformStr.Format(_T("(%.2f %.2f %.2f)"), euler.x, euler.y, euler.z);
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"BONE ROTATION");
	prop->SetValue(xformStr);

	xformStr.Format(_T("(%.2f %.2f %.2f)"), vecScaling.x, vecScaling.y, vecScaling.z);
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"BONE SCALING");
	prop->SetValue(xformStr);

	xformStr.Format(_T("(%.2f %.2f %.2f)"), vecTranslation.x, vecTranslation.y, vecTranslation.z);
	prop = m_wndPropList.FindItemByData((DWORD_PTR)"BONE TRANSLATION");
	prop->SetValue(xformStr);

	prop = m_wndPropList.FindItemByData((DWORD_PTR)"BONE INFVERTCOUNT");
	prop->SetValue((_variant_t)node->getBoneData().infVertexCount);
}

void CPropertiesWnd::updateNodeProp( ArnHierarchy* node )
{

}
void CPropertiesWnd::hideAllPropGroup()
{
	//nodeBaseGroup->Show(FALSE); /* NodeBase group is always shown */
	m_cameraGroup->Show(FALSE);
	m_animGroup->Show(FALSE);
	m_meshGroup->Show(FALSE);
	m_hierarchyGroup->Show(FALSE);
	m_skelGroup->Show(FALSE);
	m_boneGroup->Show(FALSE);
}