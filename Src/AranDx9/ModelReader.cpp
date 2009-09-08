// ModelReader.cpp
// 2007, 2008, 2009 Geoyeob Kim
//

#include "AranPCH.h"
#include "ModelReader.h"
#include "ArnTexture.h"
#include "Animation.h"
#include "ArnAnimationController.h"
#include "ArnIpo.h"
#include "ArnMath.h"
#include <string>

ModelReader* ModelReader::create(const char* fileName, const BOOL initAC)
{
    int i = 0;
	ModelReader* reader = new ModelReader(); // Returning instance
	std::string debugOutput(" - Initialize ARN File: ");
	if ( fileName && strlen( fileName ) )
		debugOutput += fileName;
	debugOutput += '\n';
	OutputDebugStringA(debugOutput.c_str());

	HRESULT hr;

	//reader->SetLoadingWindowHandle(hLoadingWnd);
	//reader->SetDev(lpDev);
	//reader->SetFVF(fvf);

	if ( strlen( fileName ) == 0 )
	{
		int size = reader->Read(fileName);
		//_tcscpy_s( reader->szFileName, TCHARSIZE(reader->szFileName), fileName );
		strcpy( reader->szFileName, fileName );
		std::string globalPathFileName(GLOBAL_ARN_FILE_PATH);
		globalPathFileName += fileName;
		if (size < 0)
		{
			size = reader->Read(globalPathFileName.c_str());
			//_tcscpy_s( reader->szFileName, TCHARSIZE(reader->szFileName), globalPathFileName.c_str() );
			strcpy( reader->szFileName, globalPathFileName.c_str() );
		}
		if (size < 0)
		{
			_LogWrite(_T("ModelReader::create() - file open error"), LOG_FAIL);
			return 0;
		}
	}
	else
	{
		int size = reader->Read(fileName);
		std::string globalPathFileName(GLOBAL_ARN_FILE_PATH);
		globalPathFileName += fileName;
		if (size < 0)
			size = reader->Read(globalPathFileName.c_str());
		if (size < 0)
		{
			_LogWrite(_T("ModelReader::create() - file open error"), LOG_FAIL);
			return 0;
		}

	}

	//// if lpAC is 0, this means the model use its own (local) AC
	//if (lpAC == 0)
	//{
	//	// TODO: optimized max counts in AC
	//	if (reader->skeletonNode.size() > 0)
	//	{
	//		V_OKAY( D3DXCreateAnimationController(
	//			50, /* MaxNumMatrices */
	//			10, /* MaxNumAnimationSets */
	//			2, /* MaxNumTracks */
	//			10, /* MaxNumEvents */
	//			&reader->lpAnimationController
	//			) );
	//	}
	//	else
	//	{
	//		V_OKAY( D3DXCreateAnimationController(
	//			reader->indTotalMeshCount + reader->notIndTotalMeshCount, /* MaxNumMatrices */
	//			1, /* MaxNumAnimationSets */
	//			2, /* MaxNumTracks */
	//			10, /* MaxNumEvents */
	//			&reader->lpAnimationController
	//			) );
	//	}
	//	reader->useLocalAC = TRUE;
	//}
	//else
	//{
	//	reader->lpAnimationController = lpAC;
	//	reader->useLocalAC = FALSE;
	//}

	//////////////////////////////////////////////////////////////////////////
	// General mesh animation loading (single animation set first)
	//////////////////////////////////////////////////////////////////////////
	//LPD3DXKEYFRAMEDANIMATIONSET lpKeyframedAnimSet = 0;
	ArnIpo* lpAnimSetTemp = 0;
	int ticksPerSecond = 1;

	int generalMeshCount = -1;
	if ( reader->exportVersion == EV_ARN10 || reader->exportVersion == EV_ARN11 )
	{
		generalMeshCount = reader->notIndTotalMeshCount;
	}
	else if ( reader->exportVersion == EV_ARN20 )
	{
		generalMeshCount = reader->indTotalMeshCount;
	}
	else
	{
		throw std::runtime_error("Unexpected ARN format");
	}

	hr = ArnCreateKeyframedAnimationSet("Default General Mesh Anim Set", ticksPerSecond, ARNPLAY_LOOP, generalMeshCount, 0, 0, &reader->lpKeyframedAnimationSet);
	if (FAILED(hr))
		goto e_NoGeneralMeshAnim;

	reader->animMatControlledByAC.resize(generalMeshCount);

	for (i = 0; i < generalMeshCount; i++)
	{
		DWORD animIndex = 0;
		//ASSERTCHECK(reader->animQuatSize[i] > 0);
		assert(reader->animQuatSize[i] > 0);
		UINT keysCount = (UINT)reader->animQuatSize[i];
		UINT scaleKeysCount = keysCount;
		UINT rotationKeysCount = keysCount;
		UINT translationKeysCount = keysCount;
		std::string keysName;
		if ( reader->exportVersion == EV_ARN10 || reader->exportVersion == EV_ARN11 )
		{
			keysName = reader->notIndMeshNames[i] + "-DefaultKeys";
		}
		else if ( reader->exportVersion == EV_ARN20 )
		{
			keysName = reader->indMeshNames[i] + "-DefaultKeys";
		}

		ARNKEY_VECTOR3* scaleKeys = 0; //new D3DXKEY_VECTOR3[keysCount];
		ARNKEY_QUATERNION* rotationKeys = 0; //new D3DXKEY_QUATERNION[keysCount];
		ARNKEY_VECTOR3* translationKeys = 0; //new D3DXKEY_VECTOR3[keysCount];

		reader->AllocateAsAnimationSetFormat(keysCount, reader->animQuat[i],
			&scaleKeysCount, &rotationKeysCount, &translationKeysCount,
			&scaleKeys, &rotationKeys, &translationKeys, TRUE);

		hr = reader->lpKeyframedAnimationSet->RegisterAnimationSRTKeys(keysName.c_str(),
			scaleKeysCount, rotationKeysCount, translationKeysCount,
			scaleKeys, rotationKeys, translationKeys,
			&animIndex);
		assert(hr == S_OK);

		SAFE_DELETE_ARRAY(scaleKeys);
		SAFE_DELETE_ARRAY(rotationKeys);
		SAFE_DELETE_ARRAY(translationKeys);

		reader->lpAnimationController->RegisterAnimationOutput(keysName.c_str(), &reader->animMatControlledByAC[i], 0, 0, 0);
	}

	reader->lpAnimationController->RegisterIpo(reader->lpKeyframedAnimationSet);
	reader->lpAnimationController->GetAnimationSet(0, &lpAnimSetTemp);
	reader->lpAnimationController->SetTrackAnimationSet(0, 0);
	reader->lpAnimationController->SetTrackEnable(0, TRUE);
	reader->lpAnimationController->SetTrackWeight(0, 1.0f);
	reader->lpAnimationController->SetTrackSpeed(0, 3.0f);
	reader->lpAnimationController->SetTrackPosition(0, 0.0f);
	reader->lpAnimationController->ResetTime();

e_NoGeneralMeshAnim:
	SAFE_RELEASE(lpAnimSetTemp);
	SAFE_RELEASE(reader->lpKeyframedAnimationSet);


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Skeletal animation loading
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//reader->lpAnimationController = lpAC;
	int animSetNum = reader->lpAnimationController->GetNumAnimationSets();

	if (initAC == TRUE)
	{
		// Unregister all animation sets (at editor)
		ArnIpo* lpAnimSetTemp2 = 0;
		for (i = 0; i < animSetNum; i++)
		{
			reader->lpAnimationController->GetAnimationSet(i, &lpAnimSetTemp2);
			if (lpAnimSetTemp2)
			{
				reader->lpAnimationController->UnregisterAnimationSet(lpAnimSetTemp2);
				SAFE_RELEASE(lpAnimSetTemp2);
			}
		}

		animSetNum = 0; // set anim set count to zero
	}


	int skeletonNodeSize = (int)reader->GetSkeletonNodeSize();
	for (i = 0; i < skeletonNodeSize; i++)
	{
		int meshIndex = reader->GetMeshIndexBySkeletonIndex(i);
		assert(meshIndex >= 0);
		//reader->BuildBlendedMeshByMeshIndex(meshIndex);
		//reader->BuildBoneHierarchyByMeshIndex(meshIndex);
		//reader->BuildKeyframedAnimationSetOfSkeletonNodeIndex( i );

	}

	if (skeletonNodeSize > 0)
	{
		reader->lpAnimationController->RegisterIpo( reader->GetKeyframedAnimationSet(0) );

		ArnIpo* lpAnimSetTemp2 = 0;

		// First Track ... (Loiter)
		reader->lpAnimationController->GetAnimationSet( animSetNum, &lpAnimSetTemp2 );
		reader->lpAnimationController->SetTrackAnimationSet( 0, 0 );
		reader->lpAnimationController->SetTrackWeight( 0, 1.0f );
		reader->lpAnimationController->SetTrackSpeed( 0, 3.5f );
		reader->lpAnimationController->SetTrackPosition( 0, 0.0f );
		reader->lpAnimationController->SetTrackEnable( 0, TRUE );

		SAFE_RELEASE( lpAnimSetTemp2 )

		MyFrame* frameRoot = 0;

		for (i = 0; i < (int)reader->GetSkeletonNodeSize(); i++)
		{
			int meshIndex = reader->GetMeshIndexBySkeletonIndex(i);
			assert(meshIndex >= 0);
			frameRoot = reader->GetFrameRootByMeshIndex(meshIndex);

			assert( frameRoot );

			ArnFrameRegisterNamedMatrices(frameRoot, reader->lpAnimationController);
		}


		frameRoot = 0;
	}

	reader->initialized = TRUE;

	return S_OK;
}

ModelReader::ModelReader(void)
{
	clearMembers();

	ZeroMemory(this->szFileName, sizeof(this->szFileName));

}

void ModelReader::clearMembers()
{
	totalFaceCount			= 0;
	notIndTotalMeshCount	= 0;
	indTotalMeshCount		= 0;
	skinnedMeshCount		= 0;
	notIndVertTotalSize		= 0;
	indVertTotalSize		= 0;
	lightCount				= 0;
	nodeCount				= 0;
	exportVersion			= EV_UNDEFINED;
	initialized				= FALSE;
	szFileName[0]			= _T('\0');

	nodeTypeCounter.clear();
	nodeTypeCounter[NDT_MESH1] = 0;
	nodeTypeCounter[NDT_MESH2] = 0;
	nodeTypeCounter[NDT_LIGHT1] = 0;
	nodeTypeCounter[NDT_CAMERA1] = 0;
	nodeTypeCounter[NDT_SKELETON1] = 0;
	nodeTypeCounter[NDT_BONE1] = 0;
	nodeTypeCounter[NDT_ANIM1] = 0;
}
void ModelReader::SetFileName( const char* fileName )
{
	//_tcscpy_s( this->szFileName, TCHARSIZE(this->szFileName), fileName );
	strcpy( this->szFileName, fileName );
}
ModelReader::~ModelReader(void)
{

	int i;


	for (i = 0; i < (int)this->materialReference.size(); i++)
	{
		SAFE_DELETE_ARRAY(materialReference[i]);
	}
	for (i = 0; i < (int)this->materialReferenceFast.size(); i++)
	{
		//if (this->materialCount[i] != 0)
		//{
			SAFE_DELETE_ARRAY(this->materialReferenceFast[i]);
		//}

	}

	for (i = 0; i < (int)this->animQuat.size(); i++)
	{
		SAFE_DELETE_ARRAY(this->animQuat[i]);
	}
	for (i = 0; i < (int)this->lightAnimQuat.size(); i++)
	{
		SAFE_DELETE_ARRAY(this->lightAnimQuat[i]);
	}


}





int ModelReader::BuildTopLevelNodeList()
{
	this->fin.clear();
	this->fin.open(this->szFileName, std::ios_base::binary);
	if (!this->fin.is_open())
	{
		throw std::runtime_error("Model loading failed; file not found");
	}

	char fileDescriptor[32]; // e.g. ARN10

	// File Reading start...
	this->fin.getline(fileDescriptor, sizeof(fileDescriptor), '\0');

	//bool isRawData = false; // ARN10(Raw Format), ARN11(Sorted Format), ARN20(Not Used)

	if (strcmp(fileDescriptor, "ARN10") == 0)
	{
		this->exportVersion = EV_ARN10;
		//isRawData = true;
	}
	else if (strcmp(fileDescriptor, "ARN11") == 0)
	{
		this->exportVersion = EV_ARN11;
		//isRawData = false;
	}
	else if (strcmp(fileDescriptor, "ARN20") == 0)
	{
		this->exportVersion = EV_ARN20;
		//isRawData = true;
	}
	else
	{
		this->exportVersion = EV_UNDEFINED;
		MessageBoxA(0, this->szFileName, "File Descriptor Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	this->fin.read((char*)&this->nodeCount, sizeof(int)); // read node(NDD) count
	if (this->nodeCount <= 0)
	{
		MessageBoxA(0, "Node count field error.", "Error", MB_OK | MB_ICONERROR);
		return -2;
	}

	int i;
	static char nodeName[128];
	for (i = 0; i < this->nodeCount; i++)
	{

		ArnNodeHeader anh;
		this->fin.read((char*)&anh.ndt, sizeof(int));
		this->fin.getline(nodeName, sizeof(nodeName), '\0');
		anh.uniqueName = nodeName;
		this->fin.read((char*)&anh.chunkSize, sizeof(int));
		anh.chunkStartPos = this->fin.tellg();

		this->nodeHeaders.push_back(anh);

		this->nodeTypeCounter[anh.ndt]++;

		this->fin.seekg(anh.chunkSize, std::ios_base::cur);
	}

	// check for the terminal string
	this->fin.getline(nodeName, sizeof(nodeName), '\0');
	ASSERTCHECK(strcmp(nodeName, "TERM") == 0);

	return 0;
}

int ModelReader::ParseNDD_Hierarchy(int nodeHeaderIndex)
{
	this->fin.read((char*)&this->hierarchySize, sizeof(int));
	//this->hierarchy.resize(this->hierarchySize);

	int j;
	for (j = 0; j < this->hierarchySize; j++)
	{
		MyFrame myFrame;

		this->fin.getline(myFrame.Name, 128, '\0');
		this->fin.read((char*)&myFrame.isRoot, sizeof(BOOL));
		this->fin.read((char*)&myFrame.sibling, sizeof(size_t));
		this->fin.read((char*)&myFrame.firstChild, sizeof(size_t));

		this->hierarchy.push_back(myFrame);
	}
	return S_OK;
}
int ModelReader::ParseNDD_Skeleton(int nodeHeaderIndex)
{
	//static char buf[1024];

	ArnNodeHeader* pCurNode = &this->nodeHeaders[nodeHeaderIndex];

	SkeletonNode skelNode;
	strcpy(skelNode.nameFixed, pCurNode->uniqueName.c_str());
	this->fin.getline(skelNode.associatedMeshName, sizeof(skelNode.associatedMeshName), '\0');
	this->fin.read((char*)&skelNode.maxWeightsPerVertex, sizeof(int));
	int bonesCount = -1;
	this->fin.read((char*)&bonesCount, sizeof(int));
	skelNode.bones.resize(bonesCount);
	skelNode.bonesCount = bonesCount;

	int j;
	for (j = 0; j < bonesCount; j++)
	{
		//Bone bone;
		NODE_DATA_TYPE boneNdt = NDT_UNKNOWN;
		int boneChunkSize = -1;
		this->fin.read((char*)&boneNdt, sizeof(int));
		assert(boneNdt == NDT_BONE1);
		char tempNameFixed[128];
		this->fin.getline(tempNameFixed, sizeof(tempNameFixed), '\0');
		skelNode.bones[j].nameFixed = tempNameFixed;
		this->fin.read((char*)&boneChunkSize, sizeof(int));
		this->fin.read((char*)skelNode.bones[j].offsetMatrix.m, 4*4*sizeof(float));
		this->fin.read((char*)&skelNode.bones[j].infVertexCount, sizeof(size_t));
		skelNode.bones[j].indices.resize(skelNode.bones[j].infVertexCount);
		skelNode.bones[j].weights.resize(skelNode.bones[j].infVertexCount);
		this->fin.read((char*)&skelNode.bones[j].indices[0], (int)skelNode.bones[j].infVertexCount * sizeof(DWORD));
		this->fin.read((char*)&skelNode.bones[j].weights[0], (int)skelNode.bones[j].infVertexCount * sizeof(float));


		//////////////////////////////////////////////////////////////////////////
		// OLD METHOD
		//////////////////////////////////////////////////////////////////////////
		/*this->fin.read(buf, 4);
		buf[4] = '\0';
		ASSERTCHECK(strcmp(buf, "ANIM") == 0);

		int rangeStart, rangeEnd;
		this->fin.read((char*)&rangeStart, sizeof(int));
		this->fin.read((char*)&rangeEnd, sizeof(int));
		int keyframeCount = rangeEnd - rangeStart + 1;
		skelNode.bones[j].keys.resize(keyframeCount);
		this->fin.read((char*)&skelNode.bones[j].keys[0], keyframeCount * sizeof(RST_DATA));*/

		this->ParseNDD_Anim(NDT_SKELETON1, &skelNode.bones[j]);

	}

	this->skeletonNode.push_back(skelNode);

	return S_OK;
}

int ModelReader::ParseNDD_Light(int nodeHeaderIndex)
{
	//this->lightCount++;
	ArnLightData light;
	this->fin.read((char*)&light, sizeof(light));
	this->lights.push_back(light);

	return S_OK;
}

int ModelReader::ParseNDD_Camera(int nodeHeaderIndex)
{
	ARN_NDD_CAMERA_CHUNK cam;

	this->fin.read( (char*)&cam, sizeof( cam ) );

	this->cameraNodes.push_back( cam );

	return S_OK;
}

int ModelReader::ParseNDD_Mesh1(int nodeHeaderIndex)
{
	static char buf[128];

	int curMeshVertCount = -1;
	HRESULT hr = 0;

	this->fin.read((char*)&curMeshVertCount, sizeof(int));

	// TODO: ambiguous usage
	this->meshVertCount.push_back(curMeshVertCount);


	// not indexed vertices (mesh)
	this->verticesOffset.push_back((int)this->fin.tellg()); // stack current mesh vertices data's offset
	this->verticesLength.push_back(curMeshVertCount * sizeof(ARN_VDD));

	this->notIndMeshNames.push_back(this->nodeHeaders[nodeHeaderIndex].uniqueName);
	this->notIndTotalMeshCount++;
	this->notIndVertTotalSize += curMeshVertCount * sizeof(ARN_VDD);


	// skip vertices data (will be processed afterwards)
	this->fin.seekg(curMeshVertCount * sizeof(ARN_VDD), std::ios_base::cur);


	//
	// Setup Face-Material Table (this will be sorted later)
	// (FMT data is equivalent to attribute buffer in D3D9 scheme)
	//
	int faceCount = -1;


	faceCount = curMeshVertCount / 3; // ARN10

	this->totalFaceCount += faceCount;

	this->fmtOffset.push_back((int)this->fin.tellg());
	this->fmtLength.push_back((int)(curMeshVertCount / 3));

	//////////////////////////////////////////////////////////////////////////
	// Setup Face-Material Table
	//////////////////////////////////////////////////////////////////////////
	std::vector<int> mref(faceCount);
	this->fin.read((char*)&mref[0], sizeof(int)*faceCount);

	this->materialReference.push_back(new int[faceCount]); // Allocate FMT (Face-Material Table)
	memcpy((char*)this->materialReference.back(), &mref[0], sizeof(int) * faceCount);


	//this->totalMeshCount++;

	//////////////////////////////////////////////////////////////////////////
	// Setup Material & Texture Definition
	//////////////////////////////////////////////////////////////////////////
	int curMaterialCount = -1;
	this->fin.read((char*)&curMaterialCount, sizeof(int));
	this->materialCount.push_back(curMaterialCount);
	ARN_MTD mtd; // Material & Texture Definition
	int i;
	for (i = 0; i < curMaterialCount; i++)
	{
		this->fin.getline(buf, sizeof(buf), '\0'); // read material name
		mtd.strMatName = buf;
		this->fin.read((char*)&mtd.d3dMat, sizeof(ArnMaterialData));
		this->fin.getline(buf, sizeof(buf), '\0');
		mtd.strTexFileName = buf;

		// - Material
		this->materialList.push_back(mtd);

		// - Texture
		if (mtd.strTexFileName.length() > 0)
		{
			//ArnTexture* lpTex = 0;

			//DWORD fileAttr = GetFileAttributesA( mtd.strTexFileName.c_str() );
			DWORD fileAttr = 0;
			if ( fileAttr != 0xffffffff )
			{

				/*

				// texture file exists
				if (this->lpDev)  // in case this model reader used in edit mode
				{
					V_OKAY(D3DXCreateTextureFromFileA(this->lpDev, mtd.strTexFileName.c_str(), &lpTex));
				}
				else
				{
					lpTex = 0;
				}

				this->textureList.push_back(lpTex);

				*/
			}
			else
			{
				/*


				//
				// first-chance file opening error
				//
				// check for the texture file sharing directory
				// cut the file name only and concatenate with global texture file path
				int getLastDirectoryDelimiter = (int)mtd.strTexFileName.find_last_of("\\") + 1;
				std::string fileNameOnly((char*)&mtd.strTexFileName.c_str()[getLastDirectoryDelimiter]);
				std::string textureFileSharingDirectory(GLOBAL_TEXTURE_FILE_PATH); // texture file sharing directory (Global)
				textureFileSharingDirectory.append(fileNameOnly);

				fileAttr = GetFileAttributesA( textureFileSharingDirectory.c_str() );
				if ( fileAttr != 0xffffffff )
				{
					if (this->lpDev) // in case this model reader used in edit mode
					{
						V_OKAY(D3DXCreateTextureFromFileA(this->lpDev, textureFileSharingDirectory.c_str(), &lpTex));
					}
					else
					{
						lpTex = 0;
					}
					this->textureList.push_back(lpTex);
				}
				else
				{
					// TODO: Error message verbosity setting
					//MessageBoxA(0, mtd.strTexFileName.c_str(), "Texture file does not exist; continuing happily :(", MB_OK | MB_ICONERROR);
					this->textureList.push_back(0);
				}

				*/
			}
		}
		else
		{
			/*

			// no texture
			this->textureList.push_back(0);

			*/
		}

	} // for (i = 0; i < curMaterialCount; i++)

	hr = S_OK;
	return hr;
}

int ModelReader::ParseNDD_Mesh2(int nodeHeaderIndex)
{
	HRESULT hr = 0;

	//
	//static char buf[128];

	//int curMeshVertCount = -1;

	//this->fin.read((char*)&curMeshVertCount, sizeof(int));

	//// TODO: ambiguous usage
	//this->meshVertCount.push_back(curMeshVertCount);

	//// skip vertices data (will be processed afterwards)
	//DWORD vertexPos = this->fin.tellg();
	//this->fin.seekg(curMeshVertCount * sizeof(ARN_VDD), std::ios_base::cur);

	//// read vertex index data
	//int numFaces = -1;
	//this->fin.read((char*)&numFaces, sizeof(int));
	//ASSERTCHECK(numFaces > 0);

	//int workingMeshIndex = this->indTotalMeshCount;

	//// initialize mesh interface
	//
	//ASSERTCHECK( this->lpDev );
	//hr = D3DXCreateMeshFVF(
	//	numFaces,
	//	curMeshVertCount,
	//	D3DXMESH_MANAGED,
	//	ARN_VDD::ARN_VDD_FVF,
	//	this->lpDev,
	//	&this->lpMeshes[workingMeshIndex]
	//);
	//if (FAILED(hr))
	//{
	//	return DXTRACE_ERR_MSGBOX(_T("Mesh Creation Failed"), hr);
	//}

	////////////////////////////////////////////////////////////////////////////
	//// Setup Index buffer
	////////////////////////////////////////////////////////////////////////////
	//WORD* ind = 0;
	//this->lpMeshes[workingMeshIndex]->LockIndexBuffer(0, (void**)&ind);
	//std::vector<int> ind32(numFaces*3);// = new int[numFaces*3];
	//this->fin.read((char*)&ind32[0], sizeof(int) * numFaces * 3);
	//int i;
	//for (i = 0; i < numFaces*3; i++)
	//{
	//	// check WORD type boundary
	//	if ((ind32[i] & 0xffff0000) != 0)
	//	{
	//		// overflow
	//		MessageBox(0, _T("Indices should be 16-bit range"), _T("Error"), MB_OK | MB_ICONERROR);
	//		return -200;
	//	}
	//	else
	//	{
	//		ind[i] = (WORD)ind32[i]; // truncation free value
	//	}
	//}

	//this->lpMeshes[workingMeshIndex]->UnlockIndexBuffer();
	//ind = 0;

	////////////////////////////////////////////////////////////////////////////
	//// Setup Vertex Buffer
	////////////////////////////////////////////////////////////////////////////
	//this->indVertTotalSize += curMeshVertCount;
	//ARN_VDD* vdd = 0;
	//this->lpMeshes[workingMeshIndex]->LockVertexBuffer(0, (void**)&vdd);

	//this->fin.seekg(vertexPos, std::ios_base::beg);
	//this->fin.read((char*)vdd, curMeshVertCount * sizeof(ARN_VDD));
	//this->fin.seekg(sizeof(int) + sizeof(int)*numFaces*3, std::ios_base::cur);


	//this->lpMeshes[workingMeshIndex]->UnlockVertexBuffer();
	//vdd = 0;


	////
	//// Setup Face-Material Table (this will be sorted later)
	//// (FMT data is equivalent to attribute buffer in D3D9 scheme)
	////
	////std::vector<int> fmtOffset;
	////std::vector<int> fmtLength; // equals faceCount(=vertCount / 3)

	//int faceCount = -1;
	//faceCount = numFaces; // ARN20

	//this->totalFaceCount += faceCount;

	//this->fmtOffset.push_back((int)this->fin.tellg());
	//this->fmtLength.push_back((int)(curMeshVertCount / 3));

	////////////////////////////////////////////////////////////////////////////
	//// Setup Face-Material Table
	////////////////////////////////////////////////////////////////////////////
	//std::vector<int> mref(faceCount);
	//this->fin.read((char*)&mref[0], sizeof(int)*faceCount);

	//this->materialReference.push_back(new int[faceCount]); // Allocate FMT (Face-Material Table)
	//memcpy((char*)this->materialReference.back(), &mref[0], sizeof(int) * faceCount);


	////////////////////////////////////////////////////////////////////////////
	//// Setup Attribute Buffer
	////////////////////////////////////////////////////////////////////////////
	//DWORD* attributeBuffer = 0;
	//this->lpMeshes[this->indTotalMeshCount]->LockAttributeBuffer(0, &attributeBuffer);

	//for (i = 0; i < numFaces; i++)
	//{
	//	attributeBuffer[i] = mref[i];
	//}
	//this->lpMeshes[this->indTotalMeshCount]->UnlockAttributeBuffer();
	//attributeBuffer = 0;

	//this->indMeshNames.push_back(this->nodeHeaders[nodeHeaderIndex].uniqueName);
	//this->indTotalMeshCount++;

	////////////////////////////////////////////////////////////////////////////
	//// Setup Material & Texture Definition
	////////////////////////////////////////////////////////////////////////////
	//int curMaterialCount = -1;
	//this->fin.read((char*)&curMaterialCount, sizeof(int));
	//this->materialCount.push_back(curMaterialCount);
	//ARN_MTD mtd; // Material & Texture Definition
	//for (i = 0; i < curMaterialCount; i++)
	//{
	//	this->fin.getline(buf, sizeof(buf), '\0'); // read material name
	//	mtd.strMatName = buf;
	//	this->fin.read((char*)&mtd.d3dMat, sizeof(D3DMATERIAL9));
	//	this->fin.getline(buf, sizeof(buf), '\0');
	//	mtd.strTexFileName = buf;

	//	// - Material
	//	this->materialList.push_back(mtd);

	//	// - Texture
	//	if ( mtd.strTexFileName.length() > 0 )
	//	{
	//		LPDIRECT3DTEXTURE9 lpTex;
	//		int oldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

	//		DWORD fileAttr = GetFileAttributesA( mtd.strTexFileName.c_str() );
	//
	//		if ( fileAttr != 0xffffffff )
	//		{
	//			// texture file exists
	//			V_OKAY( D3DXCreateTextureFromFileA( this->lpDev, mtd.strTexFileName.c_str(), &lpTex ) );
	//			this->textureList.push_back( lpTex );
	//		}
	//		else
	//		{
	//			//
	//			// first-chance file opening error
	//			//
	//			// check for the texture file sharing directory
	//			// cut the file name only and concatenate with global texture file path
	//			int getLastDirectoryDelimiter = (int)mtd.strTexFileName.find_last_of("\\") + 1;
	//			std::string fileNameOnly((char*)&mtd.strTexFileName.c_str()[getLastDirectoryDelimiter]);
	//			std::string textureFileSharingDirectory(GLOBAL_TEXTURE_FILE_PATH); // texture file sharing directory (Global)
	//			textureFileSharingDirectory.append(fileNameOnly);
	//
	//			fileAttr = GetFileAttributesA(textureFileSharingDirectory.c_str());
	//			//existenceCheck.open(textureFileSharingDirectory.c_str());
	//			if ( fileAttr != 0xffffffff /*existenceCheck.is_open()*/ )
	//			{
	//				V_OKAY(D3DXCreateTextureFromFileA(this->lpDev, textureFileSharingDirectory.c_str(), &lpTex));
	//				this->textureList.push_back(lpTex);
	//			}
	//			else
	//			{
	//				// TODO: Error message verbosity
	//				//MessageBoxA(0, mtd.strTexFileName.c_str(), "Texture file does not exist; continuing happily :(", MB_OK | MB_ICONERROR);
	//				this->textureList.push_back(0);
	//			}
	//		}

	//		SetErrorMode( oldErrorMode );
	//	}
	//	else
	//	{
	//		// no texture
	//		this->textureList.push_back(0);
	//	}

	//} // for (i = 0; i < curMaterialCount; i++)


	//////////////////////////////////////////////////////////////////////////
	// TODO: ambiguous comment;
	// mesh nodes, light nodes can be reach here
	//////////////////////////////////////////////////////////////////////////

	// read anim data(rotation-scaling-translation)
	// Check animation descriptor (NOT 0-TERMINATED!!!)

	/*this->fin.read(buf, 4);
	buf[4] = '\0';
	ASSERTCHECK(strcmp(buf, "ANIM") == 0);

	int startFrame = -1;
	int endFrame = -1;
	this->fin.read((char*)&startFrame, sizeof(int));
	this->fin.read((char*)&endFrame, sizeof(int));
	int frameCount = endFrame - startFrame + 1;

	this->animQuatSize.push_back(frameCount);
	this->animQuat.push_back(new RST_DATA[frameCount]);
	this->fin.read((char*)this->animQuat.back(), sizeof(RST_DATA) * frameCount);*/

	hr = S_OK;

	return hr;
}

int ModelReader::ParseNDD_Anim(NODE_DATA_TYPE belongsToType, Bone* pBone)
{
	HRESULT hr;

	//
	// read NDD (anim data) header
	//
	static char nodeName[128];
	ArnNodeHeader animANH;
	this->fin.read((char*)&animANH.ndt, sizeof(int));
	this->fin.getline(nodeName, sizeof(nodeName), '\0');
	animANH.uniqueName = nodeName;
	this->fin.read((char*)&animANH.chunkSize, sizeof(int));
	animANH.chunkStartPos = this->fin.tellg();

	if (animANH.ndt == NDT_ANIM1)
	{
		/*this->fin.read(buf, 4);
		buf[4] = '\0';
		ASSERTCHECK(strcmp(buf, "ANIM") == 0);

		int startFrame = -1;
		int endFrame = -1;
		this->fin.read((char*)&startFrame, sizeof(int));
		this->fin.read((char*)&endFrame, sizeof(int));
		int frameCount = endFrame - startFrame + 1;*/

		int frameCount = -1;
		this->fin.read((char*)&frameCount, sizeof(int));

		if (belongsToType == NDT_MESH1 || belongsToType == NDT_MESH2)
		{
			this->animQuatSize.push_back(frameCount);
			this->animQuat.push_back(new RST_DATA[frameCount]);
			this->fin.read((char*)this->animQuat.back(), sizeof(RST_DATA) * frameCount);
		}
		else if (belongsToType == NDT_SKELETON1)
		{
			pBone->keys.resize(frameCount);
			this->fin.read((char*)&pBone->keys[0], frameCount * sizeof(RST_DATA));
		}
		else
		{
			hr = E_FAIL;
			goto e_Exit;
		}
		hr = S_OK;
	}
	else
	{
		throw std::runtime_error("UNIMPLEMENTED ANIMATION FORMAT");
	}

e_Exit:
	return hr;
}

int ModelReader::Read(const char* fileName)
{
	strcpy(this->szFileName, fileName);

	//if (this->hLoadingWnd != 0)
	//{
	//	SendMessage(this->hLoadingWnd, WM_USER+1, 1, 2);
	//	//PostMessage(this->hLoadingWnd, WM_PAINT, 0, 0);
	//	HDC hdc = GetDC(this->hLoadingWnd);
	//	TextOut(hdc, 0, 0, fileName, (int)_tcslen(fileName));
	//	ReleaseDC(this->hLoadingWnd, hdc);
	//}

	//char nodeName[128];

	int i, j, k;

	// 1. File open
	// 2. Check for the version
	// 3. Scan all the nodes
	if (FAILED(this->BuildTopLevelNodeList()))
	{
		return E_FAIL;
	}

	if (this->exportVersion == EV_ARN20)
	{
		//int count = this->nodeTypeCounter[NDT_MESH2];
		//this->lpMeshes = new LPD3DXMESH[count]; // nodeCount == meshCount, which means such as lights, skeletons are excluded from this counting
		//ZeroMemory(this->lpMeshes, sizeof(LPD3DXMESH) * count);
	}
	else
	{
		//this->lpMeshes = 0;
	}

	//int notIndVertTotalSize = 0;

	this->notIndVertTotalSize = 0; // to allocate vertex buffer (not indexed) with right size
	this->indVertTotalSize = 0;

	this->totalFaceCount = 0;

	this->notIndTotalMeshCount = 0;
	this->indTotalMeshCount = 0;

	this->lightCount = 0;
	//NODE_DATA_TYPE ndt;

	HRESULT hr = E_FAIL;

	for (i = 0; i < (int)this->nodeHeaders.size(); i++)
	{
		// current meshe's vertices count;
		// this value will be positive when the node is in form of mesh format
		// otherwise(zero or negative), the node is ARN20 or non-mesh format
		//int curMeshVertCount = -1;

		ArnNodeHeader* pCurNode = &this->nodeHeaders[i];

		// move file pointer to the start of node chunk
		// this->ParseNDD_xxx() method will read the file assuming that file pointer is in the right place
		this->fin.seekg(pCurNode->chunkStartPos, std::ios_base::beg);

		switch (pCurNode->ndt)
		{
		case NDT_MESH1:
			this->ParseNDD_Mesh1(i);
			this->ParseNDD_Anim(NDT_MESH1, 0); // child NDD structure
			break;
		case NDT_MESH2:
			this->ParseNDD_Mesh2(i);
			this->ParseNDD_Anim(NDT_MESH2, 0); // child NDD structure
			break;
		case NDT_SKELETON1:
			this->ParseNDD_Skeleton(i);
			break;
		case NDT_HIERARCHY1:
			this->ParseNDD_Hierarchy(i);
			break;
		case NDT_LIGHT1:
			this->ParseNDD_Light(i);
			break;
		case NDT_CAMERA1:
			this->ParseNDD_Camera( i );
			break;
		default:
			MessageBoxA( 0, "Unsupported or not implemented node detected!", "Warning", MB_OK | MB_ICONEXCLAMATION );
			break;
		}

	} // node iteration complete!!


	//////////////////////////////////////////////////////////////////////////
	// Setup Vertex Buffer for not indexed meshes
	//////////////////////////////////////////////////////////////////////////
	//SAFE_RELEASE(this->lpVB);
	if (this->notIndVertTotalSize > 0 && this->lpDev != 0 /* in case 'edit' mode */)
	{
		//V_OKAY(this->lpDev->CreateVertexBuffer(this->notIndVertTotalSize, D3DUSAGE_WRITEONLY, this->fvf, D3DPOOL_MANAGED, &this->lpVB, 0));

		char* vertices = 0;
		int verticesPointerOffset = 0;
		//V_OKAY(this->lpVB->Lock(0, 0, (void**)&vertices, 0));

		this->fin.clear();
		for (i = 0; i < this->notIndTotalMeshCount; i++)
		{
			// read vertices data of each mesh's
			this->fin.seekg(this->verticesOffset[i], std::ios_base::beg);
			this->fin.read(vertices + verticesPointerOffset, this->verticesLength[i]);


			// sort FMT and vertices data by FMT(material reference, ascending)
			// may be time-consuming

			if (this->exportVersion == EV_ARN10)
			{
				for (j = 0; j < this->GetFaceCount(i) - 1; j++)
				{
					for (k = j+1; k < this->GetFaceCount(i); k++)
					{
						if (this->materialReference[i][j] > this->materialReference[i][k])
						{
							int tempRef = this->materialReference[i][j];
							this->materialReference[i][j] = this->materialReference[i][k];
							this->materialReference[i][k] = tempRef;

							ARN_VDD cf[3]; // three points; a face
							ARN_VDD* v = (ARN_VDD*)(vertices + verticesPointerOffset);
							memcpy((char*)cf, (char*)&v[j*3], sizeof(ARN_VDD) * 3);
							memcpy((char*)&v[j*3], (char*)&v[k*3], sizeof(ARN_VDD) * 3);
							memcpy((char*)&v[k*3], (char*)cf, sizeof(ARN_VDD) * 3);
						}
					}
				}
			}


			// find boundary of material reference
			if (this->exportVersion == EV_ARN10 || this->exportVersion == EV_ARN11)
			{
				int pIntAllocSize = this->materialCount[i];
				int* pIntAlloc = 0;
				if (pIntAllocSize > 0)
				{
					pIntAlloc = new int[pIntAllocSize];
					this->materialReferenceFast.push_back(pIntAlloc);
					this->materialReferenceFast[i][0] = 0;
					int matIndex = 1;

					for (j = 0; j < this->GetFaceCount(i)-1; j++)
					{

						if (this->materialReference[i][j] != this->materialReference[i][j+1])
						{
							this->materialReferenceFast[i][matIndex] = j+1;
							matIndex++;
						}
					}
				}
				else
				{
					pIntAlloc = 0;
					this->materialReferenceFast.push_back(pIntAlloc);
				}
				/*this->materialReferenceFast.push_back(pIntAlloc);
				this->materialReferenceFast[i][0] = 0;
				int matIndex = 1;

				for (j = 0; j < this->GetFaceCount(i)-1; j++)
				{

					if (this->materialReference[i][j] != this->materialReference[i][j+1])
					{
						this->materialReferenceFast[i][matIndex] = j+1;
						matIndex++;
					}
				}*/
			}

			verticesPointerOffset += this->verticesLength[i];
		}

		this->fin.close();


		// if ARN file is saved in raw format, rewrite the file with sorted one. (ARN10 --> ARN11)
		if (this->exportVersion == EV_ARN10)
		{
			std::fstream fout(fileName, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
			verticesPointerOffset = 0;

			fout.seekp(0, std::ios_base::beg);
			fout.write("ARN11", 5);

			for (i = 0; i < this->notIndTotalMeshCount; i++)
			{
				fout.seekp(this->verticesOffset[i], std::ios_base::beg);
				fout.write(vertices + verticesPointerOffset, this->verticesLength[i]);

				fout.seekp(this->fmtOffset[i], std::ios_base::beg);
				fout.write((char*)this->materialReference[i], this->fmtLength[i] * sizeof(int));

				verticesPointerOffset += this->verticesLength[i];
			}
			fout.close();
		}

		//this->lpVB->Unlock();
	}

	// return total mesh vertices count
	hr = this->notIndVertTotalSize + this->indVertTotalSize;

	if (this->fin.is_open())
	{
		this->fin.close();
		this->fin.clear();
	}

	return hr;
}
int ModelReader::GetVBLength() const
{
	return this->notIndVertTotalSize;
}
int ModelReader::GetMaterialReference(int meshIndex, int faceIndex) const
{
	if (this->materialCount[meshIndex] == 0) return -1;

	int i, acum = 0;
	for (i = 0; i < meshIndex; i++)
		acum += this->materialCount[i];
	return this->materialReference[meshIndex][faceIndex] + acum;
}
int ModelReader::GetMaterialReferenceFast(int meshIndex, int materialIndex) const
{
	while (this->materialReferenceFast[meshIndex] == 0)
	{
		if (meshIndex == 0)
			return 0;

		meshIndex--;
	}
	return this->materialReferenceFast[meshIndex][materialIndex];
}
int ModelReader::GetTotalFaceCount() const
{
	return this->totalFaceCount;
}
int ModelReader::GetIndMeshCount() const
{
	return this->indTotalMeshCount;
}
int ModelReader::GetNotIndMeshCount() const
{
	return this->notIndTotalMeshCount;
}
int ModelReader::GetFaceCount(int meshIndex) const
{
	return this->meshVertCount[meshIndex] / 3;
}
int ModelReader::GetMaterialCount(int meshIndex) const
{
	return this->materialCount[meshIndex];
}
int ModelReader::GetAnimQuatSize(int meshIndex) const
{
	return this->animQuatSize[meshIndex];
}
RST_DATA* ModelReader::GetAnimQuat(int meshIndex) const
{
	if (meshIndex >= (int)this->animQuat.size())
		return 0;
	else
		return this->animQuat[meshIndex];
}
int ModelReader::GetLightCount() const
{
	return (int)this->lights.size();
}
ArnLightData& ModelReader::GetLight(int i)
{
	return this->lights[i];
}
EXPORT_VERSION ModelReader::GetExportVersion() const
{
	return this->exportVersion;
}
MyFrame* ModelReader::GetFrameBySkeletonName(const char* skelName)
{
	size_t t;

	for (t = 0; t < this->hierarchy.size(); t++)
	{
		MyFrame* f = &this->hierarchy[t];
		if (strcmp(skelName, f->nameFixed) == 0)
		{
			return f;
		}
	}
	return 0;
}
MyFrame* ModelReader::GetFrameRootByMeshIndex(int meshIndex)
{
	size_t s, t;
	for (s = 0; s < this->skeletonNode.size(); s++)
	{
		if (strcmp(this->skeletonNode[s].associatedMeshName, this->indMeshNames[meshIndex].c_str()) == 0)
		{
			break;
		}
	}
	if (s >= this->skeletonNode.size())
		return 0;

	size_t skeletonIndex = s;

	for (s = 0; s < this->hierarchy.size(); s++)
	{
		MyFrame* f = &this->hierarchy[s];
		if (f->isRoot == FALSE)
			continue;


		// if root frame(bone) detected; compare bone names between frame and bone
search_again:
		for (t = 0; t < this->skeletonNode[skeletonIndex].bones.size(); t++)
		{
			if ((strcmp(f->nameFixed, this->skeletonNode[skeletonIndex].bones[t].nameFixed.c_str()) == 0))
			{
				return f;
			}
		}
		// 3ds Max 9 use 'Bip' as the root of biped skeleton, in spite of real skinning root is 'Pelvis'.
		// To sustain consistency, 'Bip' is assume to be a root bone in ARN's bone hierarchy, not 'Pelvis'.
		// However, Bip bone has no influences on any vertices, and root frame search will be failed.
		// (which is the purpose of this function)
		// Hopefully, since Pelvis is always the first child of Bip, we can easily find it by following code..
		if (f->firstChild != 0xffffffff)
		{
			f = &this->hierarchy[f->firstChild];
			goto search_again;
		}
	}

	return 0;
}
ArnMatrix* ModelReader::GetCombinedMatrixByBoneName( const char* boneName )
{
	size_t s;
	for (s = 0; s < this->hierarchy.size(); s++)
	{
		if (strcmp(this->hierarchy[s].nameFixed, boneName) == 0)
			return &this->hierarchy[s].combinedMatrix;
	}
	return 0;
}
const ArnMatrix* ModelReader::GetTransformationMatrixByBoneName( const char* boneName ) const
{
	size_t s;
	const MyFrame* mf = 0;
	for (s = 0; s < this->hierarchy.size(); s++)
	{
		if (strcmp(this->hierarchy[s].nameFixed, boneName) == 0)
		{
			mf = &this->hierarchy[s];
			return &mf->TransformationMatrix;

		}
	}
	return 0;
}
size_t ModelReader::GetSkeletonNodeSize() const
{
	return this->skeletonNode.size();
}
SkeletonNode* ModelReader::GetSkeletonNodePointer(int index)
{
	// TODO: constness should be remained
	return &(this->skeletonNode[index]);
}

int ModelReader::GetMeshIndexBySkeletonIndex(int skelIndex) const
{
	const SkeletonNode* sn = &this->skeletonNode[skelIndex];

	int i;


	ASSERTCHECK( this->indTotalMeshCount > 0 );
	for (i = 0; i < this->indTotalMeshCount; i++)
	{
		const std::string* meshName = &this->indMeshNames[i];
		if (strcmp(meshName->c_str(), sn->associatedMeshName) == 0)
		{
			return i;
		}
	}
	return -1;
}
int ModelReader::GetSkeletonIndexByMeshIndex(int meshIndex) const
{
	const std::string* meshName = &this->indMeshNames[meshIndex];
	int i;
	for (i = 0; i < (int)this->skeletonNode.size(); i++)
	{
		const SkeletonNode* sn = &this->skeletonNode[i];
		if (strcmp(meshName->c_str(), sn->associatedMeshName) == 0)
		{
			return i;
		}
	}
	return -1;
}




ArnNodeHeader ModelReader::GetArnNodeHeader(int idx)
{
	return this->nodeHeaders[idx];
}
size_t ModelReader::GetArnNodeHeadersSize()
{
	return this->nodeHeaders.size();
}
//HRESULT ModelReader::AdvanceTime(float timeDelta) const
//{
//	if (this->initialized)
//		return this->lpAnimationController->AdvanceTime(timeDelta, 0);
//	else
//		return E_FAIL;
//}

//const ArnMatrix* ModelReader::GetAnimMatControlledByAC(int meshIndex) const
//{
//	return (const ArnMatrix*)&this->animMatControlledByAC[meshIndex];
//}
const char* ModelReader::GetFileNameOnly()
{
	int i = 0;
	for (i = (int)strlen(szFileName)-1; i >= 0; i--)
	{
		if (szFileName[i] == '\\')
			break;
	}
	return (const char*)&szFileName[i+1];
}

ARN_NDD_CAMERA_CHUNK* ModelReader::GetFirstCamera()
{
	if (this->cameraNodes.size() > 0)
	{
		return &this->cameraNodes[0];
	}
	else
	{
		return 0;
	}
}

BOOL ModelReader::IsInitialized() const
{
	return this->initialized;
}

int ModelReader::AllocateAsAnimationSetFormat(
	UINT sourceArraySize, RST_DATA* sourceArray, UINT* pScaleSize, UINT* pRotationSize, UINT* pTranslationSize,
	ARNKEY_VECTOR3** ppScale, ARNKEY_QUATERNION** ppRotation, ARNKEY_VECTOR3** ppTranslation, BOOL removeDuplicates)
{
	ASSERTCHECK( sourceArraySize >= 1 );

	*ppScale = new ARNKEY_VECTOR3[sourceArraySize];
	*ppRotation = new ARNKEY_QUATERNION[sourceArraySize];
	*ppTranslation = new ARNKEY_VECTOR3[sourceArraySize];

	int i;
	const float timeIntervalInSeconds = 1.0f;
	RST_DATA* pRST = sourceArray;

	for ( i = 0; i < (int)sourceArraySize; i++ )
	{
		(*ppScale)[i].Time = i * timeIntervalInSeconds;
		(*ppScale)[i].Value.x = pRST->sx;
		(*ppScale)[i].Value.y = pRST->sy;
		(*ppScale)[i].Value.z = pRST->sz;

		(*ppRotation)[i].Time = i * timeIntervalInSeconds;
		(*ppRotation)[i].Value.x = pRST->x;
		(*ppRotation)[i].Value.y = pRST->y;
		(*ppRotation)[i].Value.z = pRST->z;
		(*ppRotation)[i].Value.w = pRST->w;

		(*ppTranslation)[i].Time = i * timeIntervalInSeconds;
		(*ppTranslation)[i].Value.x = pRST->tx;
		(*ppTranslation)[i].Value.y = pRST->ty;
		(*ppTranslation)[i].Value.z = pRST->tz;

		pRST++;
	}

	*pScaleSize = sourceArraySize;
	*pRotationSize = sourceArraySize;
	*pTranslationSize = sourceArraySize;

	if ( removeDuplicates )
	{
		int dupStartIndex = -1;

		for ( i = 1; i < (int)(*pScaleSize); i++ )
		{
			ArnVec3* pV3a = &( (*ppScale)[i-1].Value );
			ArnVec3* pV3b = &( (*ppScale)[i  ].Value );

			if ( almostEqualFloat3( pV3a, pV3b ) == TRUE )
			{
				if ( dupStartIndex == -1)
				{
					dupStartIndex = i;
				}
			}
			else
			{
				if ( dupStartIndex > 0 && i >= dupStartIndex )
				{
					memcpy( &((*ppScale)[dupStartIndex]), &((*ppScale)[i-1]), sizeof(ARNKEY_VECTOR3) * (*pScaleSize - i + 1) );
					memset( &((*ppScale)[*pScaleSize - (i - dupStartIndex) + 1]), 0xffffffff, sizeof(ARNKEY_VECTOR3) * (i - dupStartIndex - 1) );
					*pScaleSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}

		dupStartIndex = -1;

		for ( i = 1; i < (int)(*pTranslationSize); i++ )
		{
			ArnVec3* pV3a = &((*ppTranslation)[i-1].Value);
			ArnVec3* pV3b = &((*ppTranslation)[i  ].Value);

			if ( almostEqualFloat3( (float*)pV3a, (float*)pV3b ) == TRUE )
			{
				if ( dupStartIndex == -1)
				{
					dupStartIndex = i;
				}
			}
			else
			{
				if ( dupStartIndex > 0 && i >= dupStartIndex )
				{
					memcpy( &((*ppTranslation)[dupStartIndex]), &((*ppTranslation)[i-1]), sizeof(ARNKEY_VECTOR3) * (*pTranslationSize - i + 1) );
					memset( &((*ppTranslation)[*pTranslationSize - (i - dupStartIndex) + 1]), 0xff, sizeof(ARNKEY_VECTOR3) * (i - dupStartIndex - 1) );
					*pTranslationSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}

		dupStartIndex = -1;

		for ( i = 1; i < (int)(*pRotationSize); i++ )
		{
			ArnQuat* pV4a = &((*ppRotation)[i-1].Value);
			ArnQuat* pV4b = &((*ppRotation)[i  ].Value);

			if ( almostEqualFloat4( (float*)pV4a, (float*)pV4b ) == TRUE )
			{
				if ( dupStartIndex == -1)
				{
					dupStartIndex = i;
				}
			}
			else
			{
				if ( dupStartIndex > 0 && i >= dupStartIndex )
				{
					memcpy( &((*ppRotation)[dupStartIndex]), &((*ppRotation)[i-1]), sizeof(ARNKEY_QUATERNION) * (*pRotationSize - i + 1) );
					memset( &((*ppRotation)[*pRotationSize - (i - dupStartIndex) + 1]), 0xff, sizeof(ARNKEY_QUATERNION) * (i - dupStartIndex - 1) );
					*pRotationSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}
	}


	return 0;
}

const ArnMaterialData* ModelReader::GetMaterial( int referenceIndex ) const
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
