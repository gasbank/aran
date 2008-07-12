// ModelReader.cpp
// 2007 Geoyeob Kim
//

#include "stdafx.h"

#include "ModelReader.h"

#define COMPARE_EPSILON 1e-4
static inline BOOL almostEqualFloat3(D3DXVECTOR3* pV1, D3DXVECTOR3* pV2);
static inline BOOL almostEqualFloat3(float* floatArray1, float* floatArray2);
static inline BOOL almostEqualFloat4(float* floatArray1, float* floatArray2);



ModelReader::ModelReader(void)
{
	clearMembers();

	ZeroMemory(this->szFileName, sizeof(this->szFileName));

	// for skinned mesh feature
	ZeroMemory(&this->meshContainer, sizeof(meshContainer));
	ZeroMemory(&this->frame, sizeof(frame));
}

ModelReader::ModelReader(LPDIRECT3DDEVICE9 lpDev, DWORD fvf)
{
	clearMembers();

	this->lpDev = lpDev;
	this->fvf = fvf;

	ZeroMemory(this->szFileName, sizeof(this->szFileName));

	// for skinned mesh feature
	ZeroMemory(&this->meshContainer, sizeof(meshContainer));
	ZeroMemory(&this->frame, sizeof(frame));
}

void ModelReader::clearMembers()
{
	lpVB					= 0;
	fvf						= 0;
	totalFaceCount			= 0;
	hLoadingWnd				= 0;
	lpMeshes				= 0;
	notIndTotalMeshCount	= 0;
	indTotalMeshCount		= 0;
	lpKeyframedAnimationSet	= 0;
	lpDev					= 0;
	lpAnimationController	= 0;
	skinnedMeshCount		= 0;
	hierarchySize			= 0;
	notIndVertTotalSize		= 0;
	indVertTotalSize		= 0;
	lightCount				= 0;
	nodeCount				= 0;
	exportVersion			= EV_UNDEFINED;
	useLocalAC				= FALSE;
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
void ModelReader::SetFileName( const TCHAR* fileName )
{
	_tcscpy_s( this->szFileName, TCHARSIZE(this->szFileName), fileName );
}
ModelReader::~ModelReader(void)
{
	SAFE_RELEASE(this->lpVB);

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
	for (i = 0; i < (int)this->textureList.size(); i++)
	{
		SAFE_RELEASE(this->textureList[i]);
	}
	if (this->lpMeshes != 0)
	{
		for (i = 0; i < this->indTotalMeshCount; i++)
		{
			SAFE_RELEASE(this->lpMeshes[i]);
		}
		delete [] this->lpMeshes;
		this->lpMeshes = 0;
	}

	for (i = 0; i < (int)this->lpSkinnedMeshes.size(); i++)
	{
		SAFE_RELEASE(this->lpSkinnedMeshes[i]);
	}
	this->lpSkinnedMeshes.resize(0);

	for (i = 0; i < (int)this->lpSkinnedMeshesSkinInfo.size(); i++)
	{
		SAFE_RELEASE(this->lpSkinnedMeshesSkinInfo[i]);
	}
	this->lpSkinnedMeshesSkinInfo.resize(0);


	// Deallocate animation sets
	SAFE_RELEASE(this->lpKeyframedAnimationSet);
	for (i = 0; i < (int)this->lpKeyframedAnimationSetList.size(); i++)
	{
		SAFE_RELEASE(this->lpKeyframedAnimationSetList[i]);
	}

	if (this->useLocalAC)
	{
		SAFE_RELEASE(this->lpAnimationController);
	}

}



LPDIRECT3DVERTEXBUFFER9 ModelReader::GetVB() const
{
	return this->lpVB;
}

void ModelReader::SetDev(LPDIRECT3DDEVICE9 lpDev)
{
	this->lpDev = lpDev;
}
void ModelReader::SetFVF(DWORD fvf)
{
	this->fvf = fvf;
}
DWORD ModelReader::GetFVF() const
{
	return this->fvf;
}
void ModelReader::SetLoadingWindowHandle(HWND hLoadingWnd)
{
	this->hLoadingWnd = hLoadingWnd;
}

int ModelReader::BuildTopLevelNodeList()
{
	this->fin.clear();
	this->fin.open(this->szFileName, std::ios_base::binary);
	if (!this->fin.is_open())
	{
		DXTRACE_MSG(_T("Model loading failed; file not found"));
		return -1;
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
		MessageBox(0, this->szFileName, _T("File Descriptor Error"), MB_OK | MB_ICONERROR);
		return -1;
	}

	this->fin.read((char*)&this->nodeCount, sizeof(int)); // read node(NDD) count
	if (this->nodeCount <= 0)
	{
		MessageBox(0, _T("Node count field error."), _T("Error"), MB_OK | MB_ICONERROR);
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
	static char buf[1024];

	ArnNodeHeader* pCurNode = &this->nodeHeaders[nodeHeaderIndex];

	SkeletonNode skelNode;
	strcpy_s(skelNode.nameFixed, sizeof(skelNode.associatedMeshName), pCurNode->uniqueName.c_str());
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
	D3DLIGHT9 light;
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
	DWORD vertexPos = this->fin.tellg();
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
		this->fin.read((char*)&mtd.d3dMat, sizeof(D3DMATERIAL9));
		this->fin.getline(buf, sizeof(buf), '\0');
		mtd.strTexFileName = buf;

		// - Material
		this->materialList.push_back(mtd);

		// - Texture
		if (mtd.strTexFileName.length() > 0)
		{
			LPDIRECT3DTEXTURE9 lpTex;
			int oldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

			DWORD fileAttr = GetFileAttributesA( mtd.strTexFileName.c_str() );
			if ( fileAttr != 0xffffffff )
			{
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
			}
			else
			{
				//
				// first-chance file opening error
				//
				// check for the texture file sharing directory
				// cut the file name only and concatenate with global texture file path
				int getLastDirectoryDelimiter = (int)mtd.strTexFileName.find_last_of("\\") + 1;
				STRING fileNameOnly((char*)&mtd.strTexFileName.c_str()[getLastDirectoryDelimiter]);
				STRING textureFileSharingDirectory(GLOBAL_TEXTURE_FILE_PATH); // texture file sharing directory (Global)
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
			}
		}
		else
		{
			// no texture
			this->textureList.push_back(0);
		}

	} // for (i = 0; i < curMaterialCount; i++)

	hr = S_OK;
	return hr;
}

int ModelReader::ParseNDD_Mesh2(int nodeHeaderIndex)
{
	static char buf[128];

	int curMeshVertCount = -1;

	this->fin.read((char*)&curMeshVertCount, sizeof(int));

	// TODO: ambiguous usage
	this->meshVertCount.push_back(curMeshVertCount);

	// skip vertices data (will be processed afterwards)
	DWORD vertexPos = this->fin.tellg();
	this->fin.seekg(curMeshVertCount * sizeof(ARN_VDD), std::ios_base::cur);

	// read vertex index data
	int numFaces = -1;
	this->fin.read((char*)&numFaces, sizeof(int));
	ASSERTCHECK(numFaces > 0);

	int workingMeshIndex = this->indTotalMeshCount;

	// initialize mesh interface
	HRESULT hr = 0;
	ASSERTCHECK( this->lpDev );
	hr = D3DXCreateMeshFVF(
		numFaces,
		curMeshVertCount,
		D3DXMESH_MANAGED,
		ARN_VDD::ARN_VDD_FVF,
		this->lpDev,
		&this->lpMeshes[workingMeshIndex]
	);
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Mesh Creation Failed"), hr);
	}

	//////////////////////////////////////////////////////////////////////////
	// Setup Index buffer
	//////////////////////////////////////////////////////////////////////////
	WORD* ind = 0;
	this->lpMeshes[workingMeshIndex]->LockIndexBuffer(0, (void**)&ind);
	std::vector<int> ind32(numFaces*3);// = new int[numFaces*3];
	this->fin.read((char*)&ind32[0], sizeof(int) * numFaces * 3);
	int i;
	for (i = 0; i < numFaces*3; i++)
	{
		// check WORD type boundary
		if ((ind32[i] & 0xffff0000) != 0)
		{
			// overflow
			MessageBox(0, _T("Indices should be 16-bit range"), _T("Error"), MB_OK | MB_ICONERROR);
			return -200;
		}
		else
		{
			ind[i] = (WORD)ind32[i]; // truncation free value
		}
	}

	this->lpMeshes[workingMeshIndex]->UnlockIndexBuffer();
	ind = 0;

	//////////////////////////////////////////////////////////////////////////
	// Setup Vertex Buffer
	//////////////////////////////////////////////////////////////////////////
	this->indVertTotalSize += curMeshVertCount;
	ARN_VDD* vdd = 0;
	this->lpMeshes[workingMeshIndex]->LockVertexBuffer(0, (void**)&vdd);

	this->fin.seekg(vertexPos, std::ios_base::beg);
	this->fin.read((char*)vdd, curMeshVertCount * sizeof(ARN_VDD));
	this->fin.seekg(sizeof(int) + sizeof(int)*numFaces*3, std::ios_base::cur);


	this->lpMeshes[workingMeshIndex]->UnlockVertexBuffer();
	vdd = 0;


	//
	// Setup Face-Material Table (this will be sorted later)
	// (FMT data is equivalent to attribute buffer in D3D9 scheme)
	//
	//std::vector<int> fmtOffset;
	//std::vector<int> fmtLength; // equals faceCount(=vertCount / 3)

	int faceCount = -1;
	faceCount = numFaces; // ARN20

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


	//////////////////////////////////////////////////////////////////////////
	// Setup Attribute Buffer
	//////////////////////////////////////////////////////////////////////////
	DWORD* attributeBuffer = 0;
	this->lpMeshes[this->indTotalMeshCount]->LockAttributeBuffer(0, &attributeBuffer);

	for (i = 0; i < numFaces; i++)
	{
		attributeBuffer[i] = mref[i];
	}
	this->lpMeshes[this->indTotalMeshCount]->UnlockAttributeBuffer();
	attributeBuffer = 0;

	this->indMeshNames.push_back(this->nodeHeaders[nodeHeaderIndex].uniqueName);
	this->indTotalMeshCount++;

	//////////////////////////////////////////////////////////////////////////
	// Setup Material & Texture Definition
	//////////////////////////////////////////////////////////////////////////
	int curMaterialCount = -1;
	this->fin.read((char*)&curMaterialCount, sizeof(int));
	this->materialCount.push_back(curMaterialCount);
	ARN_MTD mtd; // Material & Texture Definition
	for (i = 0; i < curMaterialCount; i++)
	{
		this->fin.getline(buf, sizeof(buf), '\0'); // read material name
		mtd.strMatName = buf;
		this->fin.read((char*)&mtd.d3dMat, sizeof(D3DMATERIAL9));
		this->fin.getline(buf, sizeof(buf), '\0');
		mtd.strTexFileName = buf;

		// - Material
		this->materialList.push_back(mtd);

		// - Texture
		if ( mtd.strTexFileName.length() > 0 )
		{
			LPDIRECT3DTEXTURE9 lpTex;
			int oldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

			DWORD fileAttr = GetFileAttributesA( mtd.strTexFileName.c_str() );
			
			if ( fileAttr != 0xffffffff )
			{
				// texture file exists
				V_OKAY( D3DXCreateTextureFromFileA( this->lpDev, mtd.strTexFileName.c_str(), &lpTex ) );
				this->textureList.push_back( lpTex );
			}
			else
			{
				//
				// first-chance file opening error
				//
				// check for the texture file sharing directory
				// cut the file name only and concatenate with global texture file path
				int getLastDirectoryDelimiter = (int)mtd.strTexFileName.find_last_of("\\") + 1;
				STRING fileNameOnly((char*)&mtd.strTexFileName.c_str()[getLastDirectoryDelimiter]);
				STRING textureFileSharingDirectory(GLOBAL_TEXTURE_FILE_PATH); // texture file sharing directory (Global)
				textureFileSharingDirectory.append(fileNameOnly);
				
				fileAttr = GetFileAttributesA(textureFileSharingDirectory.c_str());
				//existenceCheck.open(textureFileSharingDirectory.c_str());
				if ( fileAttr != 0xffffffff /*existenceCheck.is_open()*/ )
				{
					V_OKAY(D3DXCreateTextureFromFileA(this->lpDev, textureFileSharingDirectory.c_str(), &lpTex));
					this->textureList.push_back(lpTex);
				}
				else
				{
					// TODO: Error message verbosity
					//MessageBoxA(0, mtd.strTexFileName.c_str(), "Texture file does not exist; continuing happily :(", MB_OK | MB_ICONERROR);
					this->textureList.push_back(0);
				}
			}

			SetErrorMode( oldErrorMode );
		}
		else
		{
			// no texture
			this->textureList.push_back(0);
		}

	} // for (i = 0; i < curMaterialCount; i++)

	
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

HRESULT ModelReader::ParseNDD_Anim(NODE_DATA_TYPE belongsToType, Bone* pBone)
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
		ASSERTCHECK("UNIMPLEMENTED ANIMATION FORMAT");
		hr = E_FAIL;
	}

e_Exit:
	return hr;
}

int ModelReader::Read(const TCHAR *fileName)
{
	_tcscpy_s(this->szFileName, fileName);

	if (this->hLoadingWnd != 0)
	{
		SendMessage(this->hLoadingWnd, WM_USER+1, 1, 2);
		//PostMessage(this->hLoadingWnd, WM_PAINT, 0, 0);
		HDC hdc = GetDC(this->hLoadingWnd);
		TextOut(hdc, 0, 0, fileName, (int)_tcslen(fileName));
		ReleaseDC(this->hLoadingWnd, hdc);
	}

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
		int count = this->nodeTypeCounter[NDT_MESH2];
		this->lpMeshes = new LPD3DXMESH[count]; // nodeCount == meshCount, which means such as lights, skeletons are excluded from this counting
		ZeroMemory(this->lpMeshes, sizeof(LPD3DXMESH) * count);
	}
	else
	{
		this->lpMeshes = 0;
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
	SAFE_RELEASE(this->lpVB);
	if (this->notIndVertTotalSize > 0 && this->lpDev != 0 /* in case 'edit' mode */)
	{
		V_OKAY(this->lpDev->CreateVertexBuffer(this->notIndVertTotalSize, D3DUSAGE_WRITEONLY, this->fvf, D3DPOOL_MANAGED, &this->lpVB, 0));
		
		char* vertices = 0;
		int verticesPointerOffset = 0;
		V_OKAY(this->lpVB->Lock(0, 0, (void**)&vertices, 0));

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

		this->lpVB->Unlock();
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



int ModelReader::BuildBoneHierarchyByMeshIndex(int meshIndex)
{
	size_t s;
	for (s = 0; s < this->hierarchy.size(); s++)
	{
		MyFrame* currentBone = &this->hierarchy[s];

		size_t siblingIndex = currentBone->sibling;
		if (siblingIndex == -1)
		{
			currentBone->pFrameSibling = 0;
		}
		else
		{
			currentBone->pFrameSibling = &this->hierarchy[siblingIndex];
		}

		size_t firstChildIndex = currentBone->firstChild;
		if (firstChildIndex == -1)
		{
			currentBone->pFrameFirstChild = 0;
		}
		else
		{
			currentBone->pFrameFirstChild = &this->hierarchy[firstChildIndex];
		}
		currentBone->pMeshContainer = 0;
		currentBone->Name = currentBone->nameFixed;
	}
	return 0;
}
int ModelReader::BuildBlendedMeshByMeshIndex(int meshIndex)
{
	// should Read() method called in precedence
	if (this->exportVersion != EV_ARN20)
	{
		return -1;
	}
	int i;
	for (i = 0; i < (int)this->skeletonNode.size(); i++)
	{
		if (strcmp(this->skeletonNode[i].associatedMeshName, this->indMeshNames[meshIndex].c_str()) == 0)
		{
			break;
		}
	}
	SkeletonNode* currentSkeleton = &this->skeletonNode[i];
	
	LPD3DXSKININFO lpSkinInfo = 0;
	int vertCount = this->lpMeshes[meshIndex]->GetNumVertices();
	size_t boneCount = currentSkeleton->bones.size();


	if (FAILED(D3DXCreateSkinInfoFVF(vertCount, MY_CUSTOM_MESH_VERTEX::MY_CUSTOM_MESH_VERTEX_FVF, (int)boneCount, &lpSkinInfo)))
	{
		MessageBox(0, _T("D3DXCreateSkinInfoFVF() failed."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}



	// Bone name, offset matrix, influence
	V_OKAY(lpSkinInfo->SetMinBoneInfluence(1e-4f));
	
	int j = 0;
	for (i = 0; i < (int)boneCount; i++)
	{
		Bone* bone = &currentSkeleton->bones[i];
		V_OKAY(lpSkinInfo->SetBoneName(i, bone->nameFixed.c_str()));
		V_OKAY(lpSkinInfo->SetBoneOffsetMatrix(i, &bone->offsetMatrix));

		// optimized
		//V_OKAY(lpSkinInfo->SetBoneInfluence(i, (DWORD)bone->influencingVertexCount, &bone->indices[0], &bone->weights[0]));

		// debugging
		if (i == 0)
		{
			bone->indices.resize(vertCount);
			bone->weights.resize(vertCount);
			for (j = (int)bone->infVertexCount; j < vertCount; j++)
			{
				bone->indices[j] = bone->indices[bone->infVertexCount-1];
				bone->weights[j] = bone->weights[bone->infVertexCount-1];

				//bone->indices[j] = 0xffff;
				//bone->weights[j] = 0.0f;
			}
			V_OKAY(lpSkinInfo->SetBoneInfluence(i, vertCount, &bone->indices[0], &bone->weights[0]));
			bone->indices.resize(bone->infVertexCount);
			bone->weights.resize(bone->infVertexCount);
		}
		else
		{
			V_OKAY(lpSkinInfo->SetBoneInfluence(i, (DWORD)bone->infVertexCount, &bone->indices[0], &bone->weights[0]));
		}
		
		
	}
	DWORD maxVertexInfluence, numBoneCombinations;
	LPD3DXBUFFER lpBoneCombinations = 0;
	LPD3DXMESH lpSkinnedMesh = 0;

	LPD3DXMESH lpTempMesh = this->lpMeshes[meshIndex];
	HRESULT hr;

	D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
	ZeroMemory(declaration, sizeof(declaration));


	DWORD* pNewAdjacency = new DWORD[lpTempMesh->GetNumFaces() * 3];
	if( 0 == pNewAdjacency )
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}
	hr = lpTempMesh->GenerateAdjacency( 1e-4f, pNewAdjacency );
	if( FAILED(hr) )
		goto e_Exit;

	//V_OKAY(lpTempMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, pNewAdjacency, 0, 0, 0));

	
	hr = lpSkinInfo->ConvertToIndexedBlendedMesh(
		lpTempMesh,
		D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE | D3DXMESH_MANAGED, // D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE | 
		(DWORD)boneCount,
		pNewAdjacency,
		0,
		0,
		0,
		&maxVertexInfluence,
		&numBoneCombinations,
		&lpBoneCombinations,
		&lpSkinnedMesh
		);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Convert failed"), _T("Error"), MB_OK | MB_ICONERROR);
		goto e_Exit;
	}
	D3DXBONECOMBINATION* boneComb = (D3DXBONECOMBINATION*)lpBoneCombinations->GetBufferPointer();
	
	this->lpSkinnedMeshes.push_back(lpSkinnedMesh);
	this->lpSkinnedMeshesSkinInfo.push_back(lpSkinInfo);
	lpSkinInfo->AddRef();

	hr = S_OK;


	// Debug skinned mesh declaration here...

	int numBytesPerVertex = lpTempMesh->GetNumBytesPerVertex();
	int numBytesPerVertexSkinned = lpSkinnedMesh->GetNumBytesPerVertex();

	D3DVERTEXELEMENT9 declMesh[MAX_FVF_DECL_SIZE];
	ZeroMemory(declMesh, sizeof(declMesh));
	lpTempMesh->GetDeclaration(declMesh);

	D3DVERTEXELEMENT9 declMeshSkinned[MAX_FVF_DECL_SIZE];
	ZeroMemory(declMeshSkinned, sizeof(declMeshSkinned));
	lpSkinnedMesh->GetDeclaration(declMeshSkinned);


	//
	// Debug Testing... (Change specific vertex value)
	//

	//LPDIRECT3DVERTEXBUFFER9 lpTempVB = 0;
	//lpSkinnedMesh->GetVertexBuffer(&lpTempVB);
	//struct SKINNED_MESH_VERT
	//{
	//	float x, y, z;
	//	float w0, w1, w2;
	//	unsigned char i0, i1, i2, i3;
	//	float nx, ny, nz;
	//	float color;
	//	float u, v;
	//};
	//SKINNED_MESH_VERT* skinnedVert = 0;
	//lpTempVB->Lock(0, 0, (void**)&skinnedVert, 0);

	//skinnedVert[85].x = -50.0f;
	//
	////skinnedVert[85].w0 = 0.1f;
	////skinnedVert[85].w1 = 0.2f;
	////skinnedVert[85].w2 = 0.7f;

	//lpTempVB->Unlock();
	//SAFE_RELEASE(lpTempVB);

e_Exit:
	SAFE_RELEASE(lpBoneCombinations);
	SAFE_RELEASE(lpSkinInfo);
	SAFE_DELETE_ARRAY(pNewAdjacency);
	return hr;
}
LPD3DXMESH ModelReader::GetSkinnedMeshPointer(int i) const
{
	return this->lpSkinnedMeshes[i];
}
LPD3DXSKININFO ModelReader::GetSkinInfoPointer(int i) const
{
	return this->lpSkinnedMeshesSkinInfo[i];
}
const SkeletonNode* ModelReader::GetSkeletonNodePointer(int index) const
{
	return &(this->skeletonNode[index]);
}


HRESULT ModelReader::BuildKeyframedAnimationSetOfSkeletonNodeIndex( int skeletonNodeIndex, int keyframeStartIndex, int keyframeEndIndex )
{
	
	ASSERTCHECK( keyframeStartIndex >= 0 );

	SkeletonNode* currentSkeleton = &this->skeletonNode[skeletonNodeIndex];
	size_t s;
	HRESULT hr;
	const int ticksPerSecond = 1;
	
	int totalBoneCount = 0;
	for (s = 0; s < this->skeletonNode.size(); s++)
	{
		totalBoneCount += (int)this->skeletonNode[s].bones.size();
	}

	//
	// Create animation set if 0
	// (this animation set is dedicated to keyframed animation of bone structure)
	//
	LPD3DXKEYFRAMEDANIMATIONSET lpKfAnimSet = 0; // temp animset
	if (this->lpKeyframedAnimationSet == 0)
	{
		/*std::tstring animationSetName(_T("Default Animation Set Of "));
		animationSetName = animationSetName + this->szFileName;*/
		STRING animSetName = currentSkeleton->associatedMeshName;
		animSetName += "-Animation";

		static char animSetNameChar[128];
		sprintf_s( animSetNameChar, 128, "%s-Animation-%d", currentSkeleton->associatedMeshName, (int)this->lpKeyframedAnimationSetList.size() );
		
		hr = D3DXCreateKeyframedAnimationSet(animSetNameChar, ticksPerSecond, D3DXPLAY_LOOP, totalBoneCount, 0, 0, &lpKfAnimSet);
		if (hr != D3D_OK)
			return hr;
	}

	int entireKeyframesCount = (int)currentSkeleton->bones[0].keys.size(); // assume all bones have the same amount of keyframes...
	int registeringKeyframesCount = entireKeyframesCount;

	ASSERTCHECK( entireKeyframesCount >= keyframeEndIndex );
	if ( keyframeStartIndex != 0 || keyframeEndIndex != -1 )
	{
		// called by explicit animset frame range
		registeringKeyframesCount = keyframeEndIndex - keyframeStartIndex + 1;
	}
	else
	{
		// default animset frame range: entire
		keyframeEndIndex = registeringKeyframesCount - 1;
	}
	ASSERTCHECK( keyframeEndIndex >= keyframeStartIndex);

	

	TCHAR debugString[512];
	_stprintf_s( debugString, TCHARSIZE(debugString), _T(" - Animation Set build by keyframe range: %d to %d\n"), keyframeStartIndex, keyframeEndIndex );
	OutputDebugString( debugString );
	if (lpKfAnimSet == 0)
		throw new std::runtime_error("Animation set is 0");

	for (s = 0; s < currentSkeleton->bones.size(); s++)
	{
		Bone* currentBone = &currentSkeleton->bones[s];

		this->AllocateAsAnimationSetFormat(registeringKeyframesCount, &currentBone->keys[keyframeStartIndex],
			&currentBone->scaleKeysSize, &currentBone->rotationKeysSize, &currentBone->translationKeysSize,
			&currentBone->scaleKeys, &currentBone->rotationKeys, &currentBone->translationKeys, TRUE);
		
		DWORD animIndex = 0;

		hr = lpKfAnimSet->RegisterAnimationSRTKeys(currentBone->nameFixed.c_str(),
			currentBone->scaleKeysSize, currentBone->rotationKeysSize, currentBone->translationKeysSize,
			currentBone->scaleKeys, currentBone->rotationKeys, currentBone->translationKeys,
			&animIndex);
		
		SAFE_DELETE_ARRAY(currentBone->scaleKeys);
		SAFE_DELETE_ARRAY(currentBone->translationKeys);
		SAFE_DELETE_ARRAY(currentBone->rotationKeys);

		
		_stprintf_s(debugString, TCHARSIZE(debugString), _T("    - Registered animation key index: %d\n"), animIndex);
		OutputDebugString(debugString);

		
		if (hr != D3D_OK)
			return hr;
	}

	// animset has all matrices corresponding to each bone transformation
	this->lpKeyframedAnimationSetList.push_back( lpKfAnimSet );


	return 0;
}


int ModelReader::AllocateAsAnimationSetFormat(
	UINT sourceArraySize, RST_DATA* sourceArray, UINT* pScaleSize, UINT* pRotationSize, UINT* pTranslationSize,
	D3DXKEY_VECTOR3** ppScale, D3DXKEY_QUATERNION** ppRotation, D3DXKEY_VECTOR3** ppTranslation, BOOL removeDuplicates)
{
	ASSERTCHECK( sourceArraySize >= 1 );

	*ppScale = new D3DXKEY_VECTOR3[sourceArraySize];
	*ppRotation = new D3DXKEY_QUATERNION[sourceArraySize];
	*ppTranslation = new D3DXKEY_VECTOR3[sourceArraySize];

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
			D3DXVECTOR3* pV3a = &( (*ppScale)[i-1].Value );
			D3DXVECTOR3* pV3b = &( (*ppScale)[i  ].Value );

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
					memcpy( &((*ppScale)[dupStartIndex]), &((*ppScale)[i-1]), sizeof(D3DXKEY_VECTOR3) * (*pScaleSize - i + 1) );
					memset( &((*ppScale)[*pScaleSize - (i - dupStartIndex) + 1]), 0xffffffff, sizeof(D3DXKEY_VECTOR3) * (i - dupStartIndex - 1) );
					*pScaleSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}

		dupStartIndex = -1;

		for ( i = 1; i < (int)(*pTranslationSize); i++ )
		{
			D3DXVECTOR3* pV3a = &((*ppTranslation)[i-1].Value);
			D3DXVECTOR3* pV3b = &((*ppTranslation)[i  ].Value);

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
					memcpy( &((*ppTranslation)[dupStartIndex]), &((*ppTranslation)[i-1]), sizeof(D3DXKEY_VECTOR3) * (*pTranslationSize - i + 1) );
					memset( &((*ppTranslation)[*pTranslationSize - (i - dupStartIndex) + 1]), 0xff, sizeof(D3DXKEY_VECTOR3) * (i - dupStartIndex - 1) );
					*pTranslationSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}

		dupStartIndex = -1;

		for ( i = 1; i < (int)(*pRotationSize); i++ )
		{
			D3DXQUATERNION* pV4a = &((*ppRotation)[i-1].Value);
			D3DXQUATERNION* pV4b = &((*ppRotation)[i  ].Value);

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
					memcpy( &((*ppRotation)[dupStartIndex]), &((*ppRotation)[i-1]), sizeof(D3DXKEY_QUATERNION) * (*pRotationSize - i + 1) );
					memset( &((*ppRotation)[*pRotationSize - (i - dupStartIndex) + 1]), 0xff, sizeof(D3DXKEY_QUATERNION) * (i - dupStartIndex - 1) );
					*pRotationSize -= (i - dupStartIndex - 1);
					i = dupStartIndex + 2;
					dupStartIndex = -1;
				}
			}
		}
	}


	return 0;
}

void ModelReader::UpdateBoneCombinedMatrixByMeshIndex(int meshIndex)
{
	MyFrame* frameRoot = this->GetFrameRootByMeshIndex(meshIndex);

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	UpdateBoneCombinedMatrixRecursive(frameRoot, identity);
}
void ModelReader::UpdateBoneCombinedMatrixRecursive(MyFrame* startFrame, D3DXMATRIX& parentCombinedTransform)
{
	startFrame->combinedMatrix = startFrame->TransformationMatrix * parentCombinedTransform;

	if (startFrame->pFrameSibling != 0)
		this->UpdateBoneCombinedMatrixRecursive((MyFrame*)startFrame->pFrameSibling, parentCombinedTransform);

	if (startFrame->pFrameFirstChild != 0)
		this->UpdateBoneCombinedMatrixRecursive((MyFrame*)startFrame->pFrameFirstChild, startFrame->combinedMatrix);
}
LPD3DXMESH ModelReader::GetMeshPointer(int i) const
{
	return this->lpMeshes[i];
}

int ModelReader::GetVBLength() const
{
	return this->notIndVertTotalSize;
}
LPDIRECT3DTEXTURE9 ModelReader::GetTexture(int referenceIndex) const
{
	if ( this->textureList.size() > (size_t)referenceIndex )
		return this->textureList[referenceIndex];
	else
		return 0;
}
const D3DMATERIAL9* ModelReader::GetMaterial(int referenceIndex) const
{
	const D3DMATERIAL9* ret = &(this->materialList[referenceIndex].d3dMat);
	return ret;
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
D3DLIGHT9 ModelReader::GetLight(int i) const
{
	return this->lights[i];
}
EXPORT_VERSION ModelReader::GetExportVersion() const
{
	return this->exportVersion;
}
LPD3DXKEYFRAMEDANIMATIONSET ModelReader::GetKeyframedAnimationSet(int animSetIndex) const
{
	return this->lpKeyframedAnimationSetList[animSetIndex];
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
		if (f->firstChild != -1)
		{
			f = &this->hierarchy[f->firstChild];
			goto search_again;
		}
	}

	return 0;
}
D3DXMATRIX* ModelReader::GetCombinedMatrixByBoneName( const char* boneName )
{
	size_t s;
	for (s = 0; s < this->hierarchy.size(); s++)
	{
		if (strcmp(this->hierarchy[s].nameFixed, boneName) == 0)
			return &this->hierarchy[s].combinedMatrix;
	}
	return 0;
}
const D3DXMATRIX* ModelReader::GetTransformationMatrixByBoneName( const char* boneName ) const
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
int ModelReader::GetMeshIndexBySkeletonIndex(int skelIndex) const
{
	const SkeletonNode* sn = &this->skeletonNode[skelIndex];

	int i;


	ASSERTCHECK( this->indTotalMeshCount > 0 );
	for (i = 0; i < this->indTotalMeshCount; i++)
	{
		const STRING* meshName = &this->indMeshNames[i];
		if (strcmp(meshName->c_str(), sn->associatedMeshName) == 0)
		{
			return i;
		}
	}
	return -1;
}
int ModelReader::GetSkeletonIndexByMeshIndex(int meshIndex) const
{
	const STRING* meshName = &this->indMeshNames[meshIndex];
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



HRESULT ModelReader::Initialize( LPDIRECT3DDEVICE9 lpDev, DWORD fvf,
								HWND hLoadingWnd, const TCHAR* fileName, LPD3DXANIMATIONCONTROLLER lpAC, const BOOL initAC )
{
	std::tstring debugOutput(_T(" - Initialize ARN File: "));
	if ( fileName && _tcslen( fileName ) )
		debugOutput += fileName;
	else
		debugOutput += this->szFileName;
	debugOutput += _T('\n');
	OutputDebugString(debugOutput.c_str());

	HRESULT hr;

	this->SetLoadingWindowHandle(hLoadingWnd);
	this->SetDev(lpDev);
	this->SetFVF(fvf);
	
	if ( _tcslen( szFileName ) == 0 )
	{
		int size = this->Read(fileName);
		_tcscpy_s( this->szFileName, TCHARSIZE(this->szFileName), fileName );
		std::tstring globalPathFileName(_T(GLOBAL_ARN_FILE_PATH));
		globalPathFileName += fileName;
		if (size < 0)
		{
			size = this->Read(globalPathFileName.c_str());
			_tcscpy_s( this->szFileName, TCHARSIZE(this->szFileName), globalPathFileName.c_str() );
		}
		if (size < 0)
			return E_FAIL;
	}
	else
	{
		int size = this->Read(szFileName);
		std::tstring globalPathFileName(_T(GLOBAL_ARN_FILE_PATH));
		globalPathFileName += szFileName;
		if (size < 0)
			size = this->Read(globalPathFileName.c_str());
		if (size < 0)
			return E_FAIL;

	}

	// if lpAC is 0, this means the model use its own (local) AC
	if (lpAC == 0)
	{
		// TODO: optimized max counts in AC
		if (this->skeletonNode.size() > 0)
		{
			V_OKAY( D3DXCreateAnimationController(
				50, /* MaxNumMatrices */
				10, /* MaxNumAnimationSets */
				2, /* MaxNumTracks */
				10, /* MaxNumEvents */
				&this->lpAnimationController
				) );
		}
		else
		{
			V_OKAY( D3DXCreateAnimationController(
				this->indTotalMeshCount + this->notIndTotalMeshCount, /* MaxNumMatrices */
				1, /* MaxNumAnimationSets */
				2, /* MaxNumTracks */
				10, /* MaxNumEvents */
				&this->lpAnimationController
				) );
		}
		this->useLocalAC = TRUE;
	}
	else
	{
		this->lpAnimationController = lpAC;
		this->useLocalAC = FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	// General mesh animation loading (single animation set first)
	//////////////////////////////////////////////////////////////////////////
	LPD3DXKEYFRAMEDANIMATIONSET lpKeyframedAnimSet = 0;
	LPD3DXANIMATIONSET lpAnimSetTemp = 0;
	int ticksPerSecond = 1;

	int generalMeshCount = -1;
	if ( this->exportVersion == EV_ARN10 || this->exportVersion == EV_ARN11 )
	{
		generalMeshCount = this->notIndTotalMeshCount;
	}
	else if ( this->exportVersion == EV_ARN20 )
	{
		generalMeshCount = this->indTotalMeshCount;
	}
	else
	{
		ASSERTCHECK( !"Unexpected ARN format" );
	}

	hr = D3DXCreateKeyframedAnimationSet("Default General Mesh Anim Set", ticksPerSecond, D3DXPLAY_LOOP,
		generalMeshCount, 0, 0, &lpKeyframedAnimationSet);
	if (FAILED(hr))
		goto e_NoGeneralMeshAnim;

	this->animMatControlledByAC.resize(generalMeshCount);
	int i = 0, j = 0;
	for (i = 0; i < generalMeshCount; i++)
	{
		DWORD animIndex = 0;
		ASSERTCHECK(this->animQuatSize[i] > 0);
		UINT keysCount = (UINT)this->animQuatSize[i];
		UINT scaleKeysCount = keysCount;
		UINT rotationKeysCount = keysCount;
		UINT translationKeysCount = keysCount;
		STRING keysName;
		if ( this->exportVersion == EV_ARN10 || this->exportVersion == EV_ARN11 )
		{
			keysName = this->notIndMeshNames[i] + "-DefaultKeys";
		}
		else if ( this->exportVersion == EV_ARN20 )
		{
			keysName = this->indMeshNames[i] + "-DefaultKeys";
		}

		LPD3DXKEY_VECTOR3 scaleKeys = 0; //new D3DXKEY_VECTOR3[keysCount];
		LPD3DXKEY_QUATERNION rotationKeys = 0; //new D3DXKEY_QUATERNION[keysCount];
		LPD3DXKEY_VECTOR3 translationKeys = 0; //new D3DXKEY_VECTOR3[keysCount];

		this->AllocateAsAnimationSetFormat(keysCount, this->animQuat[i],
			&scaleKeysCount, &rotationKeysCount, &translationKeysCount,
			&scaleKeys, &rotationKeys, &translationKeys, TRUE);

		hr = lpKeyframedAnimationSet->RegisterAnimationSRTKeys(keysName.c_str(),
			scaleKeysCount, rotationKeysCount, translationKeysCount,
			scaleKeys, rotationKeys, translationKeys,
			&animIndex);
		V_OKAY(hr);

		SAFE_DELETE_ARRAY(scaleKeys);
		SAFE_DELETE_ARRAY(rotationKeys);
		SAFE_DELETE_ARRAY(translationKeys);

		V_OKAY(this->lpAnimationController->RegisterAnimationOutput(keysName.c_str(), &this->animMatControlledByAC[i], 0, 0, 0));
	}
	
	this->lpAnimationController->RegisterAnimationSet(lpKeyframedAnimationSet);


	V_OKAY(this->lpAnimationController->GetAnimationSet(0, &lpAnimSetTemp));

	V_OKAY(this->lpAnimationController->SetTrackAnimationSet(0, lpAnimSetTemp));
	V_OKAY(this->lpAnimationController->SetTrackEnable(0, TRUE));
	V_OKAY(this->lpAnimationController->SetTrackWeight(0, 1.0f));
	V_OKAY(this->lpAnimationController->SetTrackSpeed(0, 3.0f));
	V_OKAY(this->lpAnimationController->SetTrackPosition(0, 0.0f));
	this->lpAnimationController->ResetTime();
	
e_NoGeneralMeshAnim:
	SAFE_RELEASE(lpAnimSetTemp);
	SAFE_RELEASE(lpKeyframedAnimationSet);


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Skeletal animation loading
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//this->lpAnimationController = lpAC;
	int animSetNum = this->lpAnimationController->GetNumAnimationSets();

	if (initAC == TRUE)
	{
		// Unregister all animation sets (at editor)
		LPD3DXANIMATIONSET lpAnimSetTemp2 = 0;
		for (i = 0; i < animSetNum; i++)
		{
			V_OKAY(this->lpAnimationController->GetAnimationSet(i, &lpAnimSetTemp2));
			if (lpAnimSetTemp2)
			{
				V_OKAY(this->lpAnimationController->UnregisterAnimationSet(lpAnimSetTemp2));
				SAFE_RELEASE(lpAnimSetTemp2);
			}
		}

		animSetNum = 0; // set anim set count to zero
	}
	
	
	int skeletonNodeSize = (int)this->GetSkeletonNodeSize();
	for (i = 0; i < skeletonNodeSize; i++)
	{
		int meshIndex = this->GetMeshIndexBySkeletonIndex(i);
		ASSERTCHECK(meshIndex >= 0);
		V_OKAY(this->BuildBlendedMeshByMeshIndex(meshIndex));
		V_OKAY(this->BuildBoneHierarchyByMeshIndex(meshIndex));
		
		V_OKAY(this->BuildKeyframedAnimationSetOfSkeletonNodeIndex( i ));
		//V_OKAY(this->BuildKeyframedAnimationSetOfSkeletonNodeIndex( i, 0, 30 ));
		//V_OKAY(this->BuildKeyframedAnimationSetOfSkeletonNodeIndex( i, 48, 60 ));
		
	}

	if (skeletonNodeSize > 0)
	{
		V_OKAY( this->lpAnimationController->RegisterAnimationSet( this->GetKeyframedAnimationSet(0) ) );
		//V_OKAY( this->lpAnimationController->RegisterAnimationSet( this->GetKeyframedAnimationSet(1) ) );

		LPD3DXANIMATIONSET lpAnimSetTemp2 = 0;
		

		// First Track ... (Loiter)
		V_OKAY( this->lpAnimationController->GetAnimationSet( animSetNum, &lpAnimSetTemp2 ) );
		V_OKAY( this->lpAnimationController->SetTrackAnimationSet( 0, lpAnimSetTemp2 ) );
		V_OKAY( this->lpAnimationController->SetTrackWeight( 0, 1.0f ) );
		V_OKAY( this->lpAnimationController->SetTrackSpeed( 0, 3.5f ) );
		V_OKAY( this->lpAnimationController->SetTrackPosition( 0, 0.0f ) );
		V_OKAY( this->lpAnimationController->SetTrackEnable( 0, TRUE ) );

		SAFE_RELEASE( lpAnimSetTemp2 )

		//animSetNum++;

		//// Second Track... (Walking)
		//V_OKAY( this->lpAnimationController->GetAnimationSet( animSetNum, &lpAnimSetTemp ) );
		//V_OKAY( this->lpAnimationController->SetTrackAnimationSet( 1, lpAnimSetTemp ) );
		//V_OKAY( this->lpAnimationController->SetTrackWeight( 1, 0.0f ) );
		//V_OKAY( this->lpAnimationController->SetTrackSpeed( 1, 3.5f ) );
		//V_OKAY( this->lpAnimationController->SetTrackPosition( 1, 0.0f ) );
		//V_OKAY( this->lpAnimationController->SetTrackEnable( 1, FALSE ) );

		//SAFE_RELEASE(lpAnimSetTemp);


		MyFrame* frameRoot = 0;

		for (i = 0; i < (int)this->GetSkeletonNodeSize(); i++)
		{
			int meshIndex = this->GetMeshIndexBySkeletonIndex(i);
			ASSERTCHECK(meshIndex >= 0);
			frameRoot = this->GetFrameRootByMeshIndex(meshIndex);
			
			ASSERTCHECK( frameRoot );

			V_OKAY(D3DXFrameRegisterNamedMatrices(frameRoot, this->lpAnimationController));
		}

		
		frameRoot = 0;
	}

	this->initialized = TRUE;

	return S_OK;
}

ArnNodeHeader ModelReader::GetArnNodeHeader(int idx)
{
	return this->nodeHeaders[idx];
}
size_t ModelReader::GetArnNodeHeadersSize()
{
	return this->nodeHeaders.size();
}
HRESULT ModelReader::AdvanceTime(float timeDelta) const
{
	if (this->initialized)
		return this->lpAnimationController->AdvanceTime(timeDelta, 0);
	else
		return E_FAIL;
}
const D3DXMATRIX* ModelReader::GetAnimMatControlledByAC(int meshIndex) const
{
	return (const D3DXMATRIX*)&this->animMatControlledByAC[meshIndex];
}
const TCHAR* ModelReader::GetFileNameOnly()
{
	int i = 0;
	for (i = (int)_tcslen(szFileName)-1; i >= 0; i--)
	{
		if (szFileName[i] == _T('\\'))
			break;
	}
	return (const TCHAR*)&szFileName[i+1];
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













//////////////////////////////////////////////////////////////////////////
// Static Global Functions
//////////////////////////////////////////////////////////////////////////


static inline BOOL almostEqualFloat3(D3DXVECTOR3* pV1, D3DXVECTOR3* pV2)
{
	return ( fabsf( pV1->x - pV2->x ) < COMPARE_EPSILON )
		&& ( fabsf( pV1->y - pV2->y ) < COMPARE_EPSILON )
		&& ( fabsf( pV1->z - pV2->z ) < COMPARE_EPSILON );
}

static inline BOOL almostEqualFloat3(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < COMPARE_EPSILON );
}
static inline BOOL almostEqualFloat4(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[3] - floatArray2[3] ) < COMPARE_EPSILON );
}