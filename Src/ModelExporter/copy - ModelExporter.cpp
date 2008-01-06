// ModelExporter.cpp
// 2007 Geoyeob Kim
//
#include "Main.h"
#include "ModelExporter.h"
#include "ErrorProcedure.h"
#include "NullRestoreObj.h"

#ifndef DEBUG_MSG
#define DEBUG_MSG(x) (;)
#endif

extern HINSTANCE hInstance;



ModelExporter::ModelExporter(void)
:isFirstTimeInit(FALSE), isExportSuccessful(FALSE)
{
}

ModelExporter::~ModelExporter(void)
{
}


int ModelExporter::ExtCount()
{
	return 1;
}
const TCHAR* ModelExporter::Ext(int n)
{
	return _T("ARN");
}

const TCHAR* ModelExporter::LongDesc()
{
	return _T("Aran Model Exporter (2007)");
}

const TCHAR* ModelExporter::ShortDesc() 
{
	return _T("Aran Model Exporter");
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
void ModelExporter::ShowAbout(HWND hWnd)
{
	//::DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
}
BOOL ModelExporter::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}
int	ModelExporter::DoExport(const TCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options)
{
	DebugPrint(_T("===================================================\n"));
	DebugPrint(_T("DoExport() Function Start!\n"));
	DebugPrint(_T("===================================================\n"));

	srand(time(NULL));

	if (this->isFirstTimeInit == FALSE)
	{
		::InitCustomControls(hInstance);
		INITCOMMONCONTROLSEX iccx;
		iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		iccx.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
		bool initControls = (::InitCommonControlsEx(&iccx) != 0);
		assert(initControls != false);
		this->isFirstTimeInit = TRUE;
	}

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
	cm->SetCoordSystem(IGameConversionManager::IGAME_D3D);

	// 애니메이션 기능은 빠져있으므로 현재 프레임으로 익스포팅한다고 가정
	this->coreFrame = this->coreInterface->GetTime();

	// 선택된 개체만 할 것인가 전부 할 것인가를 결정해야하는데,
	// 일단 전부 익스포트하는 것으로 한다.
	this->game->InitialiseIGame(false);

	try
	{
		theHold.Begin();

		// Do your job here!

		this->StartExporting();

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
	return 1;
}

int ModelExporter::StartExporting()
{
	IGameScene* ig = this->game;

	int nodeCount = ig->GetTopLevelNodeCount();
	int i;

	this->rootMaterialCount = this->game->GetRootMaterialCount();

	this->fout.open(this->exportName.c_str(), std::ios_base::binary);

	for (i = 0; i < ig->GetRootMaterialCount(); i++)
	{
		DebugPrint(_T("RootMaterial INDEX: %d, Name: %s\n"), i, ig->GetRootMaterial(i)->GetMaterialName());
		this->PrintChildMaterialInfo(ig->GetRootMaterial(i), 1);
	}

	for (i = 0; i < nodeCount; i++)
	{
		IGameNode* node = ig->GetTopLevelNode(i);
		this->ExportNode(node);
	}

	this->fout.close();

	return 0;
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

int ModelExporter::ExportNode(IGameNode* node)
{
	std::string nodeName = node->GetName();
	DebugPrint(_T("Exporting Node: %s\n"), nodeName.c_str());

	GMatrix maxToDx, otm, ptm;
	
	maxToDx.SetRow(0, Point4(1, 0, 0, 0));
	maxToDx.SetRow(1, Point4(0, 0, -1, 0));
	maxToDx.SetRow(2, Point4(0, 1, 0, 0));
	maxToDx.SetRow(3, Point4(0, 0, 0, 1));

	otm = node->GetWorldTM(this->coreFrame) * maxToDx;
	ptm.SetIdentity();
	if (node->GetNodeParent() != NULL)
	{
		ptm = node->GetNodeParent()->GetWorldTM(this->coreFrame) * maxToDx;
		otm = otm * ptm.Inverse();
	}
	//otm.SetRow(3, Point4(otm.Translation() * 1.0f / 1.0f, 1));
	

	IGameObject* obj = node->GetIGameObject();
	int nodeMaterialIndex = -1;

	if (obj)
	{
		if (obj->GetIGameType() == IGameObject::IGAME_SPLINE)
		{
			
		}
		else if (obj->IsEntitySupported())
		{
			IGameObject::ObjectTypes objectType = obj->GetIGameType();

			switch (objectType)
			{
			case IGameObject::IGAME_MESH:
				////////////////////////////////////////////////////////////////////
				//
				// Start of Node Definition Data (NDD) for ARN format
				//
				////////////////////////////////////////////////////////////////////
				nodeMaterialIndex = node->GetMaterialIndex();
				//
				// - Node name
				// - Vertex Data Definition (VDD)
				// - Free-Material Table (FMT)
				// - Material & Texture Definition (MTD)
				//
				// are processed by following function;
				//
				this->ExportMesh(node, obj);
				//
				// - Animation Data Definition (ADD)
				//
				// is processed by following function;
				//
				this->ExportAnimation(node, obj);
				break;
			case IGameObject::IGAME_CAMERA:
				//this->ExportCamera(node, obj);
				//this->ExportAnimation(node, obj);
				break;
			case IGameObject::IGAME_LIGHT:
				break;
			default:
				break;
			}
		}
	}

	int childCount = node->GetChildCount();
	int i;
	for (i = 0; i < childCount; i++)
	{
		DebugPrint(_T("Child node detected.\n"));
		this->ExportNode(node->GetNodeChild(i));
	}

	return 0;
}
int ModelExporter::ExportCamera(IGameNode* node, IGameObject* obj)
{
	IGameCamera* igc = static_cast<IGameCamera*>(obj);
	
	return 0;
}
int ModelExporter::ExportAnimation(IGameNode* node, IGameObject* obj)
{
	const DWORD tick = 4800;
	const int fps = 30;
	const int startFrame = 0;
	const int endFrame = 40;

	int i = 0;
	/*GMatrix maxToDx;
	maxToDx.SetRow(0, Point4(1, 0, 0, 0));
	maxToDx.SetRow(1, Point4(0, 0, -1, 0));
	maxToDx.SetRow(2, Point4(0, 1, 0, 0));
	maxToDx.SetRow(3, Point4(0, 0, 0, 1));*/


	//
	// Write down Animation Data Definition (ADD) for ARN format
	//
	// Rotation-Scaling-Translation Data
	this->fout.write("ANIM", 4);

	// Frame range
	this->fout.write((char*)&startFrame, sizeof(int));
	this->fout.write((char*)&endFrame, sizeof(int));

	
	for (i = startFrame; i <= endFrame; i++)
	{
		// get current frame's world transformation matrix
		GMatrix tm = node->GetWorldTM(tick * i / fps);

		// decompose it to RST matrix
		Matrix3 m3 = tm.ExtractMatrix3();
		Point3 translation;
		Quat rotation;
		Point3 scale;
		DecomposeMatrix(m3, translation, rotation, scale);

		DebugPrint(_T("Ticks: %d / R: %.4f, %.4f, %.4f, %.4f / S: %.4f, %.4f, %.4f / T: %.4f %.4f %.4f\n"),
			tick*i/fps,
			rotation.x, rotation.y, rotation.z, rotation.w,
			scale.x, scale.y, scale.z,
			translation.x, translation.y, translation.z);

		//
		// CAUTION: AXIS SWAPPING occurs instead of matrix calculation
		//
		this->fout.write((char*)&rotation.x, sizeof(float));
		this->fout.write((char*)&rotation.z, sizeof(float));
		this->fout.write((char*)&rotation.y, sizeof(float));
		this->fout.write((char*)&rotation.w, sizeof(float));

		this->fout.write((char*)&scale.x, sizeof(float));
		this->fout.write((char*)&scale.z, sizeof(float));
		this->fout.write((char*)&scale.y, sizeof(float));

		this->fout.write((char*)&translation.x, sizeof(float));
		this->fout.write((char*)&translation.z, sizeof(float));
		this->fout.write((char*)&translation.y, sizeof(float));

	}

	return 0;
}

int ModelExporter::ExportMesh(IGameNode *node, IGameObject *obj)
{
	IGameMesh* igm = static_cast<IGameMesh*>(obj);
	IGameSkin* igs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : 0; // unused for now
	static char buf[4096];
	
	


	igm->SetCreateOptimizedNormalList();
	if (!igm->InitializeData())
	{
		_snprintf_s(buf, 4095, 4095, "Could not read mesh data from '%s'\n", node->GetName());
		buf[4095] = '\0';
		DebugPrint(_T(buf));
	}

	DWORD faceCount = igm->GetNumberOfFaces();
	DWORD vertCount = faceCount * 3;

	// write mesh(node) name(Null terminated variable length string)
	this->fout.write(node->GetName(), (int)(strlen(node->GetName()) + 1));
	// write total vertices count(int)
	this->fout.write((char*)&vertCount, sizeof(vertCount));
	DebugPrint(_T("Total Vertices: %d\n"), vertCount);
	int vertsDataStartingOffset = this->fout.tellp(); // 버텍스 정보가 시작되는 오프셋 저장

	int i, j, k;
	Custom_FVF fvf;

	GMatrix maxToDx;

	maxToDx.SetRow(0, Point4(1, 0, 0, 0));
	maxToDx.SetRow(1, Point4(0, 0, -1, 0));
	maxToDx.SetRow(2, Point4(0, 1, 0, 0));
	maxToDx.SetRow(3, Point4(0, 0, 0, 1));

	std::vector<Custom_Material> materialList;

	
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

			Custom_Material cm;
			cm.texFileName = _T("");

			if (igmat->GetNumberOfTextureMaps() != 0)
			{
				
				IGameTextureMap* igtm = igmat->GetIGameTextureMap(0);
				TCHAR* textureFileName = igtm->GetBitmapFileName();
				DebugPrint(_T(", %s)\n"), textureFileName);
				cm.texFileName = textureFileName;
				igtm = NULL;
			}
			else
			{
				DebugPrint(_T(")\n"));
			}

			// insert to material list
			bool isFirstMaterial = true;
			for (k = 0; k < materialList.size(); k++)
			{
				
				if (materialList[k].matName.compare(matName) == 0)
				{
					isFirstMaterial = false;
					break;
				}
			}
			if (isFirstMaterial == true)
			{
				
				Point3 p3;
				cm.matName = matName;
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


			/*IGameProperty* igprop = igmat->GetDiffuseData();
			float dummyFloat; int dummyInt; Point3 dummyPoint3; Point4 dummyPoint4; TCHAR* dummyTchar;
			switch (igprop->GetType())
			{
			case IGAME_FLOAT_PROP:
				igprop->GetPropertyValue(dummyFloat);
				break;
			case IGAME_INT_PROP:
				igprop->GetPropertyValue(dummyInt);
				break;
			case IGAME_POINT3_PROP:
				igprop->GetPropertyValue(dummyPoint3);
				break;
			case IGAME_POINT4_PROP:
				igprop->GetPropertyValue(dummyPoint4);
				break;
			case IGAME_STRING_PROP:
				igprop->GetPropertyValue(dummyTchar);
				break;
			case IGAME_UNKNOWN_PROP:
				break;
			}*/
		}
		else
		{
			DebugPrint(_T("\n"));
		}
		
		// three vertices per face
		for (j = 0; j < 3; j++)
		{
			fvf.vertex = igm->GetVertex(face->vert[j], false) * maxToDx;
			fvf.normal = ModelExporter::TransformVector(maxToDx, igm->GetNormal(face->norm[j], false));
			
			Point2 texCoord = igm->GetTexVertex(face->texCoord[j]);
			
			//fvf.vertex = igm->GetVertex(face->vert[j], false);
			//fvf.normal = igm->GetNormal(face->norm[j], false);
			
			//fvf.vertex.z = -fvf.vertex.z;
			//fvf.normal.z = -fvf.normal.z;
			
			//Point3 color = igm->GetColorVertex(face->vert[j]);
			//fvf.color = ModelExporter::Point3ToIntColor(color);
			fvf.color = 0xffffffff;
			
			fvf.u = texCoord.x;
			fvf.v = texCoord.y;
			//fvf.u = 0.0f;
			//fvf.v = 0.0f;

			//DebugPrint("v %5.2f, %5.2f, %5.2f n %5.2f %5.2f %5.2f\n", fvf.vertex.x, fvf.vertex.y, fvf.vertex.z, fvf.normal.x, fvf.normal.y, fvf.normal.z);
			
			this->fout.write((const char*)&fvf, sizeof(Custom_FVF));
		}

		// write material reference
		this->fout.seekp(vertsDataStartingOffset + vertCount * sizeof(Custom_FVF) + i * sizeof(int), std::ios_base::beg);
		bool isRightMaterialName = false;
		
		for (k = 0; k < materialList.size(); k++)
		{
			if (igmat == NULL) break;

			if (materialList[k].matName.compare(igmat->GetMaterialName()) == 0)
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
	

	int materialCount = (int)materialList.size();
	this->fout.write((char*)&materialCount, sizeof(int));
	for (i = 0; i < materialCount; i++)
	{
		this->fout.write(materialList[i].matName.c_str(), (int)(materialList[i].matName.length() + 1));
		this->fout.write((char*)&materialList[i].d3dMat, sizeof(D3DMATERIAL9));
		this->fout.write(materialList[i].texFileName.c_str(), (int)(materialList[i].texFileName.length() + 1));
	}


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
int ModelExporter::Point3ToIntColor(Point3& p3)
{
	return (int)(255 * p3.x) + ((int)(255 * p3.y) << 8) + ((int)(255 * p3.z) << 16);
}

