#include "stdafx.h"
#include "ModelExporter2.h"
#include "ErrorProcedure.h"
#include "NullRestoreObj.h"
#include "ArNodeFactory.h"
#include "ArMesh.h"
#include "ArCamera.h"
#include "ArLight.h"
#include "MeshExporter.h"

extern HINSTANCE g_hInstance; // declared at Main.cpp

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_SINGLETON(ModelExporter2)

ModelExporter2::ModelExporter2(void)
{
}

ModelExporter2::~ModelExporter2(void)
{
	NodeMap::iterator it = m_nodes.begin();
	for (; it != m_nodes.end(); ++it)
	{
		SAFE_DELETE(it->second);
	}
	m_nodes.clear();
}


const TCHAR* ModelExporter2::Ext(int n)
{
	return _T("ARN"); // use the same extension
}

const TCHAR* ModelExporter2::LongDesc()
{
	return _T("Aran Model Exporter 2");
}

const TCHAR* ModelExporter2::ShortDesc() 
{
	return _T("Aran 2");
}

const TCHAR* ModelExporter2::AuthorName()
{
	return _T("Geoyeob Kim");
}

const TCHAR* ModelExporter2::CopyrightMessage() 
{
	return _T("");
}

const TCHAR* ModelExporter2::OtherMessage1() 
{
	return _T("");
}

const TCHAR* ModelExporter2::OtherMessage2() 
{
	return _T("");
}

unsigned int ModelExporter2::Version()
{
	return 0 * 100 + 0 * 10 + 2;
}
int ModelExporter2::ExtCount()
{
	return 1;
}
void ModelExporter2::ShowAbout(HWND hWnd)
{
}
BOOL ModelExporter2::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}


int	ModelExporter2::DoExport(const TCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options)
{
	DebugPrint("   DoExport() Function Start!\n");
	srand((unsigned int)time(NULL));

#ifdef MODEL_EXPORTER_FOR_MAX_9
	if (m_bFirstTimeInit == FALSE)
	{
		::InitCustomControls(g_hInstance);
		INITCOMMONCONTROLSEX iccx;
		iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		iccx.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
		bool initControls = (::InitCommonControlsEx(&iccx) != 0);
		assert(initControls != false);
		m_bFirstTimeInit = TRUE;
	}
#endif

	time_t currentTime = 0;
	time(&currentTime);
	tm tempTM;
	localtime_s(&tempTM, &currentTime);
	strftime(m_exportTime, sizeof(m_exportTime) / sizeof(m_exportTime[0]), "%Y-%m-%d %H:%M:%S", &tempTM);

	m_coreInterface = GetCOREInterface();
	m_exportName = name;

	ErrorProcedure ep;
	SetErrorCallBack(&ep);

	float iGameVersion = GetIGameVersion();
	DebugPrint(_T("3ds Max compatible version %.2f\n"), GetSupported3DSVersion());

	m_gameScene = GetIGameInterface();
	IGameConversionManager* cm = GetConversionManager();

	// D3D Coordinates conversion
	cm->SetCoordSystem(IGameConversionManager::IGAME_D3D);

	m_coreFrame = m_coreInterface->GetTime();

	// TODO: Selected object export only feature is not implemented yet
	m_gameScene->InitialiseIGame(false);


	try
	{
		theHold.Begin();

		// Do the exporting stuff here...
		exportWorld();

		// implement undo, redo feature; not needed actually
		/*theHold.Put(new NullRestoreObj()); theHold.Accept(_T("Export Settings"));*/
	}
	catch(char const* error)
	{
		theHold.Cancel();
		::MessageBox(0, error, _T("Error while exporting"), MB_OK);
		m_bExportSuccessful = false;
	}

	m_gameScene->ReleaseIGame();
	m_coreInterface->ProgressEnd();

	return 1;
}

void ModelExporter2::exportWorld()
{
	int i;
	int topLevelNodeCount = m_gameScene->GetTopLevelNodeCount();
	for (i = 0; i < topLevelNodeCount; i++)
	{
		IGameNode* node = m_gameScene->GetTopLevelNode(i);
		exportNode(node);
	}
}

void ModelExporter2::exportNode(IGameNode* node)
{
	ArNode* arNode = NULL;
	const char* nodeName = node->GetName();
	IGameObject* gameObject = node->GetIGameObject();
	switch (gameObject->GetIGameType())
	{
		case IGameObject::IGAME_MESH:
			arNode = ArNodeFactory::create(ArNode::eArMesh, nodeName);
			exportNodeSub(node, dynamic_cast<IGameMesh*>(gameObject), dynamic_cast<ArMesh*>(arNode));
			break;
		case IGameObject::IGAME_CAMERA:
			arNode = ArNodeFactory::create(ArNode::eArCamera, nodeName);
			break;
		case IGameObject::IGAME_LIGHT:
			arNode = ArNodeFactory::create(ArNode::eArLight, nodeName);
			break;
		default:
			return; // skip this node!!!
			break;
	}
	std::string nodeStr(arNode->getName());
	m_nodes[nodeStr] = arNode;


	// delete sub-nodes
	int childCount = node->GetChildCount();
	int i;
	for (i = 0; i < childCount; i++)
	{
		exportNode(node->GetNodeChild(i));
	}
}

void ModelExporter2::exportNodeSub(IGameNode* node, IGameMesh* mesh, ArMesh* arMesh)
{
	MeshExporter* meshExporter = new MeshExporter(node, mesh, arMesh);
	meshExporter->make();
	SAFE_DELETE(meshExporter);
}

void ModelExporter2::exportNodeSub( IGameCamera* camera, ArCamera* arCamera )
{

}

void ModelExporter2::exportNodeSub( IGameLight* light, ArLight* arLight )
{

}

ArNode* const ModelExporter2::getNodeByName( std::string name ) const
{
	NodeMap::const_iterator it = m_nodes.find(name);
	if (it != m_nodes.end())
		return it->second;
	else
		return NULL;
}