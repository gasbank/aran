
#include "NodeViewerPCH.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "NodeViewer.h"
#include "MaterialListProperty.h"
//////////////////////////////////////////////////////////////////////////
#include "ArnNode.h"
#include "ArnXformable.h"
#include "ArnContainer.h"
#include "ArnMesh.h"
#include "ArnSkeleton.h"
#include "ArnHierarchy.h"
#include "ArnLight.h"
#include "ArnCamera.h"
#include "ArnAnim.h"
#include "ArnBone.h"
#include "ArnMaterial.h"
#include "ArnLight.h"
#include "ArnIpo.h"
#include "ArnMath.h"
#include "Animation.h"

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

	CMFCPropertyGridProperty* pPropNDT = new CMFCPropertyGridProperty(_T("NODE_DATA_TYPE"), (_variant_t) _T("NDT_RT_CONTAINER"), _T("ARN Node data type enum"), PROP_BASE_NDT);
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
	m_nodeBaseGroup->AddSubItem(pPropNDT);

	m_nodeBaseGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Name"), (_variant_t) _T("Node Name"), _T("Node unique name"), PROP_BASE_NAME));
	m_nodeBaseGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Parent"), (_variant_t) _T("Parent Name"), _T("Current node's parent name"), PROP_BASE_PARENT));
	
	CMFCPropertyGridProperty* pProp = 0;
	
	m_wndPropList.AddProperty(m_nodeBaseGroup);
	
	//////////////////////////////////////////////////////////////////////
	
	m_movableGroup = new CMFCPropertyGridProperty(_T("Movable"));
	
	CMFCPropertyGridProperty* localXformProp = new CMFCPropertyGridProperty(_T("Local Transform"));
	m_movableGroup->AddSubItem(localXformProp);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Deg)"), (_variant_t) _T("(0, 0, 0)"), _T("Local rotation in Euler form (Unit: Degrees)"), PROP_MOV_LX_ROT);
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Quat)"), (_variant_t) _T("(0, 0, 0)"), _T("Local rotation in Euler form (Unit: Degrees)"), PROP_MOV_LX_QUAT);
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Scaling"), (_variant_t) _T("(0, 0, 0)"), _T("Local scaling"), PROP_MOV_LX_SCALING);
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Translation"), (_variant_t) _T("(0, 0, 0)"), _T("Local translation"), PROP_MOV_LX_TRANS);
	localXformProp->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("IPO"), (_variant_t) _T("IPO linked to the node"), _T("IPO linked to the node"), PROP_MOV_IPO);
	m_movableGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_movableGroup);

	//////////////////////////////////////////////////////////////////////
	m_cameraGroup = new CMFCPropertyGridProperty(_T("Camera"));

	pProp = new CMFCPropertyGridProperty( _T("Target Position"), (_variant_t) _T("(0, 0, 0)"), _T("Camera target position"), PROP_CAM_TARGETPOS);
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Up Vector"), (_variant_t) _T("(0, 0, 0)"), _T("Camera up vector"), PROP_CAM_UPVEC);
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("LookAt Vector"), (_variant_t) _T("(0, 0, 0)"), _T("Camera look at vector"), PROP_CAM_LOOKATVEC);
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Far Clip"), (_variant_t) 0.0f, _T("Camera far clip"), PROP_CAM_FARCLIP);
	m_cameraGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Near Clip"), (_variant_t) 0.0f, _T("Camera near clip"), PROP_CAM_NEARCLIP);
	m_cameraGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_cameraGroup);
	m_cameraGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_animGroup = new CMFCPropertyGridProperty(_T("Animation"));
	
	pProp = new CMFCPropertyGridProperty( _T("Key Count"), (_variant_t)(unsigned int) 0, _T("Animation RST key-frame count"), PROP_ANIM_KEYCOUNT);
	m_animGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_animGroup);
	m_animGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_meshGroup = new CMFCPropertyGridProperty(_T("Mesh"));

	pProp = new CMFCPropertyGridProperty( _T("Vertex Count"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), PROP_MESH_VERTCOUNT);
	m_meshGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Face Count"), (_variant_t)(unsigned int) 0, _T("Mesh face count"), PROP_MESH_FACECOUNT);
	m_meshGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Material Count"), (_variant_t)(unsigned int) 0, _T("Mesh material count"), PROP_MESH_MATERIALCOUNT);
	m_meshGroup->AddSubItem(pProp);

	pProp = new CMaterialListProperty( _T("Material Name List"), (_variant_t) _T("MatNames"), _T("Material name list"), PROP_MESH_MATERIALNAMELIST);
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

	pProp = new CMFCPropertyGridProperty( _T("Max Weights Per Vertex"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), PROP_SKEL_MAXWEIGHTSPERVERT);
	m_skelGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Bone Count"), (_variant_t)(unsigned int) 0, _T("Mesh vertex count"), PROP_SKEL_BONECOUNT);
	m_skelGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_skelGroup);
	m_skelGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_boneGroup = new CMFCPropertyGridProperty(_T("Bone"));

	CMFCPropertyGridProperty* boneOffsetMatrix = new CMFCPropertyGridProperty(_T("Offset Matrix"));
	m_boneGroup->AddSubItem(boneOffsetMatrix);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Deg)"), (_variant_t) _T("(0, 0, 0)"), _T("Rotation in Euler form (Unit: Degrees)"), PROP_BONE_OFF_ROT);
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Rotation (Quat)"), (_variant_t) _T("(0, 0, 0, 0)"), _T("Rotation in Euler form (Unit: Degrees)"), PROP_BONE_OFF_QUAT);
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Scaling"), (_variant_t) _T("(0, 0, 0)"), _T("Scaling"), PROP_BONE_OFF_SCALING);
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Translation"), (_variant_t) _T("(0, 0, 0)"), _T("Translation"), PROP_BONE_OFF_TRANS);
	boneOffsetMatrix->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Inf Vertex Count"), (_variant_t)(unsigned int) 0, _T("The influenced vertex count by this bone"), PROP_BONE_INFVERTCOUNT);
	m_boneGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_boneGroup);
	m_boneGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////
	CMFCPropertyGridColorProperty* pColorProp;
	m_materialGroup = new CMFCPropertyGridProperty(_T("Material"));
	
	pProp = new CMFCPropertyGridProperty( _T("Count"), (_variant_t)(unsigned int) 0, _T("Material count embedded in this node"), PROP_MAT_COUNT);
	m_materialGroup->AddSubItem(pProp);

	CMFCPropertyGridProperty* d3dMaterial9 = new CMFCPropertyGridProperty(_T("D3DMATERIAL9"));
	m_materialGroup->AddSubItem(d3dMaterial9);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Diffuse"), RGB(0, 0, 0), NULL, _T("Diffuse color RGBA"), PROP_MAT_DIFFUSE);
	d3dMaterial9->AddSubItem(pColorProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Ambient"), RGB(0, 0, 0), NULL, _T("Ambient color RGB"), PROP_MAT_AMBIENT);
	d3dMaterial9->AddSubItem(pColorProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Specular"), RGB(0, 0, 0), NULL, _T("Specular 'shininess'"), PROP_MAT_SPECULAR);
	d3dMaterial9->AddSubItem(pColorProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Emissive"), RGB(0, 0, 0), NULL, _T("Emissive color RGB"), PROP_MAT_EMISSIVE);
	d3dMaterial9->AddSubItem(pColorProp);
	pProp = new CMFCPropertyGridProperty( _T("Power"), (_variant_t)0.0f, _T("Sharpness if specular highlight"), PROP_MAT_POWER);
	d3dMaterial9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Texture Images"), (_variant_t) _T("TexImgs"), _T("Texture images"), PROP_MAT_TEXIMGS);
	m_materialGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_materialGroup);
	m_materialGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_lightGroup = new CMFCPropertyGridProperty(_T("Light"));

	CMFCPropertyGridProperty* d3dLight9 = new CMFCPropertyGridProperty(_T("D3DLIGHT9"));
	m_lightGroup->AddSubItem(d3dLight9);

	pProp = new CMFCPropertyGridProperty( _T("Type"), _T("Point"), _T("Light type"), PROP_LIGHT_TYPE);
	pProp->AddOption(_T("Point"));
	pProp->AddOption(_T("Spot"));
	pProp->AddOption(_T("Directional"));
	pProp->AllowEdit(FALSE);
	d3dLight9->AddSubItem(pProp);

	pColorProp = new CMFCPropertyGridColorProperty(_T("Diffuse"), RGB(0, 0, 0), NULL, _T("Diffuse color of light"), PROP_LIGHT_DIFFUSE);
	d3dLight9->AddSubItem(pColorProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Specular"), RGB(0, 0, 0), NULL, _T("Specular color of light"), PROP_LIGHT_SPECULAR);
	d3dLight9->AddSubItem(pColorProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("Ambient"), RGB(0, 0, 0), NULL, _T("Ambient color of light"), PROP_LIGHT_AMBIENT);
	d3dLight9->AddSubItem(pColorProp);
	
	pProp = new CMFCPropertyGridProperty( _T("Position"), _T("(0, 0, 0)"), _T("Position in world(local) space"), PROP_LIGHT_POS);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Direction"), _T("(0, 0, 0)"), _T("Direction in world(local) space"), PROP_LIGHT_DIR);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Range"), (_variant_t)0.0f, _T("Cutoff range"), PROP_LIGHT_RANGE);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Falloff"), (_variant_t)0.0f, _T("Falloff"), PROP_LIGHT_FALLOFF);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Attenuation0"), (_variant_t)0.0f, _T("Constant attenuation"), PROP_LIGHT_ATT0);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Attenuation1"), (_variant_t)0.0f, _T("Linear attenuation"), PROP_LIGHT_ATT1);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Attenuation2"), (_variant_t)0.0f, _T("Quadratic attenuation"), PROP_LIGHT_ATT2);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Theta"), (_variant_t)0.0f, _T("Inner angle of spotlight cone"), PROP_LIGHT_THETA);
	d3dLight9->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Phi"), (_variant_t)0.0f, _T("Outer angle of spotlight cone"), PROP_LIGHT_PHI);
	d3dLight9->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_lightGroup);
	m_lightGroup->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////

	m_ipoGroup = new CMFCPropertyGridProperty(_T("IPO"));

	pProp = new CMFCPropertyGridProperty(_T("IPO Count"), (_variant_t) (unsigned int)0, _T("IPOs embedded in this node"), PROP_IPO_COUNT);
	m_ipoGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Curve Count"), (_variant_t) (unsigned int)0, _T("Curves included in the IPO (valid only if IPO count is 1)"), PROP_IPO_CURVECOUNT);
	m_ipoGroup->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Curve Names"), (_variant_t) _T("Names..."), _T("Curve names included in the IPO (valid only if IPO count is 1)"), PROP_IPO_CURVENAMES);
	m_ipoGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(m_ipoGroup);
	m_ipoGroup->Show(FALSE);
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

//////////////////////////////////////////////////////////////////////////

void CPropertiesWnd::updateNodeProp( ArnNode* node )
{
	CString ndtVal;
	hideAllPropGroup();
	switch (node->getType())
	{
	case NDT_RT_MESH:
		ndtVal = _T("NDT_RT_MESH");
		m_movableGroup->Show(TRUE);
		m_meshGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnXformable*>(node));
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
		m_movableGroup->Show(TRUE);
		m_cameraGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnXformable*>(node));
		updateNodeProp(static_cast<ArnCamera*>(node));
		break;
	case NDT_RT_HIERARCHY:
		ndtVal = _T("NDT_RT_HIERARCHY");
		m_hierarchyGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnHierarchy*>(node));
		break;
	case NDT_RT_LIGHT:
		ndtVal = _T("NDT_RT_LIGHT");
		m_movableGroup->Show(TRUE);
		m_lightGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnXformable*>(node));
		updateNodeProp(static_cast<ArnLight*>(node));
		break;
	case NDT_RT_MATERIAL:
		ndtVal = _T("NDT_RT_MATERIAL");
		m_materialGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnMaterial*>(node));
		break;
	case NDT_RT_IPO:
		ndtVal = _T("NDT_RT_IPO");
		m_ipoGroup->Show(TRUE);
		updateNodeProp(static_cast<ArnIpo*>(node));
		// Bezier interpolation testing
		writePrecomputedCurvesToFile(static_cast<ArnIpo*>(node));
		break;
	default:
		ndtVal = _T("NDT_RT_CONTAINER");
		break;
	}
	
	propEnumSetValue(PROP_BASE_NDT,			ndtVal);
	propEnumSetValue(PROP_BASE_NAME,		node->getName());
	propEnumSetValue(PROP_BASE_PARENT,		node->getParentName());
}

void CPropertiesWnd::updateNodeProp( ArnMesh* node )
{
	const MeshData& md = node->getMeshData();
	CString str, substr;
	propEnumSetValue(PROP_MESH_VERTCOUNT,		md.vertexCount);
	propEnumSetValue(PROP_MESH_FACECOUNT,		md.faceCount);
	propEnumSetValue(PROP_MESH_MATERIALCOUNT,	md.materialCount);
	unsigned int i;
	for (i = 0; i < md.materialCount; ++i)
	{
		substr.Format(_T("%s"), CString(md.matNameList[i].c_str()));
		str += (i!=0)?_T(","):_T("");
		str += substr;
	}
	propEnumSetValue(PROP_MESH_MATERIALNAMELIST, str);
}

void CPropertiesWnd::updateNodeProp( ArnCamera* node )
{
	CString str;
	const ARN_NDD_CAMERA_CHUNK& cameraData = node->getCameraData();
	propEnumSetValue(PROP_CAM_FARCLIP,		cameraData.farClip);
	propEnumSetValue(PROP_CAM_NEARCLIP,		cameraData.nearClip);
	propEnumSetValue(PROP_CAM_TARGETPOS,	cameraData.targetPos);
	propEnumSetValue(PROP_CAM_UPVEC,		cameraData.upVector);
	propEnumSetValue(PROP_CAM_LOOKATVEC,	cameraData.lookAtVector);
}

void CPropertiesWnd::updateNodeProp( ArnAnim* node )
{
	propEnumSetValue(PROP_ANIM_KEYCOUNT, node->getKeyCount());
}

void CPropertiesWnd::updateNodeProp( ArnSkeleton* node )
{
	const SkeletonData& sd = node->getSkeletonData();
	propEnumSetValue(PROP_SKEL_MAXWEIGHTSPERVERT,	sd.maxWeightsPerVertex);
	propEnumSetValue(PROP_SKEL_BONECOUNT,			sd.bonesCount);
}

void CPropertiesWnd::updateNodeProp( ArnBone* node )
{
	const D3DXMATRIX& offsetMat = node->getBoneData().offsetMatrix;
	D3DXVECTOR3 vecScaling, vecTranslation;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&vecScaling, &quat, &vecTranslation, &offsetMat);

	D3DXVECTOR3 euler = ArnMath::QuatToEuler(&quat);
	euler = ArnMath::Vec3RadToDeg(&euler);

	propEnumSetValue(PROP_BONE_OFF_ROT,			euler);
	propEnumSetValue(PROP_BONE_OFF_QUAT,		quat);
	propEnumSetValue(PROP_BONE_OFF_SCALING,		vecScaling);
	propEnumSetValue(PROP_BONE_OFF_TRANS,		vecTranslation);
	propEnumSetValue(PROP_BONE_INFVERTCOUNT,	node->getBoneData().infVertexCount);
}

void CPropertiesWnd::updateNodeProp( ArnHierarchy* node )
{
	// TODO: not implemented yet
}

void CPropertiesWnd::updateNodeProp( ArnMaterial* node )
{
	CString str, substr;
	const D3DMATERIAL9& mat = node->getD3DMaterialData();
	propEnumSetValue(PROP_MAT_COUNT,	node->getMaterialCount());
	propEnumSetValue(PROP_MAT_DIFFUSE,	mat.Diffuse);
	propEnumSetValue(PROP_MAT_AMBIENT,	mat.Ambient);
	propEnumSetValue(PROP_MAT_SPECULAR, mat.Specular);
	propEnumSetValue(PROP_MAT_EMISSIVE, mat.Emissive);
	propEnumSetValue(PROP_MAT_POWER,	mat.Power);
	unsigned int i;
	for (i = 0; i < node->getTexImgCount(); ++i)
	{
		substr = node->getTexImgName(i).c_str();
		str += (i!=0)?_T(","):_T("");
		str += substr;
	}
	propEnumSetValue(PROP_MAT_TEXIMGS, str);
}

void CPropertiesWnd::updateNodeProp( ArnLight* node )
{
	CString str;
	const D3DLIGHT9& light = node->getD3DLightData();
	
	switch (node->getD3DLightData().Type)
	{
	case D3DLIGHT_POINT:		str = "Point";			break;
	case D3DLIGHT_SPOT:			str = "Spot";			break;
	case D3DLIGHT_DIRECTIONAL:	str = "Directional";	break;
	}

	propEnumSetValue(PROP_LIGHT_TYPE,		str);
	propEnumSetValue(PROP_LIGHT_DIFFUSE,	light.Diffuse);
	propEnumSetValue(PROP_LIGHT_SPECULAR,	light.Specular);
	propEnumSetValue(PROP_LIGHT_AMBIENT,	light.Ambient);
	propEnumSetValue(PROP_LIGHT_POS,		light.Position);
	propEnumSetValue(PROP_LIGHT_DIR,		light.Direction);
	propEnumSetValue(PROP_LIGHT_RANGE,		light.Range);
	propEnumSetValue(PROP_LIGHT_FALLOFF,	light.Falloff);
	propEnumSetValue(PROP_LIGHT_ATT0,		light.Attenuation0);
	propEnumSetValue(PROP_LIGHT_ATT1,		light.Attenuation1);
	propEnumSetValue(PROP_LIGHT_ATT2,		light.Attenuation2);
}

void CPropertiesWnd::updateNodeProp( ArnIpo* node )
{
	CString str, substr;
	unsigned int curveCount = node->getCurveCount();

	propEnumSetValue(PROP_IPO_COUNT, node->getIpoCount());
	propEnumSetValue(PROP_IPO_CURVECOUNT, curveCount);

	unsigned int i;
	for (i = 0; i < curveCount; ++i)
	{
		const CurveData& cd = node->getCurveData(i);
		substr.Format(_T("%s(%d)"), CString(cd.nameStr.c_str()), cd.points.size());
		str += (i!=0)?_T(","):_T("");
		str += substr;
	}
	propEnumSetValue(PROP_IPO_CURVENAMES, str);
}

void CPropertiesWnd::updateNodeProp( ArnXformable* node )
{
	const D3DXMATRIX& localXform = node->getFinalLocalXform();
	D3DXVECTOR3 vecScaling, vecTranslation;
	D3DXQUATERNION quat;
	D3DXMatrixDecompose(&vecScaling, &quat, &vecTranslation, &localXform);

	D3DXVECTOR3 euler = ArnMath::QuatToEuler(&quat);
	euler = ArnMath::Vec3RadToDeg(&euler);

	propEnumSetValue(PROP_MOV_LX_ROT,		euler);
	propEnumSetValue(PROP_MOV_LX_QUAT,		quat);
	propEnumSetValue(PROP_MOV_LX_SCALING,	vecScaling);
	propEnumSetValue(PROP_MOV_LX_TRANS,		vecTranslation);
	propEnumSetValue(PROP_MOV_IPO,			node->getIpoName());
}
void CPropertiesWnd::hideAllPropGroup()
{
	//nodeBaseGroup->Show(FALSE); /* NodeBase group is always shown */
	m_movableGroup->Show(FALSE);
	m_cameraGroup->Show(FALSE);
	m_animGroup->Show(FALSE);
	m_meshGroup->Show(FALSE);
	m_hierarchyGroup->Show(FALSE);
	m_skelGroup->Show(FALSE);
	m_boneGroup->Show(FALSE);
	m_materialGroup->Show(FALSE);
	m_lightGroup->Show(FALSE);
	m_ipoGroup->Show(FALSE);
}

// Prop SetValue() helper methods

void CPropertiesWnd::propEnumSetValue(PROP_ENUM pe, const POINT3FLOAT& p3f)
{	
	CString str;
	str.Format(_T("(%.2f %.2f %.2f)"), p3f.x, p3f.y, p3f.z);
	m_wndPropList.FindItemByData(pe)->SetValue(str);
}
void CPropertiesWnd::propEnumSetValue(PROP_ENUM pe, float f)
{	
	m_wndPropList.FindItemByData(pe)->SetValue((_variant_t)f);
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, unsigned int ui )
{
	m_wndPropList.FindItemByData(pe)->SetValue((_variant_t)ui);
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, CString& cstr )
{
	m_wndPropList.FindItemByData(pe)->SetValue(cstr);
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, const D3DVECTOR& d3dVec )
{
	CString str;
	str.Format(_T("(%.2f %.2f %.2f)"), d3dVec.x, d3dVec.y, d3dVec.z);
	m_wndPropList.FindItemByData(pe)->SetValue(str);
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, const D3DCOLORVALUE& d3dColVal )
{
	CMFCPropertyGridColorProperty* colorProp = (CMFCPropertyGridColorProperty*)m_wndPropList.FindItemByData(pe);
	colorProp->SetColor((COLORREF)ArnMath::Float4ColorToDword(&d3dColVal));
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, const D3DXQUATERNION& quat )
{
	CString str;
	str.Format(_T("(%.2f %.2f %.2f; %.2f)"), quat.x, quat.y, quat.z, quat.w);
	m_wndPropList.FindItemByData(pe)->SetValue(str);
}

void CPropertiesWnd::propEnumSetValue( PROP_ENUM pe, const STRING& arnStr )
{
	m_wndPropList.FindItemByData(pe)->SetValue(CString(arnStr.c_str()));
}
void CPropertiesWnd::writePrecomputedCurvesToFile( ArnIpo* node )
{
	const float timeStep = 0.1f;
	unsigned int curveCount = node->getCurveCount();
	unsigned int i, j;

	CString fileName;
	fileName = _T("c:\\");
	fileName += node->getName();
	fileName += _T(".txt");
	std::ofstream file(fileName);

	for (i = 0; i < curveCount; ++i)
	{
		const CurveData& cd = node->getCurveData(i);
		file << cd.nameStr << std::endl;
		for (j = 0; j < 1000; j++)
			file << Animation::EvalCurveInterp(&cd, timeStep * j) << std::endl;
	}

	file.close();
}