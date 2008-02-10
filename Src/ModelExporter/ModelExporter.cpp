// ModelExporter.cpp
// 2007 Geoyeob Kim
//


#include "stdafx.h"
#include "ModelExporter.h"
#include "ErrorProcedure.h"
#include "NullRestoreObj.h"
#include "resource.h"

#include "../VideoLib/Macros.h"

// Define this if want additional dump information
#define EXPORT_DEBUG

#define COMPARE_EPSILON			1e-4

//////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////
extern HINSTANCE g_hInstance;												// declared at Main.cpp

// Export Settings dialog
EXPORT_VERSION g_exportVersion = EV_UNDEFINED;								// ARN version
NODE_DATA_TYPE g_ndtMesh = NDT_UNKNOWN;										// Mesh export type
NODE_DATA_TYPE g_ndtAnim = NDT_UNKNOWN;										// Anim export type
DWORD g_animStartFrame = 0;													// Anim start frame
DWORD g_animEndFrame = 0;													// Anim end frame
DWORD g_animTicks = 0;

HWND g_hProgressBarDialog;
HWND g_hProgressBar; // indicates export progress

//////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
//////////////////////////////////////////////////////////////////////////
static float dot(Quat const & q, Quat const & p);							// Quaternion dot product
static void GlobalMirror(GMatrix &m);										// Z-axis mirroring (Not used)
// TODO: incline function considered
static bool almostEqual(Point3 const & a, Point3 const & b);
static bool almostEqual(Quat const & a, Quat const & b);
INT_PTR CALLBACK SettingsDialogProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressDialogProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


ModelExporter::ModelExporter(void)
:isFirstTimeInit(FALSE), isExportSuccessful(FALSE), nodeCount(0)
{
	// Axis transform matrix init
	this->maxToDx.SetRow(0, Point4(1, 0, 0, 0));
	this->maxToDx.SetRow(1, Point4(0, 0, -1, 0));
	this->maxToDx.SetRow(2, Point4(0, 1, 0, 0));
	this->maxToDx.SetRow(3, Point4(0, 0, 0, 1));
}

ModelExporter::~ModelExporter(void)
{
}

#pragma region /* 3ds Max 9 Interfaces */
const TCHAR* ModelExporter::Ext(int n)
{
	return _T("ARN");
}

const TCHAR* ModelExporter::LongDesc()
{
	return _T("Aran Model Exporter");
}

const TCHAR* ModelExporter::ShortDesc() 
{
	return _T("AAA Aran");
}

const TCHAR* ModelExporter::AuthorName()
{
	return _T("Geoyeob Kim");
}

const TCHAR* ModelExporter::CopyrightMessage() 
{
	return _T("");
}

const TCHAR* ModelExporter::OtherMessage1() 
{
	return _T("");
}

const TCHAR* ModelExporter::OtherMessage2() 
{
	return _T("");
}

unsigned int ModelExporter::Version()
{
	return 0 * 100 + 0 * 10 + 1;
}
int ModelExporter::ExtCount()
{
	return 1;
}
void ModelExporter::ShowAbout(HWND hWnd)
{
	//::DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
}
BOOL ModelExporter::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}


#pragma endregion



DWORD WINAPI ProgressDialogThread(void* arg)
{
	DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_PROGRESS_DIALOG), 0, ProgressDialogProc);
	return 0;
};

//////////////////////////////////////////////////////////////////////////
// Exporting Entry Point
//////////////////////////////////////////////////////////////////////////
int	ModelExporter::DoExport(const TCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options)
{
	DebugPrint(_T("===================================================\n"));
	DebugPrint(_T("   DoExport() Function Start!\n"));
	DebugPrint(_T("===================================================\n"));

	srand((unsigned int)time(NULL));

#ifdef MODEL_EXPORTER_FOR_MAX_9
	if (this->isFirstTimeInit == FALSE)
	{
		::InitCustomControls(g_hInstance);
		INITCOMMONCONTROLSEX iccx;
		iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		iccx.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
		bool initControls = (::InitCommonControlsEx(&iccx) != 0);
		assert(initControls != false);
		this->isFirstTimeInit = TRUE;
	}
#endif

	time_t currentTime = 0;
	time(&currentTime);
	tm tempTM;
	localtime_s(&tempTM, &currentTime);
	strftime(this->exportTime, sizeof(this->exportTime) / sizeof(this->exportTime[0]), "%Y-%m-%d %H:%M:%S", &tempTM);

	this->coreInterface = GetCOREInterface();
	this->exportName = name;

	ErrorProcedure ep;
	SetErrorCallBack(&ep);

	float iGameVersion = GetIGameVersion();
	DebugPrint(_T("3ds Max compatible version %.2f\n"), GetSupported3DSVersion());

	this->game = GetIGameInterface();
	IGameConversionManager* cm = GetConversionManager();

	// D3D Coordinates conversion
	cm->SetCoordSystem(IGameConversionManager::IGAME_D3D);
	//cm->SetCoordSystem(IGameConversionManager::IGAME_MAX);



	this->coreFrame = this->coreInterface->GetTime();

	//
	// Open Export Settings window
	//
	if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SETTINGS_DIALOG), 0, SettingsDialogProc, (LPARAM)this))
	{
		return 1; // cancel button clicked
	}

	// Export settings dialog closed by clicking Okay button

	// Progress bar dialog ( sleek look :) )
	DWORD tid;
	CreateThread (
		0, // Security attributes
		0, // Stack size
		ProgressDialogThread,
		NULL,
		0,
		&tid );

	// TODO: Selected object export only feature is not implemented yet
	this->game->InitialiseIGame(false);

	try
	{
		theHold.Begin();

		
		////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////

		this->StartExporting(); // open output file and do the main exporting job

		this->FinalizeExporting(); // do the final exporting job and close the file

		////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////

		theHold.Put(new NullRestoreObj());
		theHold.Accept(_T("Export Settings"));
	}
	catch(char const* error)
	{
		theHold.Cancel();
		::MessageBox(0, error, _T("Error while exporting"), MB_OK);
		this->isExportSuccessful = false;
	}

	this->game->ReleaseIGame();
	this->coreInterface->ProgressEnd();

	//::MessageBox(0, _T("Export Process"), _T("DoExport()"), MB_OK);
	SendMessage( g_hProgressBarDialog, WM_CLOSE, 0, 0 );
	return 1;
}

HRESULT ModelExporter::StartExporting()
{
	IGameScene* ig = this->game;
	int i;

	//
	// open output file
	//
	this->fout.clear();
	this->fout.open(this->exportName.c_str(), std::ios_base::binary);

	if ( g_exportVersion == EV_ARN10 )
	{
		_snprintf_s( this->fileDescriptor, 6, 6, "ARN10" );
	}
	else if ( g_exportVersion == EV_ARN20 )
	{
		_snprintf_s( this->fileDescriptor, 6, 6, "ARN20" );
	}
	else
	{
		MessageBox(NULL, _T("Unknown Export Type"), _T("Error"), MB_ICONERROR | MB_OK);
		return E_FAIL;
	}
	fileDescriptor[5] = '\0';




	this->fout.write(this->fileDescriptor, (int)strlen(this->fileDescriptor) + 1); // file descriptor (ARNxy; x means version, y means compression method)
	this->fout.write((char*)&ffffffff, sizeof(int)); // reserve space for node(NDD) Array Size


	this->rootMaterialCount = ig->GetRootMaterialCount();
	for (i = 0; i < this->rootMaterialCount; i++)
	{
		DebugPrint(_T(" - RootMaterial INDEX: %d, Name: %s\n"), i, ig->GetRootMaterial(i)->GetMaterialName());
		this->PrintChildMaterialInfo(ig->GetRootMaterial(i), 1);
	}

	// progress bar range setting
	SendMessage( g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ig->GetTotalNodeCount()) );

	int topLevelNodeCount = ig->GetTopLevelNodeCount();
	for (i = 0; i < topLevelNodeCount; i++)
	{
		IGameNode* node = ig->GetTopLevelNode(i);
		this->ExportNode(node);
	}

	return S_OK;
}

HRESULT ModelExporter::ExportNDD_BoneHierarchy()
{
	//
	// Write NDD(Bone Hierarchy)
	//
	size_t boneHierarchySize = this->boneHierarchy.size();

	int ndt = NDT_HIERARCHY;
	int chunkSize = -1;
	this->fout.write((char*)&ndt, sizeof(int));
	this->fout.write("Bone Hierarchy", (int)strlen("Bone Hierarchy") + 1);
	this->fout.write((char*)&chunkSize, sizeof(int)); // reserve for chuck size
	DWORD chuckStartPos = this->fout.tellp();

	//this->fout.write((char*)&ffffffff, sizeof(int));
	//this->fout.write("HIER", (int)strlen("HIER")); // not null-terminated; fixed length


	this->fout.write((char*)&boneHierarchySize, sizeof(size_t));
	size_t s;
	for (s = 0; s < boneHierarchySize; s++)
	{
		int nameLen = (int)strlen(this->boneHierarchy[s].boneName);
		this->fout.write(this->boneHierarchy[s].boneName, nameLen + 1);
		this->fout.write((char*)&this->boneHierarchy[s].isRootBone, sizeof(BOOL));
		this->fout.write((char*)&this->boneHierarchy[s].sibling, sizeof(size_t));
		this->fout.write((char*)&this->boneHierarchy[s].firstChild, sizeof(size_t));
	}

	//
	// calculate chunk size and write it
	//
	DWORD chuckEndPos = this->fout.tellp();
	chunkSize = chuckEndPos - chuckStartPos;
	this->fout.seekp(chuckStartPos-sizeof(int), std::ios_base::beg);
	this->fout.write((char*)&chunkSize, sizeof(int));
	this->fout.seekp(chuckEndPos, std::ios_base::beg);

	this->nodeCount++;

	return S_OK;
}

HRESULT ModelExporter::FinalizeExporting()
{
	if (g_exportVersion == EV_ARN20)
	{
		this->ExportNDD_BoneHierarchy();
	}

	// go back to the node count field and write it
	this->fout.seekp((int)strlen(fileDescriptor) + 1, std::ios_base::beg);
	this->fout.write((char*)&this->nodeCount, sizeof(int));

	// go to the end of file and attach terminal string
	this->fout.seekp(0, std::ios_base::end);
	this->fout.write("TERM", (int)strlen("TERM") + 1);

	//
	// close output file
	//
	this->fout.close();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// NDD Exporting Main Entry
//////////////////////////////////////////////////////////////////////////
int ModelExporter::ExportNode(IGameNode* node)
{
	std::string nodeName = node->GetName();
	DebugPrint(_T(" - Exporting Node: %s\n"), nodeName.c_str());
	SendMessage( g_hProgressBar, PBM_STEPIT, 0, 0 ); // progress bar increment


	IGameObject* obj = node->GetIGameObject();
	IGameSkin* igs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : NULL;

	if (obj)
	{
		IGameObject::ObjectTypes objectType = obj->GetIGameType();

		switch (objectType)
		{
		case IGameObject::IGAME_MESH:
			if ( igs != NULL && g_ndtMesh == NDT_MESH3 )
			{
				// NDT_MESH3(Skinned); Skinned mesh with NDT_MESH3 selected
				// TODO: NOT IMPLEMENTED YET
				this->ExportNDD_Mesh3(node);
			}
			else
			{
				// Otherwise:
				// NDT_MESH1, NDT_MESH2, NDT_MESH3(No skinning)

				this->ExportNDD_Mesh(node);

				// NDT_MESH2 only
				if (g_exportVersion == EV_ARN20 && g_ndtMesh == NDT_MESH2)
				{
					this->ExportNDD_SkinningData(node); // NDD (Bone; Skeleton)
				}
			}

			break;

		case IGameObject::IGAME_HELPER: // TODO: Testing...
		case IGameObject::IGAME_BONE:
			// Build this->boneHierarchy only (no fout access)
			// Will be written to the file when this->ExportNDD_BoneHierarchy() called
			this->BuildNDD_BoneHierarchy(node);
			break;

		case IGameObject::IGAME_CAMERA:
			this->ExportNDD_Camera( node );
			//this->ExportAnimation(node, obj);
			break;

		case IGameObject::IGAME_LIGHT:
			// TODO: Should be checked
			//
			//this->ExportLight(node, obj);
			//this->ExportKeyframeAnimationOfNode(node);
			break;

		case IGameObject::IGAME_SPLINE:
			break;

		default:
			break;
		}
	}

	int childCount = node->GetChildCount();
	int i;
	for (i = 0; i < childCount; i++)
	{
		DebugPrint(_T(" - Child node detected.\n"));
		this->ExportNode(node->GetNodeChild(i));
	}

	return 0;
}

HRESULT ModelExporter::ExportNDD_Mesh3( IGameNode* node )
{
	HRESULT hr = ExportMeshARN30( node );
	
	if ( FAILED( hr ) )
	{
		return hr;
	}
	else
	{
		this->nodeCount++;
		return hr;
	}
}
int ModelExporter::ExportMeshARN30( IGameNode* node )
{
	// Export Node Data Definition (Mesh Type 3; Skin Integrated)
	// Author: Geoyeob Kim
	// Date: 2007-12-29

	IGameObject* iobj = node->GetIGameObject();
	IGameMesh* igm = static_cast<IGameMesh*>( iobj );
	IGameSkin* igs = iobj->IsObjectSkinned() ? iobj->GetIGameSkin() : NULL;
	static char buffer[2048];

	// ARN30 format is defined to implement 3D skinning object in the engine
	ASSERTCHECK( igm != NULL );

	igm->SetCreateOptimizedNormalList(); // is it necessary?

	if ( !igm->InitializeData() )
	{
		_snprintf_s( buffer, 2047, 2047, "Could not read mesh data from '%s'\n", node->GetName() );
		buffer[2047] = '\0';
		DebugPrint( _T( buffer ) );
	}

	//DWORD numFacesUsed = 0;
	const DWORD numFaces = igm->GetNumberOfFaces();

	if ( numFaces <= 0 )
	{
		DebugPrint( _T( " - Node '%s' has no faces. Skip exporting this node...\n" ), node->GetName() );
		return -1;
	}

	const DWORD numVertices = numFaces * 3;
	std::vector<DWORD> indices; // vertex indices
	std::vector<int> materialIndices; // one material index per three vertex indices
	indices.resize( numVertices );

	FULL_VERTEX fv;
#ifdef EXPORT_DEBUG
	std::vector<FULL_VERTEX> fvList; // uncondensed(not indexed) vertices list [DEBUG USE ONLY!]
#endif

	std::vector<FULL_VERTEX> condensedVertices;
	std::vector<std::pair<float, DWORD>> condensedVerticesAcc; // helps to speed-up optimization of vertices
	std::vector<ARN_MTD> materialList;

	int i, j, k;

	for ( i = 0; i < (int)numFaces; i++ )
	{

		FaceEx* faceEx = igm->GetFace( i );
		IGameMaterial* igmat = igm->GetMaterialFromFace( faceEx );
		TCHAR* matName = NULL;
		TCHAR* matClassName = NULL;

		DebugPrint( _T( "Face #%d - MatID: %d" ), i, faceEx->matID );

		int matID = -1;
		if ( igmat != NULL )
		{
			DebugPrint( _T( " (has material info: %s" ), igmat->GetMaterialName() );

			matName = igmat->GetMaterialName();
			matClassName = igmat->GetMaterialClass();

			ARN_MTD cm; // "current" material
			cm.strTexFileName = _T("");

			if ( igmat->GetNumberOfTextureMaps() != 0 )
			{
				IGameTextureMap* igtm = igmat->GetIGameTextureMap( 0 );
				TCHAR* textureFileName = igtm->GetBitmapFileName();
				DebugPrint( _T( ", %s" ), textureFileName );
				cm.strTexFileName = textureFileName;
				igtm = NULL;
			}
			DebugPrint( _T( ")\n" ) );
			
			// insert to material list
			// remove duplicates, of course
			bool isFirstMaterial = true;
			for ( k = 0; k < (int)materialList.size(); k++ )
			{
				if ( materialList[k].strMatName.compare( matName ) == 0 )
				{
					isFirstMaterial = false;
					break;
				}
			}
			if ( isFirstMaterial == true )
			{
				FillARN_MTDWithIGameMaterial( cm /* reference */, igmat, matName );

				materialList.push_back( cm );
			}
		}
		else
		{
			DebugPrint( _T( "\n" ) );
		}

		// three vertices per face
		for ( j = 0; j < 3; j++ )
		{
			ZeroMemory( &fv, sizeof( fv ) );
			fv.origVert = faceEx->vert[j]; // vert id

			fv.pos = ModelExporter::TransformVector4(
				node->GetWorldTM().Inverse(),
				Point4( igm->GetVertex( faceEx->vert[j], false ), 1.0f )
				);

			fv.normal = igm->GetNormal( faceEx->norm[j], false );

			Point2 texCoord = igm->GetTexVertex( faceEx->texCoord[j] );

			ZeroMemory( &fv.color, sizeof( fv.color ) );

			fv.uvw.x = texCoord.x;
			fv.uvw.y = texCoord.y;

			indices[i * 3 + j] = this->AddCondensedVertex( condensedVertices, condensedVerticesAcc, fv );

#ifdef EXPORT_DEBUG
			fvList.push_back( fv );
#endif
		}

		bool isRightMaterialName = false;
		for ( k = 0; k < (int)materialList.size(); k++ )
		{
			if ( igmat == NULL )
			{
				break;
			}

			if ( materialList[k].strMatName.compare( igmat->GetMaterialName() ) == 0 )
			{
				isRightMaterialName = true;
				materialIndices.push_back( k );
				break;
			}
		}
		if ( isRightMaterialName != true )
		{
			int ffffffff = -1;
			materialIndices.push_back( -1 );
			DebugPrint( _T( "Material Not Found!" ) );
		}

	}

	//  Calculate mapping from old vertex index to new vertex indices
	ZeroMemory( &this->currentMeshRemap, sizeof( Remap ) );
	if ( condensedVertices.size() > 0 )
	{
		FULL_VERTEX* fvp = &condensedVertices[0];
		FULL_VERTEX* base = fvp;
		FULL_VERTEX* end = fvp + condensedVertices.size();

		int mo = -1;
		while ( fvp < end )
		{
			if ( mo < fvp->origVert )
			{
				mo = fvp->origVert;
			}
			++fvp;
		}
		this->currentMeshRemap.SetOldCount( mo + 1 );
		fvp = base;
		while ( fvp < end )
		{
			this->currentMeshRemap.MapOldToNew( (int)fvp->origVert, (int)(fvp-base) );
			++fvp;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// all data except skinning built
	// before build skinning information, prepare an intermediate dumping
	// first...
	//////////////////////////////////////////////////////////////////////////

	ARN_NDD_MESH3 nddMesh3; // finally result struct will be stored this variable

	nddMesh3.header.ndt = NDT_MESH3;
	strncpy_s(
		nddMesh3.header.uniqueName,
		(int)( strlen( node->GetName() ) + 1 ),
		node->GetName(),
		sizeof( nddMesh3.header.uniqueName )
		);
	nddMesh3.header.chunkSize = -1; // calculated afterwards

	nddMesh3.vertexSize = (int)condensedVertices.size();
	nddMesh3.indexSize = numFaces * 3;
	ASSERTCHECK( materialList.size() == 1 ); // skinned mesh assumed to have only one material
	nddMesh3.mtd = materialList[0];

	// copy FULL_VERTEX subset from condensedVertices to arnVDDS
	std::vector<ARN_VDD_S> arnVDDS;
	arnVDDS.resize( nddMesh3.vertexSize );
	for ( i = 0; i < nddMesh3.vertexSize; i++ )
	{
		FULL_VERTEX* pFV = &condensedVertices[i];
		ARN_VDD_S* pVDDS = &arnVDDS[i];

		// TODO: NOT IMPLEMENTED
		/*memcpy( &pVDDS->fv, pFV, sizeof( FULL_VERTEX) );
		pVDDS->boneIndices = 0;
		ZeroMemory( pVDDS->boneWeights, sizeof( float ) * 4 );*/
	}


	HRESULT hr = this->BuildSkinningData( node, arnVDDS /* reference */ );
	ASSERTCHECK( SUCCEEDED( hr ) );

	//////////////////////////////////////////////////////////////////////////
	// Dump this mesh to text file for debug
	//////////////////////////////////////////////////////////////////////////
	std::ofstream foutDump("c:\\exporter dump.txt");
	
	foutDump << "=========================================================================" << std::endl;
	foutDump << "NDT_MESH3 EXPORTING INTERMEDIATE DUMP"										<< std::endl;
	foutDump << "=========================================================================" << std::endl;
	foutDump << " "																			<< std::endl;
	foutDump << " [ SUMMARY ]"																<< std::endl;
	foutDump << "    - Mesh Node Name                    : " << node->GetName()				<< std::endl;
	foutDump << "    - Total Vertices Count (Not indexed): " << numFaces * 3				<< std::endl;
	foutDump << "    - Total Vertices Count (Indexed)    : " << condensedVertices.size()	<< std::endl;
	foutDump << "    - Total Faces Count                 : " << numFaces					<< std::endl;
	foutDump << " "																			<< std::endl;
#ifdef EXPORT_DEBUG
	foutDump << "=========================================================================" << std::endl;
	foutDump << " [ VERTICES (NOT INDEXED) ]"												<< std::endl;
	for ( i = 0; i < (int)numFaces; i++ )
	{
		foutDump << "Face# " << i															<< std::endl;
		for ( j = 0; j < 3; j++ )
		{
			foutDump << "    V# " << std::setw(5) << i*3 + j << "("
				<< std::setiosflags(std::ios::fixed) << std::setprecision(3)
				<< std::setw(10) << fvList[i*3 + j].pos.x << ","
				<< std::setw(10) << fvList[i*3 + j].pos.y << ","
				<< std::setw(10) << fvList[i*3 + j].pos.z << ")"
				<< " IV# " << std::setw(5) << indices[i*3 + j]
				<< std::endl;
		}
	}
#endif
	foutDump << " "																			<< std::endl;
	foutDump << "=========================================================================" << std::endl;
	foutDump << " [ VERTICES (INDEXED) ]"													<< std::endl;
	for ( i = 0; i < (int)numFaces; i++ )
	{
		foutDump << "Face# " << i															<< std::endl;
		foutDump << " IVs# " << indices[i*3 + 0] << " " << indices[i*3 + 1] << " " << indices[i*3 + 2] << std::endl;
	}
	for ( i = 0; i < (int)condensedVertices.size(); i++ )
	{
		foutDump << "  IV# " << std::setw(5) << i << "("
			<< std::setiosflags(std::ios::fixed) << std::setprecision(3)
			<< std::setw(10) << condensedVertices[i].pos.x << ","
			<< std::setw(10) << condensedVertices[i].pos.y << ","
			<< std::setw(10) << condensedVertices[i].pos.z << ")" << std::endl;
	}
	
	foutDump.close();

	

	

	//
	// - Animation Data Definition (ADD)
	//
	// is processed by following function;
	//
	//this->ExportNDD_Anim(node);

	//
	// calculate chunk size and write it
	//


	return hr;
}

int ModelExporter::FillARN_MTDWithIGameMaterial( ARN_MTD& cm, IGameMaterial* igmat, TCHAR* matName )
{
	// Texture part of 'ARN_MTD* cm' is init at another place
	// So ZeroMemory'ing of 'ARN_MTD* cm' is unsafe.
	cm.strMatName = matName;

	Point3 p3;

	if ( igmat->GetDiffuseData() != NULL )
	{
		igmat->GetDiffuseData()->GetPropertyValue( p3 );
		cm.d3dMat.Diffuse.r = p3.x;
		cm.d3dMat.Diffuse.g = p3.y;
		cm.d3dMat.Diffuse.b = p3.z;
	}
	else
	{
		ZeroMemory( &cm.d3dMat.Diffuse, sizeof( D3DCOLORVALUE ) );
	}
	cm.d3dMat.Diffuse.a = 1.0f;


	if ( igmat->GetAmbientData() != NULL)
	{
		igmat->GetAmbientData()->GetPropertyValue( p3 );
		cm.d3dMat.Ambient.r = p3.x;
		cm.d3dMat.Ambient.g = p3.y;
		cm.d3dMat.Ambient.b = p3.z;
	}
	else
	{
		ZeroMemory( &cm.d3dMat.Ambient, sizeof( D3DCOLORVALUE ) );
	}
	cm.d3dMat.Ambient.a = 1.0f;

	if ( igmat->GetSpecularData() != NULL )
	{
		igmat->GetSpecularData()->GetPropertyValue( p3 );
		cm.d3dMat.Specular.r = p3.x;
		cm.d3dMat.Specular.g = p3.y;
		cm.d3dMat.Specular.b = p3.z;
	}
	else
	{
		ZeroMemory( &cm.d3dMat.Specular, sizeof( D3DCOLORVALUE ) );
	}
	cm.d3dMat.Specular.a = 1.0f;


	if ( igmat->GetEmissiveData() != NULL )
	{
		igmat->GetEmissiveData()->GetPropertyValue( p3 );
		cm.d3dMat.Emissive.r = p3.x;
		cm.d3dMat.Emissive.g = p3.y;
		cm.d3dMat.Emissive.b = p3.z;
	}
	else
	{
		ZeroMemory( &cm.d3dMat.Emissive, sizeof( D3DCOLORVALUE ) );
	}
	cm.d3dMat.Emissive.a = 1.0f;

	return 0;
}

HRESULT ModelExporter::ExportNDD_Mesh(IGameNode* node)
{

	// Start of Node Definition Data (NDD) for ARN format
	//
	// - Node name
	// - Vertex Data Definition (VDD)
	// - Free-Material Table (FMT)
	// - Material & Texture Definition (MTD)
	//
	// are processed by following function;
	//
	if (g_exportVersion == EV_ARN10)
	{
		if (SUCCEEDED(this->ExportMeshARN10(node)))
		{
			this->nodeCount++;
		}
	}
	else if (g_exportVersion == EV_ARN20)
	{
		if (SUCCEEDED(this->ExportMeshARN20(node)))
		{
			this->nodeCount++;
		}
	}
	else
	{
		MessageBox(NULL, _T("Export Version Error! Target file corrupted!"), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}



	return S_OK;
}
int ModelExporter::BuildNDD_BoneHierarchy(IGameNode* node)
{
	if ( g_ndtMesh == NDT_MESH3 )
		return -2;
	else if ( findBoneHierarchyByNode( node ) )
		return 0; // Already exist

	BoneHierarchy bh(node->GetName(), -1, -1, false);

	if (node->GetNodeParent() == NULL)
	{
		// this bone is root
		bh.isRootBone = TRUE;
		this->currentBoneDepth = 0;
		this->currentBoneParentIndex = -1; // no parent means this is root bone
		this->boneHierarchy.push_back(bh);
	}
	else
	{
		// child bone
		bh.isRootBone = FALSE;
		IGameNode* boneParent = node->GetNodeParent();
		this->currentBoneDepth++;

		TCHAR* boneParentName = boneParent->GetName();
		size_t parentIndex = this->FindBoneIndexByName(boneParent->GetName());

		// Parent bone should be available from this point
		if ( parentIndex >= this->boneHierarchy.size() )
		{
			// parent bone is not available... just call recursively
			return BuildNDD_BoneHierarchy( boneParent );
		}

		BoneHierarchy* parent = &this->boneHierarchy[parentIndex];
		if (parent->firstChild == -1)
		{
			// this node is first child
			parent->firstChild = this->boneHierarchy.size();

		}
		else
		{
			size_t parentsLastChildIndex = this->FindLastSiblingBoneIndexByIndex(parent->firstChild);

			this->boneHierarchy[parentsLastChildIndex].sibling = this->boneHierarchy.size();
		}
		this->boneHierarchy.push_back(bh);
	}

	return 0;
}
size_t ModelExporter::FindBoneIndexByName(const char* boneName)
{
	size_t s;
	for (s = 0; s < this->boneHierarchy.size(); s++)
	{
		if (strcmp(boneName, this->boneHierarchy[s].boneName) == 0)
		{
			return s;
		}
	}
	return s;
}
size_t ModelExporter::FindLastSiblingBoneIndexByIndex(size_t idx)
{
	size_t sib = this->boneHierarchy[idx].sibling;
	if (sib == -1)
		return idx;
	else
		return this->FindLastSiblingBoneIndexByIndex(sib);

}
int ModelExporter::ExportLight(IGameNode* node, IGameObject* obj)
{
	IGameLight* igl = static_cast<IGameLight*>(obj);

	this->fout.write(node->GetName(), (int)(strlen(node->GetName()) + 1));
	int ffffffff = -1;
	this->fout.write((char*)&ffffffff, sizeof(int)); // vertex count set to 0xffffffff --> Non-mesh part
	this->fout.write("LIGT", 4);

	D3DLIGHT9 light;
	memset((void*)&light, 0, sizeof(light));
	GMatrix tm;
	Point3 p3Translation;
	Quat qRotation;
	Point3 p3Scaling;
	PropType propType;
	float dummyFloat = 0.0f;
	Point3 dummyPoint3;
	IGameNode* targetNode = NULL;
	D3DXQUATERNION d3dQuat;
	D3DXMATRIX d3dMatRot;
	D3DXVECTOR3 direction;
	D3DXVECTOR4 directionRotated;
	Matrix3 m3;

	switch (igl->GetLightType())
	{
	case IGameLight::IGAME_DIR:
		light.Type = D3DLIGHT_DIRECTIONAL;
		break;
	case IGameLight::IGAME_OMNI:
		light.Type = D3DLIGHT_POINT;
		break;
	case IGameLight::IGAME_FSPOT:
		light.Type = D3DLIGHT_SPOT;
		propType = igl->GetLightColor()->GetType();
		igl->GetLightColor()->GetPropertyValue(dummyPoint3);
		light.Diffuse.r = dummyPoint3.x;
		light.Diffuse.g = dummyPoint3.y;
		light.Diffuse.b = dummyPoint3.z;
		light.Diffuse.a = 1.0f;
		tm = node->GetWorldTM(); //->GetObjectTM();

		m3 = tm.ExtractMatrix3();

		DecomposeMatrix(m3, p3Translation, qRotation, p3Scaling);


		//p3Translation = tm.Translation();

		light.Position.x = p3Translation.x;
		light.Position.y = p3Translation.z;
		light.Position.z = -p3Translation.y;

		//targetNode = igl->GetLightTarget()

		//qRotation = objectTM.Rotation();
		d3dQuat.x = qRotation.x;
		d3dQuat.y = qRotation.z;
		d3dQuat.z = qRotation.y;
		d3dQuat.w = qRotation.w;
		D3DXMatrixRotationQuaternion(&d3dMatRot, &d3dQuat);

		direction.x = 0.0f;
		direction.y = 0.0f;
		direction.z = -1.0f;

		D3DXVec3Transform(&directionRotated, &direction, &d3dMatRot);

		light.Direction.x = directionRotated.x;
		light.Direction.y = directionRotated.y;
		light.Direction.z = -directionRotated.z;
		propType = igl->GetLightFallOff()->GetType();
		//igl->GetLightFallOff()->GetPropertyValue(light.Falloff);
		light.Falloff = 1.0f;
		light.Range = 100.0f;
		light.Attenuation1 = 0.200f;
		light.Phi = D3DXToRadian(40.0f);
		light.Theta = D3DXToRadian(20.0f);

		fout.write((char*)&light, sizeof(light));


		break;
	case IGameLight::IGAME_TSPOT:
		MessageBox(NULL, _T("Unsupported Object: Target Spot Light"), _T("Exporter Error"), MB_OK | MB_ICONERROR);
		break;
	case IGameLight::IGAME_TDIR:
		MessageBox(NULL, _T("Unsupported Object: Target Directional Light"), _T("Exporter Error"), MB_OK | MB_ICONERROR);
		break;
	case IGameLight::IGAME_UNKNOWN:
		MessageBox(NULL, _T("Unsupported Object: Unknown Light Type"), _T("Exporter Error"), MB_OK | MB_ICONERROR);
		break;

	}

	return 0;
}

int ModelExporter::ExportNDD_Camera( IGameNode* node )
{
	IGameObject* iobj = node->GetIGameObject();
	IGameCamera* igc = static_cast<IGameCamera*>(iobj);

	IGameProperty* propFarClip = igc->GetCameraFarClip();		// FLOAT
	IGameProperty* propNearClip = igc->GetCameraNearClip();		// FLOAT
	IGameProperty* propFOV = igc->GetCameraFOV();				// FLOAT
	
	PropType pt;
	pt = propFarClip->GetType();
	pt = propNearClip->GetType();
	pt = propFOV->GetType();

	float farClip, nearClip, fov;
	propFarClip->GetPropertyValue( farClip );
	propNearClip->GetPropertyValue( nearClip );
	propFOV->GetPropertyValue( fov );

	static char debugOutput[2048];

	sprintf_s( debugOutput, sizeof( debugOutput ), "    - Far Clip: %.3f\n", farClip );
	OutputDebugString( debugOutput );
	sprintf_s( debugOutput, sizeof( debugOutput ), "    - Near Clip: %.3f\n", nearClip );
	OutputDebugString( debugOutput );
	sprintf_s( debugOutput, sizeof( debugOutput ), "    - FOV: %.3f\n", fov );
	OutputDebugString( debugOutput );
	
	GMatrix worldTM = node->GetWorldTM();
	Matrix3 m3 = worldTM.ExtractMatrix3();
	Point3 trans;
	Quat rot;
	Point3 scale;
	
	DecomposeMatrix( m3, trans, rot, scale );

	/*IGameNode* cameraTargetNode = igc->GetCameraTarget();
	GMatrix targetWorldTM = cameraTargetNode->GetWorldTM();
	Matrix3 targetM3 = targetWorldTM.ExtractMatrix3();
	Point3 targetTrans;
	Quat targetRot;
	Point3 targetScale;
	DecomposeMatrix( targetM3, targetTrans, targetRot, targetScale );*/

	//
	// Look out for axis swizzling and z-mirroring !!!!!!!!
	//
	sprintf_s( debugOutput, sizeof( debugOutput ), "    - Position: (%.3f, %.3f, %.3f)\n", trans.x, trans.z, -trans.y );
	OutputDebugString( debugOutput );
	/*sprintf_s( debugOutput, sizeof( debugOutput ), "    - Target Position: (%.3f, %.3f, %.3f)\n", targetTrans.x, targetTrans.z, -targetTrans.y );
	OutputDebugString( debugOutput );*/
	sprintf_s( debugOutput, sizeof( debugOutput ), "    - Rotation: (%.3f, %.3f, %.3f, %.3f)\n", rot.x, rot.z, -rot.y, rot.w );
	OutputDebugString( debugOutput );
	

	float ex, ey, ez;
	rot.GetEuler( &ex, &ey, &ez );
	
	rot.SetEuler( ex, ez, -ey );

	rot.GetEuler( &ex, &ey, &ez );
	ex = D3DXToDegree( ex );
	ey = D3DXToDegree( ey );
	ez = D3DXToDegree( ez );
	
	//rot = worldTM.Rotation();
	Matrix3 rotMat;
	rot.MakeMatrix( rotMat );

	Point3 upVector( 0.0f, 1.0f, 0.0f );
	Point3 lookAtVector( 0.0f, 0.0f, -1.0f );

	Point3 upVectorTrans = TransformVector(GMatrix(rotMat), upVector);
	Point3 lookAtVectorTrans = TransformVector(GMatrix(rotMat), lookAtVector);
	
	//
	// Look out for axis swizzling and z-mirroring !!!!!!!!
	//
	ARN_NDD_CAMERA_CHUNK camChunk;
	camChunk.pos.x = trans.x;				camChunk.pos.y = trans.z;				camChunk.pos.z = -trans.y;
	//camChunk.targetPos.x = targetTrans.x;	camChunk.targetPos.y = targetTrans.z;	camChunk.targetPos.z = -targetTrans.y;
	//camChunk.rot.x = rot.x;					camChunk.rot.y = rot.y;					camChunk.rot.z = rot.z;				camChunk.rot.w = rot.w;
	
	camChunk.upVector.x = upVectorTrans.x;
	camChunk.upVector.y = upVectorTrans.y;
	camChunk.upVector.z = upVectorTrans.z;

	camChunk.lookAtVector.x = lookAtVectorTrans.x;
	camChunk.lookAtVector.y = lookAtVectorTrans.y;
	camChunk.lookAtVector.z = lookAtVectorTrans.z;
	
	camChunk.farClip = farClip;
	camChunk.nearClip = nearClip;
	
	int ndt = NDT_CAMERA;
	int chunkSize = 12 + 12 + 16 + 12 + 12 + 4 + 4;
	
	this->fout.write( (char*)&ndt, sizeof( int ) );
	this->fout.write( node->GetName(), (int)( strlen( node->GetName() ) ) + 1 );
	this->fout.write( (char*)&chunkSize, sizeof( int ) );
	this->fout.write( (char*)&camChunk, sizeof ( camChunk ) );

	this->nodeCount++;
	
	return 0;
}
int ModelExporter::ExportNDD_Anim(IGameNode* node)
{
	HRESULT hr;
	const DWORD tick = 4800;
	const int fps = 30;
	const int tickPerFPS = tick / fps;

	// TODO: animation frame range control
	const int startFrame = g_animStartFrame;
	const int endFrame = g_animEndFrame;
	const int frameCount = endFrame - startFrame + 1;

	int i = 0;

	//
	// Write NDD header
	//
	std::string animNodeName("Anim-");
	animNodeName += node->GetName();

	int ndt = g_ndtAnim; // set by settings dialog
	int chunkSize = -1;
	this->fout.write((char*)&ndt, sizeof(int));
	this->fout.write(animNodeName.c_str(), (int)(strlen(animNodeName.c_str()) + 1));
	this->fout.write((char*)&chunkSize, sizeof(int)); // reserve for chuck size
	DWORD chuckStartPos = this->fout.tellp();

	////
	//// Write down Animation Data Definition (ADD) for ARN format
	////
	//// Rotation-Scaling-Translation Data
	//this->fout.write("ANIM", 4);

	//// Frame range
	//this->fout.write((char*)&startFrame, sizeof(int));
	//this->fout.write((char*)&endFrame, sizeof(int));

	// write down Keyframe Data Definition (KDD) array

	Point3 translation;
	Quat rotation, oldRotation;
	Point3 scale;

	if (ndt == NDT_ANIM1)
	{
		this->fout.write((char*)&frameCount, sizeof(int));

		for (i = startFrame; i <= endFrame; i++)
		{
			GMatrix tm = node->GetWorldTM(tickPerFPS * i); // get current frame's world transformation matrix
			
			
			//  decompose it to RST matrix
			Matrix3 m3 = tm.ExtractMatrix3();
			DecomposeMatrix(m3, translation, rotation, scale);

			if ((i > startFrame) && (dot(rotation, oldRotation) < 0))
			{
				// take the short way around (VERY IMPORTANT!!!)
				rotation = Quat(-rotation.x, -rotation.y, -rotation.z, -rotation.w);
			}
			oldRotation = rotation;

			
			this->fout.write((char*)&rotation, sizeof(rotation));
			this->fout.write((char*)&scale, sizeof(scale));
			this->fout.write((char*)&translation, sizeof(translation));
		}



		//for (i = startFrame; i <= endFrame; i++)
		//{
		//	GMatrix tm = node->GetWorldTM(tickPerFPS * i);// * this->maxToDx; // get current frame's world transformation matrix

		//	//  decompose it to RST matrix
		//	Matrix3 m3 = tm.ExtractMatrix3();
		//	DecomposeMatrix(m3, translation, rotation, scale);

		//	Point3 rotationAxis = rotation.Vector();

		//	float ax, ay, az;
		//	rotation.GetEuler(&ax, &ay, &az);
		//	rotation.SetEuler(-ax, ay, -az);

		//	translation.y *= -1.0f;


		//	Point3 rotationAx;
		//	float s = sqrtf(rotation.x*rotation.x + rotation.y*rotation.y + rotation.z*rotation.z);
		//	rotationAx.x = rotation.x/s;
		//	rotationAx.y = rotation.y/s;
		//	rotationAx.z = rotation.z/s;
		//	float angle = 2*acosf(rotation.w);
		//	float angleDeg = D3DXToDegree(angle);


		//	if ((i > startFrame) && (dot(rotation, oldRotation) < 0)) {
		//		//  take the short way around
		//		rotation = Quat(-rotation.x, -rotation.y, -rotation.z, -rotation.w);
		//	}
		//	oldRotation = rotation;


		//	//DebugPrint(_T("Ticks: %d / R: (%.4f, %.4f, %.4f) %.4f ; axis(%.4f,%.4f,%.4f); deg(%.4f)/ S: (%.4f, %.4f, %.4f) / T: (%.4f %.4f %.4f)\n"),
		//	//	tick*i/fps,
		//	//	//rotation.x, rotation.y, rotation.z, rotation.w,
		//	//	rotation.Vector().x,rotation.Vector().y,rotation.Vector().z,rotation.w,
		//	//	rotationAx.x,rotationAx.y,rotationAx.z,angleDeg,
		//	//	scale.x, scale.y, scale.z,
		//	//	translation.x, translation.y, translation.z);

		//	//
		//	// CAUTION: AXIS SWAPPING occurs instead of matrix calculation
		//	//

		//	//////////////////////////////////////////


		//	//rotation.z *= -1;
		//	this->fout.write((char*)&rotation.x, sizeof(float));
		//	this->fout.write((char*)&rotation.z, sizeof(float));
		//	this->fout.write((char*)&rotation.y, sizeof(float));
		//	this->fout.write((char*)&rotation.w, sizeof(float));


		//	this->fout.write((char*)&scale.x, sizeof(float));
		//	this->fout.write((char*)&scale.z, sizeof(float));
		//	this->fout.write((char*)&scale.y, sizeof(float));

		//	this->fout.write((char*)&translation.x, sizeof(float));
		//	this->fout.write((char*)&translation.z, sizeof(float));
		//	this->fout.write((char*)&translation.y, sizeof(float));

		//}
	}
	else
	{
		MessageBox(NULL, _T("Not implemented type"), _T("Settings Error"), MB_OK | MB_ICONERROR);
		hr = E_FAIL;
		goto e_Exit;
	}


	//
	// calculate chunk size and write it
	//
	DWORD chuckEndPos = this->fout.tellp();
	chunkSize = chuckEndPos - chuckStartPos;
	this->fout.seekp(chuckStartPos-sizeof(int), std::ios_base::beg);
	this->fout.write((char*)&chunkSize, sizeof(int));
	this->fout.seekp(chuckEndPos, std::ios_base::beg);

	hr = S_OK;

e_Exit:
	return hr;
}

int ModelExporter::ExportMeshARN20(IGameNode* node)
{
	IGameObject* obj = node->GetIGameObject();
	IGameMesh* igm = static_cast<IGameMesh*>(obj);
	IGameSkin* igs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : NULL;
	static char buffer[4096];
	

	igm->SetCreateOptimizedNormalList();

	if ( !igm->InitializeData() )
	{
		_snprintf_s( buffer, 4095, 4095, "Could not read mesh data from '%s'\n", node->GetName() );
		buffer[4095] = '\0';
		DebugPrint( _T( buffer ) );
	}

	//DWORD numFacesUsed = 0;
	const DWORD numFaces = igm->GetNumberOfFaces();

	if (numFaces <= 0)
	{
		DebugPrint(_T(" - Node '%s' has no faces. Skip exporting this node...\n"), node->GetName());
		return -1;
	}

	const DWORD numVertices = numFaces * 3;
	std::vector<DWORD> indices; // vertex indices
	std::vector<int> materialIndices; // one material index per three vertex indices
	indices.resize(numVertices);

	FULL_VERTEX fv;
	std::vector<FULL_VERTEX> condensedVertices;
	std::vector<std::pair<float, DWORD>> condensedVerticesAcc; // help for speed-up optimization of vertices
	std::vector<ARN_MTD> materialList;


	

	int i, j, k;
	
	for (i = 0; i < (int)numFaces; i++)
	{

		FaceEx* faceEx = igm->GetFace(i);
		IGameMaterial* igmat = igm->GetMaterialFromFace(faceEx);
		TCHAR* matName = NULL;
		TCHAR* matClassName = NULL;

		//DebugPrint(_T("Face #%d - MatID: %d"), i, faceEx->matID);

		int matID = -1;
		if (igmat != NULL)
		{
			//DebugPrint(_T(" (has material info: %s"), igmat->GetMaterialName());

			matName = igmat->GetMaterialName();
			matClassName = igmat->GetMaterialClass();

			ARN_MTD cm; // "current" material
			cm.strTexFileName = _T("");

			if (igmat->GetNumberOfTextureMaps() != 0)
			{

				IGameTextureMap* igtm = igmat->GetIGameTextureMap(0);
				TCHAR* textureFileName = igtm->GetBitmapFileName();
				//DebugPrint(_T(", %s)\n"), textureFileName);
				cm.strTexFileName = textureFileName;
				igtm = NULL;
			}
			else
			{
				//DebugPrint(_T(")\n"));
			}

			// insert to material list
			bool isFirstMaterial = true;
			for (k = 0; k < (int)materialList.size(); k++)
			{

				if (materialList[k].strMatName.compare(matName) == 0)
				{
					isFirstMaterial = false;
					break;
				}
			}
			if (isFirstMaterial == true)
			{

				Point3 p3;
				cm.strMatName = matName;
				if ( igmat->GetDiffuseData() != NULL )
				{
					igmat->GetDiffuseData()->GetPropertyValue( p3 );
					cm.d3dMat.Diffuse.r = p3.x;
					cm.d3dMat.Diffuse.g = p3.y;
					cm.d3dMat.Diffuse.b = p3.z;
				}
				else
				{
					ZeroMemory( &cm.d3dMat.Diffuse, sizeof( D3DCOLORVALUE ) );
				}
				cm.d3dMat.Diffuse.a = 1.0f;


				if ( igmat->GetAmbientData() != NULL)
				{
					igmat->GetAmbientData()->GetPropertyValue( p3 );
					cm.d3dMat.Ambient.r = p3.x;
					cm.d3dMat.Ambient.g = p3.y;
					cm.d3dMat.Ambient.b = p3.z;
				}
				else
				{
					ZeroMemory( &cm.d3dMat.Ambient, sizeof( D3DCOLORVALUE ) );
				}
				cm.d3dMat.Ambient.a = 1.0f;

				if ( igmat->GetSpecularData() != NULL )
				{
					igmat->GetSpecularData()->GetPropertyValue( p3 );
					cm.d3dMat.Specular.r = p3.x;
					cm.d3dMat.Specular.g = p3.y;
					cm.d3dMat.Specular.b = p3.z;
				}
				else
				{
					ZeroMemory( &cm.d3dMat.Specular, sizeof( D3DCOLORVALUE ) );
				}
				cm.d3dMat.Specular.a = 1.0f;


				if ( igmat->GetEmissiveData() != NULL )
				{
					igmat->GetEmissiveData()->GetPropertyValue( p3 );
					cm.d3dMat.Emissive.r = p3.x;
					cm.d3dMat.Emissive.g = p3.y;
					cm.d3dMat.Emissive.b = p3.z;
				}
				else
				{
					ZeroMemory( &cm.d3dMat.Emissive, sizeof( D3DCOLORVALUE ) );
				}
				cm.d3dMat.Emissive.a = 1.0f;

				//PropType pt = igmat->GetOpacityData()->GetType();

				materialList.push_back(cm);

			}

		}
		else
		{
			//DebugPrint(_T("\n"));
		}

		// three vertices per face
		for (j = 0; j < 3; j++)
		{
			ZeroMemory(&fv, sizeof(fv));
			fv.origVert = faceEx->vert[j]; // vert id

			// normal routine
			//fv.pos = igm->GetVertex(faceEx->vert[j], false) * this->maxToDx;
			//fv.normal = ModelExporter::TransformVector(this->maxToDx, igm->GetNormal(faceEx->norm[j], false));

			//////////////////////////////////////////////////////////////////////////
			// TODO: experimental (vertex transform)
			/*GMatrix localTM = node->GetLocalTM();
			GMatrix objectTM = node->GetObjectTM();
			GMatrix worldTM = node->GetWorldTM();*/
			fv.pos = ModelExporter::TransformVector4(node->GetWorldTM().Inverse(), Point4(igm->GetVertex(faceEx->vert[j], false), 1.0f)); // * this->maxToDx;
			fv.normal = igm->GetNormal(faceEx->norm[j], false);
			//////////////////////////////////////////////////////////////////////////

			Point2 texCoord = igm->GetTexVertex(faceEx->texCoord[j]);

			ZeroMemory(&fv.color, sizeof(fv.color));

			fv.uvw.x = texCoord.x;
			fv.uvw.y = texCoord.y;

			indices[i * 3 + j] = this->AddCondensedVertex(condensedVertices, condensedVerticesAcc, fv);

		}

		bool isRightMaterialName = false;

		for (k = 0; k < (int)materialList.size(); k++)
		{
			if (igmat == NULL) break;

			if (materialList[k].strMatName.compare(igmat->GetMaterialName()) == 0)
			{
				isRightMaterialName = true;
				//this->fout.write((char*)&k, sizeof(int));
				materialIndices.push_back(k);
				break;
			}
		}
		if (isRightMaterialName != true)
		{
			int ffffffff = -1;
			//this->fout.write((char*)&ffffffff, sizeof(int));
			materialIndices.push_back(-1);
			DebugPrint(_T("Material Not Found!"));
		}

	}

	//  Calculate mapping from old vertex index to new vertex indices
	ZeroMemory(&this->currentMeshRemap, sizeof(Remap));
	if (condensedVertices.size() > 0)
	{
		FULL_VERTEX* fvp = &condensedVertices[0];
		FULL_VERTEX* base = fvp;
		FULL_VERTEX* end = fvp + condensedVertices.size();

		int mo = -1;
		while (fvp < end)
		{
			if (mo < fvp->origVert)
			{
				mo = fvp->origVert;
			}
			++fvp;
		}
		this->currentMeshRemap.SetOldCount(mo + 1);
		fvp = base;
		while (fvp < end)
		{
			this->currentMeshRemap.MapOldToNew((int)fvp->origVert, (int)(fvp-base));
			++fvp;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WRITE NDD (Mesh-ARN20)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Write NDD header
	//
	int ndt = NDT_MESH2;
	int chunkSize = -1;
	this->fout.write((char*)&ndt, sizeof(int));
	this->fout.write(node->GetName(), (int)(strlen(node->GetName()) + 1));
	this->fout.write((char*)&chunkSize, sizeof(int)); // reserve for chuck size
	DWORD chuckStartPos = this->fout.tellp();

	//
	// Write down Indexed VDD
	//
	int condensedVerticesSize = (int)condensedVertices.size();
	this->currentMeshCondensedVerticesCount = condensedVerticesSize;

	this->fout.write((char*)&condensedVerticesSize, sizeof(int));
	for (i = 0; i < condensedVerticesSize; i++)
	{
		Custom_FVF fvf;
		fvf.color = 0xffffffff;
		fvf.vertex = condensedVertices[i].pos;
		fvf.normal = condensedVertices[i].normal;
		fvf.u = condensedVertices[i].uvw.x;
		fvf.v = condensedVertices[i].uvw.y;
		this->fout.write((char*)&fvf, sizeof(Custom_FVF));
	}

	int indicesSize = (int)indices.size();
	int materialIndicesSize = (int)materialIndices.size();
	ASSERTCHECK(indicesSize == materialIndicesSize * 3);

	//
	// Write down Vertex Index
	//
	// face number cannot be induced by the number of indexed vertices, so should be written to file
	this->fout.write((char*)&numFaces, sizeof(int));
	this->fout.write((char*)&indices[0], sizeof(DWORD) * (int)indices.size());

	//
	// Write down Face-Material Table (FMT)
	//
	this->fout.write((char*)&materialIndices[0], sizeof(int) * (int)materialIndices.size());

	//
	// Write down Material & Texture Definition (MTD)
	//
	int materialListSize = (int)materialList.size();
	this->fout.write((char*)&materialListSize, sizeof(int));
	for (i = 0; i < materialListSize; i++)
	{
		// material name
		this->fout.write(materialList[i].strMatName.c_str(), (int)(materialList[i].strMatName.length() + 1));
		// struct D3DMATERIAL9
		this->fout.write((char*)&materialList[i].d3dMat, sizeof(D3DMATERIAL9));
		// texture file full path
		this->fout.write(materialList[i].strTexFileName.c_str(), (int)(materialList[i].strTexFileName.length() + 1));
	}


	//
	// - Animation Data Definition (ADD)
	//
	// is processed by following function;
	//
	this->ExportNDD_Anim(node);

	//
	// calculate chunk size and write it
	//
	DWORD chuckEndPos = this->fout.tellp();
	chunkSize = chuckEndPos - chuckStartPos;
	this->fout.seekp(chuckStartPos-sizeof(int), std::ios_base::beg);
	this->fout.write((char*)&chunkSize, sizeof(int));
	this->fout.seekp(chuckEndPos, std::ios_base::beg);

	return 0;
}

HRESULT ModelExporter::BuildSkinningData( IGameNode* meshNodeToBeSkinned, std::vector<ARN_VDD_S>& arnVDDS )
{
	// Skinning Data Builder for Export Node Data Definition (Mesh Type 3; Skin Integrated)
	// Author: Geoyeob Kim
	// Date: 2007-12-30
	IGameObject* iobj = meshNodeToBeSkinned->GetIGameObject();
	IGameSkin* igs = iobj->IsObjectSkinned() ? iobj->GetIGameSkin() : NULL;

	if ( igs == NULL )
	{
		return -1;
	}

	std::vector<std::vector<BoneWeight>> assignments;
	std::map<IGameNode*, int> boneIndices;
	std::map<IGameNode*, int>::iterator ptr, end;
	const int numOfSkinnedVerts = igs->GetNumOfSkinnedVerts();

	BoneWeight boneWeightsPerVertex[ModelExporter::MAX_BONE_WEIGHTS_PER_VERTEX];

	this->numberOfBonesInfluencingMax = -1;

	static char debugOutput[2048];
	sprintf_s( debugOutput, sizeof( debugOutput ), " --- Start building skin data of %s\n", meshNodeToBeSkinned->GetName() );
	OutputDebugString( debugOutput );

	sprintf_s( debugOutput, sizeof( debugOutput ), " --- Total Bone Count: %d\n", igs->GetTotalSkinBoneCount() );
	OutputDebugString( debugOutput );
	sprintf_s( debugOutput, sizeof( debugOutput ), " --- Total \"Effective\"Bone Count: %d\n", igs->GetTotalBoneCount() );
	OutputDebugString( debugOutput );
		

	sprintf_s( debugOutput, sizeof( debugOutput ), " - Skinned Vertices Count: %d\n", numOfSkinnedVerts );
	OutputDebugString( debugOutput );

	int i, j;
	for ( i = 0; i < numOfSkinnedVerts; i++ )
	{

		int numberOfBonesInfluencing = igs->GetNumberOfBones( i );
		
		
		if ( numberOfBonesInfluencing > ModelExporter::MAX_BONE_WEIGHTS_PER_VERTEX )
		{
			OutputDebugString( _T( " - Exporter warning: MAX_BONE_WEIGHTS_PER_VERTEX exceeded\n" ) );
			numberOfBonesInfluencing = ModelExporter::MAX_BONE_WEIGHTS_PER_VERTEX;
		}

		sprintf_s( debugOutput, sizeof( debugOutput ), " - Vertex: %d, ", i );
		OutputDebugString( debugOutput );

		ASSERTCHECK( numberOfBonesInfluencing > 0 );
		for ( j = 0; j < numberOfBonesInfluencing; j++ )
		{
			IGameNode* boneNode = igs->GetIGameBone( i, j );
			
			sprintf_s( debugOutput, sizeof( debugOutput ), " %s(%.3f) ... ", boneNode->GetName(), igs->GetWeight(i, j) );
			OutputDebugString( debugOutput );
			
			ptr = boneIndices.find( boneNode ); // check for existance of this bone node
			if ( ptr != boneIndices.end() )
			{
				// exist
				boneWeightsPerVertex[j].index = ptr->second;
			}
			else
			{
				// new bone
				boneWeightsPerVertex[j].index = (int)boneIndices.size();
				boneIndices[boneNode] = boneWeightsPerVertex[j].index;
				assignments.push_back(std::vector<BoneWeight>());
			}
			boneWeightsPerVertex[j].weight = igs->GetWeight(i, j);
		}
		OutputDebugString( "\n" );
		std::sort(boneWeightsPerVertex, &boneWeightsPerVertex[numberOfBonesInfluencing]);
	}

	return S_OK;
}


HRESULT ModelExporter::ExportNDD_SkinningData(IGameNode* meshNodeToBeSkinned)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Export bones(skeleton) skinning data if exists
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IGameObject* obj = meshNodeToBeSkinned->GetIGameObject();
	IGameSkin* igs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : NULL;

	if (igs == NULL)
		return -1;

	std::string skeletonNodeName("Skeleton-");
	skeletonNodeName += meshNodeToBeSkinned->GetName();


	//
	// Write NDD header
	//
	int ndt = NDT_SKELETON;
	int chunkSize = -1;
	this->fout.write((char*)&ndt, sizeof(int));
	this->fout.write(skeletonNodeName.c_str(), (int)strlen(skeletonNodeName.c_str())+1); // write skeleton name
	this->fout.write((char*)&chunkSize, sizeof(int)); // reserve for chuck size
	DWORD chuckStartPos = this->fout.tellp();

	//this->fout.write((char*)&ffffffff, 4); // write code
	//this->fout.write("SKEL", 4); // write node descriptor

	this->fout.write(meshNodeToBeSkinned->GetName(), (int)strlen(meshNodeToBeSkinned->GetName())+1); // associated mesh name

	std::vector<std::vector<BoneWeight>> assignments;
	std::map<IGameNode*, int> boneIndices;
	std::map<IGameNode*, int>::iterator ptr, end;
	const int numOfSkinnedVerts = igs->GetNumOfSkinnedVerts();

	BoneWeight boneWeightsPerVertex[this->MAX_BONE_WEIGHTS_PER_VERTEX];

	this->numberOfBonesInfluencingMax = -1;

	int i, j;
	for (i = 0; i < numOfSkinnedVerts; i++)
	{
		int numberOfBonesInfluencing = igs->GetNumberOfBones(i);
		if (numberOfBonesInfluencing > this->MAX_BONE_WEIGHTS_PER_VERTEX)
			numberOfBonesInfluencing = this->MAX_BONE_WEIGHTS_PER_VERTEX;

		for (j = 0; j < numberOfBonesInfluencing; j++)
		{
			IGameNode* boneNode = igs->GetIGameBone(i, j);
			TCHAR* boneNodeName = boneNode->GetName();
			
			BuildNDD_BoneHierarchy( boneNode );
			
			ptr = boneIndices.find(boneNode); // check for existing this bone node
			if (ptr != boneIndices.end())
			{
				boneWeightsPerVertex[j].index = ptr->second;
			}
			else
			{
				boneWeightsPerVertex[j].index = (int)boneIndices.size();
				boneIndices[boneNode] = boneWeightsPerVertex[j].index;
				assignments.push_back(std::vector<BoneWeight>());
			}
			boneWeightsPerVertex[j].weight = igs->GetWeight(i, j);
		}
		std::sort(boneWeightsPerVertex, &boneWeightsPerVertex[numberOfBonesInfluencing]);

		if (numberOfBonesInfluencing < this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING)
		{
			ZeroMemory(&boneWeightsPerVertex[numberOfBonesInfluencing], (this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING - numberOfBonesInfluencing) * sizeof(BoneWeight));
			if (numberOfBonesInfluencing > this->numberOfBonesInfluencingMax)
			{
				this->numberOfBonesInfluencingMax = numberOfBonesInfluencing;
			}
		}
		else
		{
			this->numberOfBonesInfluencingMax = this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING;
		}

		float weightSum = 0.0f;
		for (j = 0; j < this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING; j++)
			weightSum += boneWeightsPerVertex[j].weight;
		weightSum = 1.0f / weightSum; // reciprocal
		// normalize weights
		for (j = 0; j < this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING; j++)
			boneWeightsPerVertex[j].weight = boneWeightsPerVertex[j].weight * weightSum;

		for (j = 0; j < this->MAX_BONE_WEIGHTS_PER_VERTEX_INFLUENCING; j++)
		{
			BoneWeight bw;
			bw.index = i; // i; vertex index
			bw.weight = boneWeightsPerVertex[j].weight;
			// only include that has effective weight
			if (bw.weight > 0.0f)
				assignments[boneWeightsPerVertex[j].index].push_back(bw);
		}
	}

	// count number of bones that affect this mesh
	int numAffect = 0;
	size_t s;
	for (s = 0; s < assignments.size(); s++)
	{
		if (assignments[s].size() > 0)
			numAffect++;
	}

	this->fout.write((char*)&this->numberOfBonesInfluencingMax, sizeof(int)); // same as number of weights influencing a vertex
	this->fout.write((char*)&numAffect, sizeof(int)); // number of bones affecting mesh. same as BDD array size. same as bone count

	//
	// iterate by bone count
	// write down Bone Data Definition array
	//
	for (ptr = boneIndices.begin(), end = boneIndices.end(); ptr != end; ptr++)
	{
		IGameNode* boneNode = ptr->first;


		this->fout.write(boneNode->GetName(), (int)strlen(boneNode->GetName()) + 1); // write Bone Name
		GMatrix boneInitTM = boneNode->GetWorldTM();
		//boneInitTM = boneInitTM * this->maxToDx;

		boneInitTM *= meshNodeToBeSkinned->GetWorldTM().Inverse();

		Matrix3 m3 = boneInitTM.ExtractMatrix3();
		Point3 translation;
		Quat rotation;
		Point3 scale;
		DecomposeMatrix(m3, translation, rotation, scale);
		D3DXVECTOR3 vTrans(translation.x, translation.z, translation.y);
		D3DXVECTOR3 vScale(scale.x, scale.z, scale.y);
		D3DXQUATERNION qRot(rotation.x, rotation.z, rotation.y, rotation.w);

		D3DXMATRIX matFinal;
		D3DXMatrixTransformation(&matFinal, NULL, NULL, &vScale, NULL, &qRot, &vTrans);

		// resolve axis transform problem
		memcpy(&matFinal, &boneInitTM, sizeof(float)*4*4);
		
		// TODO: ambiguous offset matrix usage
		//this->fout.write((char*)&boneInitTM[0].x, 4*4*sizeof(float)); // offset matrix (maybe?)
		this->fout.write((char*)&matFinal, sizeof(matFinal));


		int boneIndex = ptr->second;
		std::vector<BoneWeight>& bwv = assignments[boneIndex];
		DWORD bwvSize = (DWORD)bwv.size();
		if (bwvSize >= 0)
		{
			std::vector<DWORD> skinIndices;
			std::vector<float> skinWeights;
			BoneWeight* bw = &bwv[0];

			DWORD d;
			for (d = 0; d < bwvSize; d++)
			{
				int oldIndex = bw[d].index;
				float weight = bw[d].weight;

				// each old vertex may generate more than one new vertex
				for (j = 0; j < 8; j++)
				{
					int newIndex = this->currentMeshRemap.GetNew(oldIndex, j);
					if (newIndex == -1) break;
					ASSERTCHECK(newIndex < this->currentMeshCondensedVerticesCount);
					skinIndices.push_back(newIndex);
					skinWeights.push_back(weight);
				}
			}

			ASSERTCHECK(skinIndices.size() == skinWeights.size());
			size_t skinIndicesSize = skinIndices.size();
			this->fout.write((char*)&skinIndicesSize, sizeof(size_t)); // write influencing vertex count
			this->fout.write((char*)&skinIndices[0], (int)skinIndices.size() * sizeof(DWORD)); // write influencing skin vertex indices
			this->fout.write((char*)&skinWeights[0], (int)skinWeights.size() * sizeof(float)); // write influencing skin vertex weights


			//
			// write animation data definition (ADD)
			//
			this->ExportNDD_Anim(boneNode);
		}

	}


	//
	// calculate chunk size and write it
	//
	DWORD chuckEndPos = this->fout.tellp();
	chunkSize = chuckEndPos - chuckStartPos;
	this->fout.seekp(chuckStartPos-sizeof(int), std::ios_base::beg);
	this->fout.write((char*)&chunkSize, sizeof(int));
	this->fout.seekp(chuckEndPos, std::ios_base::beg);

	this->nodeCount++;

	return 0;
}
DWORD ModelExporter::AddCondensedVertex( std::vector<FULL_VERTEX>& vec, std::vector<std::pair<float, DWORD> >& acc, FULL_VERTEX const& fv )
{
	size_t n = vec.size();
	size_t i = 0;
	std::pair<float, DWORD> p(fv.pos.x, 0);
	std::pair<float, DWORD>* base = NULL;

	if (acc.size() > 0) {
		std::pair<float, DWORD>* low = &acc[0];
		std::pair<float, DWORD>* high = low+acc.size();
		base = low;
		for (;;) {
			std::pair<float, DWORD> * mid = low + (high-low)/2; // get the middle of low-high range of 'acc'
			if (mid == low) { // mid and low point to the same
				i = mid-&acc[0]; // set i to the mid's index and break the infinite for loop
				break;
			}

			// make range narrower by comparing x value(.first) of mid and p
			if (mid->first < p.first) {
				low = mid;
			}
			else {
				high = mid;
			}
		}
	}


	for (; i < n; ++i) {
		if (vec[base[i].second].pos.x > fv.pos.x) {
			//  no need searching more
			i = base[i].second;
			break;
		}
		if (!memcmp(&vec[base[i].second], &fv, sizeof(fv))) {
			return base[i].second;
		}
	}

	p.second = (DWORD)vec.size(); // set p.second to current vec's size


#if !defined(NDEBUG)
	// check for duplicated vertex (should not exist)
	size_t nv = vec.size();
	for (size_t i = 0; i < nv; ++i) {
		assert( memcmp(&fv, &vec[i], sizeof(FULL_VERTEX)) );
	}
#endif

	acc.push_back(p);
	std::inplace_merge(&acc[0], &acc[acc.size()-1], &acc[0]+acc.size());
	vec.push_back(fv);

	return p.second;
}


int ModelExporter::ExportMeshARN10(IGameNode* node)
{
	IGameObject* obj = node->GetIGameObject();
	IGameMesh* igm = static_cast<IGameMesh*>(obj);
	IGameSkin* igs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : 0; // unused; ARN10 does not support skinned mesh
	static char buf[4096];

	igm->SetCreateOptimizedNormalList();
	if (!igm->InitializeData())
	{
		_snprintf_s(buf, 4095, 4095, "Could not read mesh data from '%s'\n", node->GetName());
		buf[4095] = '\0';
		DebugPrint(_T(buf));
	}

	const DWORD faceCount = igm->GetNumberOfFaces();
	if (faceCount <= 0)
	{
		DebugPrint(_T(" - Node '%s' has no faces. Skip exporting this node...\n"), node->GetName());
		return -1;
	}

	const DWORD vertCount = faceCount * 3;


	//
	// Write NDD header
	//
	int ndt = NDT_MESH1;
	int chunkSize = -1;
	this->fout.write((char*)&ndt, sizeof(int));
	this->fout.write(node->GetName(), (int)(strlen(node->GetName()) + 1));
	this->fout.write((char*)&chunkSize, sizeof(int)); // reserve for chuck size
	DWORD chuckStartPos = this->fout.tellp();

	DebugPrint(_T("Total Vertices: %d\n"), vertCount);


	int i, j, k;
	Custom_FVF fvf;

	//GMatrix this->maxToDx;

	/*this->maxToDx.SetRow(0, Point4(1, 0, 0, 0));
	this->maxToDx.SetRow(1, Point4(0, 0, -1, 0));
	this->maxToDx.SetRow(2, Point4(0, 1, 0, 0));
	this->maxToDx.SetRow(3, Point4(0, 0, 0, 1));*/

	std::vector<ARN_MTD> materialList;

	//
	// Write total vertices count(int)
	//
	this->fout.write((char*)&vertCount, sizeof(vertCount));

	int vertsDataStartingOffset = this->fout.tellp(); // 버텍스 정보가 시작되는 오프셋 저장

	for (i = 0; i < (int)faceCount; i++)
	{
		FaceEx* face = igm->GetFace(i);

		IGameMaterial* igmat = igm->GetMaterialFromFace(face);
		TCHAR* matName = NULL;
		TCHAR* matClassName = NULL;

		DebugPrint(_T("Face #%d - MatID: %d"), i, face->matID);

		int matID = -1;
		if (igmat != NULL)
		{
			DebugPrint(_T(" (has material info: %s"), igmat->GetMaterialName());

			//matID = igmat->GetMaterialID(0);
			matName = igmat->GetMaterialName();
			matClassName = igmat->GetMaterialClass();

			ARN_MTD cm;
			cm.strTexFileName = _T("");

			if (igmat->GetNumberOfTextureMaps() != 0)
			{

				IGameTextureMap* igtm = igmat->GetIGameTextureMap(0);
				TCHAR* textureFileName = igtm->GetBitmapFileName();
				DebugPrint(_T(", %s)\n"), textureFileName);
				cm.strTexFileName = textureFileName;
				igtm = NULL;
			}
			else
			{
				DebugPrint(_T(")\n"));
			}

			// insert to material list
			bool isFirstMaterial = true;
			for (k = 0; k < (int)materialList.size(); k++)
			{

				if (materialList[k].strMatName.compare(matName) == 0)
				{
					isFirstMaterial = false;
					break;
				}
			}
			if (isFirstMaterial == true)
			{

				Point3 p3;
				cm.strMatName = matName;
				igmat->GetDiffuseData()->GetPropertyValue(p3);
				cm.d3dMat.Diffuse.r = p3.x;
				cm.d3dMat.Diffuse.g = p3.y;
				cm.d3dMat.Diffuse.b = p3.z;
				cm.d3dMat.Diffuse.a = 1.0f;

				igmat->GetAmbientData()->GetPropertyValue(p3);
				cm.d3dMat.Ambient.r = p3.x;
				cm.d3dMat.Ambient.g = p3.y;
				cm.d3dMat.Ambient.b = p3.z;
				cm.d3dMat.Ambient.a = 1.0f;

				igmat->GetSpecularData()->GetPropertyValue(p3);
				cm.d3dMat.Specular.r = p3.x;
				cm.d3dMat.Specular.g = p3.y;
				cm.d3dMat.Specular.b = p3.z;
				cm.d3dMat.Specular.a = 1.0f;

				igmat->GetEmissiveData()->GetPropertyValue(p3);
				cm.d3dMat.Emissive.r = p3.x;
				cm.d3dMat.Emissive.g = p3.y;
				cm.d3dMat.Emissive.b = p3.z;
				cm.d3dMat.Emissive.a = 1.0f;

				PropType pt = igmat->GetOpacityData()->GetType();


				materialList.push_back(cm);

			}

		}
		else
		{
			DebugPrint(_T("\n"));
		}

		// three vertices per face (Vertex Data Definition)
		for (j = 0; j < 3; j++)
		{
			// normal routine
			//fvf.vertex = igm->GetVertex(face->vert[j], false) * this->maxToDx;
			//fvf.normal = ModelExporter::TransformVector(this->maxToDx, igm->GetNormal(face->norm[j], false));

			//////////////////////////////////////////////////////////////////////////
			// TODO: experimental (vertex transform)
			/*GMatrix localTM = node->GetLocalTM();
			GMatrix objectTM = node->GetObjectTM();
			GMatrix worldTM = node->GetWorldTM();*/
			fvf.vertex = ModelExporter::TransformVector4(node->GetWorldTM().Inverse(), Point4(igm->GetVertex(face->vert[j], false), 1.0f)); // * this->maxToDx;
			fvf.normal = igm->GetNormal(face->norm[j], false);
			//////////////////////////////////////////////////////////////////////////

			Point2 texCoord = igm->GetTexVertex(face->texCoord[j]);

			fvf.color = 0xffffffff;

			fvf.u = texCoord.x;
			fvf.v = texCoord.y;

			//DebugPrint("v %5.2f, %5.2f, %5.2f n %5.2f %5.2f %5.2f\n", fvf.vertex.x, fvf.vertex.y, fvf.vertex.z, fvf.normal.x, fvf.normal.y, fvf.normal.z);

			//
			// Write down Vertex Data Definition (VDD)
			//
			this->fout.write((const char*)&fvf, sizeof(Custom_FVF));
		}

		//
		// Write down Face-Material Table (FMT)
		//
		this->fout.seekp(vertsDataStartingOffset + vertCount * sizeof(Custom_FVF) + i * sizeof(int), std::ios_base::beg);
		bool isRightMaterialName = false;

		for (k = 0; k < (int)materialList.size(); k++)
		{
			if (igmat == NULL) break;

			if (materialList[k].strMatName.compare(igmat->GetMaterialName()) == 0)
			{
				isRightMaterialName = true;
				this->fout.write((char*)&k, sizeof(int));
				break;
			}
		}
		if (isRightMaterialName != true)
		{
			int ffffffff = -1;
			this->fout.write((char*)&ffffffff, sizeof(int));
			DebugPrint(_T("Material Not Found!"));
		}

		this->fout.seekp(vertsDataStartingOffset + 3 * (i+1) * sizeof(Custom_FVF), std::ios_base::beg);
		//this->fout.close();
	}

	this->fout.seekp(vertsDataStartingOffset + vertCount * sizeof(Custom_FVF) + faceCount * sizeof(int), std::ios_base::beg);


	//
	// Write down Material & Texture Definition (MTD)
	//
	int materialCount = (int)materialList.size();
	this->fout.write((char*)&materialCount, sizeof(int));
	for (i = 0; i < materialCount; i++)
	{
		// material name
		this->fout.write(materialList[i].strMatName.c_str(), (int)(materialList[i].strMatName.length() + 1));
		// struct D3DMATERIAL9
		this->fout.write((char*)&materialList[i].d3dMat, sizeof(D3DMATERIAL9));
		// texture file full path
		this->fout.write(materialList[i].strTexFileName.c_str(), (int)(materialList[i].strTexFileName.length() + 1));
	}


	//
	// - Animation Data Definition (ADD)
	//
	// is processed by following function;
	//
	this->ExportNDD_Anim(node);

	//
	// calculate chunk size and write it
	//
	DWORD chuckEndPos = this->fout.tellp();
	chunkSize = chuckEndPos - chuckStartPos;
	this->fout.seekp(chuckStartPos-sizeof(int), std::ios_base::beg);
	this->fout.write((char*)&chunkSize, sizeof(int));
	this->fout.seekp(chuckEndPos, std::ios_base::beg);

	return 0;
}


Point3 ModelExporter::TransformVector(GMatrix const& gm, Point3 const& p)
{
	float out[3];
	float const* v = &p.x;
	GRow const* m = gm.GetAddr();
	int i, j;
	for (i = 0; i < 3; i++)
	{
		float o = 0;
		for (j = 0; j < 3; j++)
		{
			o += m[j][i] * v[j];
		}
		out[i] = o;
	}
	return (Point3&)out;
}
Point3 ModelExporter::TransformVector4(GMatrix const& gm, Point4 const& p)
{
	float out[3];
	float const* v = &p.x;
	GRow const* m = gm.GetAddr();
	int i, j;
	for (i = 0; i < 3; i++)
	{
		float o = 0;
		for (j = 0; j < 4; j++)
		{
			o += m[j][i] * v[j];
		}
		out[i] = o;
	}
	return out;
}
int ModelExporter::Point3ToIntColor(Point3& p3)
{
	return (int)(255 * p3.x) + ((int)(255 * p3.y) << 8) + ((int)(255 * p3.z) << 16);
}

void ModelExporter::PrintChildMaterialInfo(IGameMaterial *igm, int depth)
{
	int i, j;
	if (igm->GetSubMaterialCount() == 0) return;
	for (i = 0; i < igm->GetSubMaterialCount(); i++)
	{
		for (j = 0; j < depth; j++)
			DebugPrint(_T("   "));

		DebugPrint(_T("ChildMaterial INDEX: %d, Name: %s, ID: %d\n"), i, igm->GetSubMaterial(i)->GetMaterialName(), igm->GetMaterialID(i));
		this->PrintChildMaterialInfo(igm->GetSubMaterial(i), depth + 1);
	}
	return;
}
IGameScene* ModelExporter::GetGame() const
{
	return this->game;
}

BoneHierarchy* ModelExporter::findBoneHierarchyByNode( IGameNode* node )
{
	std::vector<BoneHierarchy>::iterator it = this->boneHierarchy.begin();
	std::vector<BoneHierarchy>::iterator itEnd = this->boneHierarchy.end();
	for ( ; it != itEnd; ++it )
	{
		if ( _tcscmp( it->boneName, node->GetName() ) == 0 )
			return &*it;
	}
	return NULL;
}







//////////////////////////////////////////////////////////////////////////
// Global Functions
//////////////////////////////////////////////////////////////////////////

static float dot(Quat const & q, Quat const & p)
{
	return q.x*p.x + q.y*p.y + q.z*p.z + q.w*p.w;
}
static void GlobalMirror(GMatrix &m)
{
	((float *)&m)[14] *= -1.0f;
	((float *)&m)[2] *= -1.0f;
	((float *)&m)[6] *= -1.0f;
	((float *)&m)[8] *= -1.0f;
	((float *)&m)[9] *= -1.0f;
}

static bool almostEqual(Point3 const & a, Point3 const & b)
{
	return fabsf(a.x-b.x) < COMPARE_EPSILON && fabsf(a.y-b.y) < COMPARE_EPSILON && fabsf(a.z-b.z) < COMPARE_EPSILON;
}

static bool almostEqual(Quat const & a, Quat const & b)
{
	return fabsf(a.x-b.x) < COMPARE_EPSILON && fabsf(a.y-b.y) < COMPARE_EPSILON && fabsf(a.z-b.z) < COMPARE_EPSILON && fabsf(a.w-b.w) < COMPARE_EPSILON;
}



//////////////////////////////////////////////////////////////////////////
// Export Settings Dialog Procedure
//////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SettingsDialogProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static TCHAR buf[128];
	ModelExporter* pME = (ModelExporter*)lParam;

	

	switch ( msg )
	{
	case WM_INITDIALOG:
		/* default selected: NDT MESH2 */
		CheckDlgButton(hWndDlg, IDC_NDT_MESH2, 1);
		SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_MESH, "NDT_MESH2");
		g_exportVersion = EV_ARN20;
		g_ndtMesh = NDT_MESH2;

		/* default selected: NDT_ANIM1 */
		CheckDlgButton(hWndDlg, IDC_NDT_ANIM1, 1);
		SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_ANIM, "NDT_ANIM1");
		g_ndtAnim = NDT_ANIM1;

		g_animTicks = pME->GetGame()->GetSceneTicks();
		SetDlgItemInt(hWndDlg, IDC_STATIC_TICKS, g_animTicks, FALSE);

		g_animStartFrame = pME->GetGame()->GetSceneStartTime() / g_animTicks;
		//SetDlgItemText(hWndDlg, IDC_EDIT_START_FRAME, "0");
		SetDlgItemInt(hWndDlg, IDC_EDIT_START_FRAME, g_animStartFrame, FALSE);
		
		g_animEndFrame = pME->GetGame()->GetSceneEndTime() / g_animTicks;
		//SetDlgItemText(hWndDlg, IDC_EDIT_END_FRAME, "40");
		SetDlgItemInt(hWndDlg, IDC_EDIT_END_FRAME, g_animEndFrame, FALSE);

		return TRUE;
	case WM_CLOSE:
		EndDialog(hWndDlg, 1);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hWndDlg, 0); // exporting started
			break;
		case IDCANCEL:
			EndDialog(hWndDlg, 1); // exporting aborted
			break;
		case IDC_NDT_MESH1:
			g_exportVersion = EV_ARN10;
			g_ndtMesh = NDT_MESH1;
			SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_MESH, "NDT_MESH1");
			break;
		case IDC_NDT_MESH2:
			g_exportVersion = EV_ARN20;
			g_ndtMesh = NDT_MESH2;
			SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_MESH, "NDT_MESH2");
			break;
		case IDC_NDT_MESH3:
			g_exportVersion = EV_ARN20;
			g_ndtMesh = NDT_MESH3;
			SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_MESH, "NDT_MESH3");
			break;
		case IDC_NDT_ANIM1:
			g_ndtAnim = NDT_ANIM1;
			SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_ANIM, "NDT_ANIM1");
			break;
		case IDC_NDT_ANIM2:
			g_ndtAnim = NDT_ANIM2;
			SetDlgItemText(hWndDlg, IDC_SELECTED_NDT_ANIM, "NDT_ANIM2");
			break;
		case IDC_EDIT_START_FRAME:
			GetDlgItemText(hWndDlg, IDC_EDIT_START_FRAME, buf, 128);
			g_animStartFrame = (DWORD)_ttoi(buf);
			break;
		case IDC_EDIT_END_FRAME:
			GetDlgItemText(hWndDlg, IDC_EDIT_END_FRAME, buf, 128);
			g_animEndFrame = (DWORD)_ttoi(buf);
			break;

		}
	}

	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
// Export Settings Dialog Procedure
//////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK ProgressDialogProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static TCHAR buf[128];
	
	g_hProgressBarDialog = hWndDlg;
	g_hProgressBar = GetDlgItem( hWndDlg, IDC_PROGRESS_BAR );
	switch ( msg )
	{
	case WM_INITDIALOG:
		
		SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
		SendMessage(g_hProgressBar, PBM_SETSTEP, 1, 0);
		SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);
		SendMessage(g_hProgressBar, PBM_SETBARCOLOR, 0, 0x00afafaf);


		return TRUE;
	case WM_CLOSE:
		EndDialog( hWndDlg, 1 );
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			break;
		case IDCANCEL:
			break;
		}
		break;
	}

	return FALSE;
}