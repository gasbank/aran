// VideoMan.cpp
// 2007, 2008, 2009 Geoyeob Kim
#include "AranPCH.h"
#include "VideoMan.h"
#include "ArnCamera.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "ArnMath.h"
#include "Animation.h"
#include "ArnTexture.h"
#include "ArnLight.h"
#include "ArnSceneGraph.h"
#include "ArnMath.h"
#include "ArnViewportData.h"
#include "PreciseTimer.h"
#include "RenderLayer.h"
IMPLEMENT_SINGLETON(VideoMan)

VideoMan::VideoMan()
: shaderFlags(ARNSHADER_DEBUG)
, m_bRendering(false)
, m_bAllowRenderNewFrame(true)
, drawCount(0)
, curFrameIndex(0)
, okayToDestruct(FALSE)
, totalLightCount(0)
, m_dLastRefreshedTime(0)
, m_preciseTimer(new CPreciseTimer)
, m_sceneGraph(0)
{
	std::cout << "Constructing VideoMan..." << std::endl;

	dungeonTranslation = CreateArnVec3( 0.0f, 0.0f, 0.0f );

	ArnMatrixIdentity(&this->modelArcBallRotation);

	m_prevLastTime = 0.0f;
	m_prevLastTime = 0.0f;
	m_fFPS = 0.0f;
	m_prevUpdateFrames = 0;
	m_preciseTimer->StartTimer();
}

VideoMan::~VideoMan()
{
	std::cout << "Destructing VideoMan..." << std::endl;
	unregisterAllRenderLayers();
	delete m_preciseTimer;
}

/*
VideoMan*
VideoMan::create(RendererType type, int width, int height, int argc, char** argv)
{
	VideoMan* vm = 0;
	if (type == RENDERER_DX9)
	{
#ifdef WIN32
		vm = VideoManDx9::create();
#else
		fprintf(stderr, "In Linux platform, DX9 renderer is not supported.\n");
#endif
	}
	else if (type == RENDERER_GL)
	{
		vm = VideoManGl::create(argc, argv, width, height);
	}
	return vm;
}
*/

HRESULT
VideoMan::InitShader()
{

	ARN_THROW_NOT_IMPLEMENTED_ERROR

//	HRESULT hr = S_OK;
//	LPD3DXBUFFER lpErrorBuffer = 0;
//
//	//////////////////////////////////////////////////////////////////////////
//	// Testing Shader
//	//////////////////////////////////////////////////////////////////////////
//	hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("Shaders\\basic.fx"), 0, 0, this->shaderFlags, 0, &this->lpEffect, 0);
//	if (hr != D3D_OK)
//	{
//		hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("..\\Aran\\basic.fx"), 0, 0, this->shaderFlags, 0, &this->lpEffect, 0);
//		if (hr != D3D_OK)
//		{
//			MessageBox(0, _T("Shader file not found."), _T("Error"), MB_OK | MB_ICONERROR);
//			hr = -1234;
//			goto e_Exit;
//		}
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	// Skinned Mesh Shader
//	//////////////////////////////////////////////////////////////////////////
//	//MessageBox(0, _T("About to call D3DXCreateEffectFromFile()"), _T("Notice"), MB_OK);
//
//	hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("Shaders\\vertBlendDynamic.fx"), 0, 0, D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG, 0, &this->lpEffectSkinning, &lpErrorBuffer);
//	if (hr != D3D_OK)
//	{
//		hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("..\\Aran\\vertBlendDynamic.fx"), 0, 0, D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG, 0, &this->lpEffectSkinning, &lpErrorBuffer);
//		if (hr != D3D_OK)
//		{
//			MessageBox(0, _T("D3DXCreateEffectFromFile() failed"), _T("Error"), MB_OK | MB_ICONERROR);
//			hr = -12345;
//			goto e_Exit;
//		}
//	}
//	if (lpErrorBuffer)
//	{
//		MessageBoxA(0, (char*)lpErrorBuffer->GetBufferPointer(), "D3DXCreateEffectFromFile()", MB_OK | MB_ICONERROR);
//		hr = E_FAIL;
//		goto e_Exit;
//	}
//
//	hr = S_OK;
//
//e_Exit:
//	SAFE_RELEASE(lpErrorBuffer);
//	return hr;
}

HWND hProgress;

LRESULT CALLBACK
LoadingDialogProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef WIN32

	switch(Msg)
	{

	case WM_NOTIFY:
		//MessageBox(hWndDlg, _T("Notify"), _T("Message"), MB_OK);
		break;
	case WM_SHOWWINDOW:
		//MessageBox(hWndDlg, _T("ShowWindow"), _T("Message"), MB_OK);
		break;
	case WM_INITDIALOG:
		//MessageBox(hWndDlg, _T("Hello"), _T("Message"), MB_OK);
		//SendMessage(hWndDlg, WM_USER+1, 10, 20);

		hProgress = CreateWindowEx(0, PROGRESS_CLASS, 0, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 20, 20, 260, 17, hWndDlg, 0, 0, 0);

		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		//SendMessage(hProgress, PBM_SETPOS, 80, 0);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			EndDialog(hWndDlg, 0);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	case WM_USER+1:
		//MessageBox(hWndDlg, _T("Message Arrived!"), _T("Message"), MB_OK);
		break;
	}
#endif

	return FALSE;
}

void
VideoMan::SetWindowSize(int w, int h)
{
	this->screenWidth = w;
	this->screenHeight = h;
}

HRESULT
VideoMan::InitMainCamera()
{
	if (ArnVec3Equals(getMainCamera()->up, ArnConsts::ARNVEC3_ZERO))
	{
		this->mainCamera.eye = CreateArnVec3( 0.0f, 0.0f, -50.0f );
		this->mainCamera.at = CreateArnVec3( 0.0f, 0.0f, 0.0f );
		this->mainCamera.up = CreateArnVec3( 0.0f, 1.0f, 0.0f );

		this->mainCamera.farClip = 1000.0f;
		this->mainCamera.nearClip = 1.0f;
	}
	return S_OK;
}

void
VideoMan::updateTime()
{
	m_fTime = m_preciseTimer->GetTimeInSeconds();
}

int
VideoMan::Draw()
{
	drawCount++;
	DWORD dwFrames = m_prevUpdateFrames;
	dwFrames++;
	m_prevUpdateFrames = dwFrames;
	m_fElapsedTime = (float)(m_fTime - m_prevLastTime);
	// Update the scene stats once per second
	if( m_fTime - m_prevLastTime > 1.0f )
	{
		m_fFPS = ( float )( dwFrames / ( m_fTime - m_prevLastTime ) );
		m_prevUpdateFrames = 0;
		m_prevLastTime = m_fTime;
	}
	//////////////////////////////////////////////////////////////////////////

	m_dLastRefreshedTime = m_preciseTimer->GetTimeInSeconds();

	return 0;
}

HRESULT
VideoMan::SetCamera(float x, float y, float z)
{
	this->cameraVector.x = x;
	this->cameraVector.y = y;
	this->cameraVector.z = z;

	return S_OK;
}

HRESULT
VideoMan::SetCamera( ARN_NDD_CAMERA_CHUNK* pCamChunk )
{
	this->mainCamera.eye.x = pCamChunk->pos.x;
	this->mainCamera.eye.y = pCamChunk->pos.y;
	this->mainCamera.eye.z = pCamChunk->pos.z;

	ArnVec3 lookAtVector = ArnConsts::ARNVEC3_Z;
	ArnVec3 upVector = ArnConsts::ARNVEC3_Y;
	ArnMatrix upVectorRot;
	ArnQuat quat( pCamChunk->rot.x, pCamChunk->rot.y, pCamChunk->rot.z, pCamChunk->rot.w );
	ArnMatrixRotationQuaternion( &upVectorRot, &quat );

	ArnVec4 upv;
	ArnVec3Transform( &upv, &upVector, &upVectorRot );
	ArnVec3TransformNormal( &upVector, &upVector, &upVectorRot );
	ArnVec3TransformNormal( &lookAtVector, &lookAtVector, &upVectorRot );
	this->mainCamera.up = CreateArnVec3( pCamChunk->upVector.x, pCamChunk->upVector.y, pCamChunk->upVector.z );
	this->mainCamera.at = CreateArnVec3( pCamChunk->lookAtVector.x, pCamChunk->lookAtVector.y, pCamChunk->lookAtVector.z );
	this->mainCamera.farClip = pCamChunk->farClip;
	this->mainCamera.nearClip = pCamChunk->nearClip;
	return S_OK;
}

HRESULT
VideoMan::SetCamera( ArnCamera& arnCam )
{
	arnCam.recalcLocalXform();
	ArnMatrix localTf = arnCam.getLocalXform();

#ifdef WIN32
	localTf = ArnMatrixTranspose(localTf);
#endif
	mainCamera.eye.x = localTf.m[0][3];
	mainCamera.eye.y = localTf.m[1][3];
	mainCamera.eye.z = localTf.m[2][3];
	mainCamera.at.x = localTf.m[0][3] - localTf.m[0][2];
	mainCamera.at.y = localTf.m[1][3] - localTf.m[1][2];
	mainCamera.at.z = localTf.m[2][3] - localTf.m[2][2];
	mainCamera.up.x = localTf.m[0][1];
	mainCamera.up.y = localTf.m[1][1];
	mainCamera.up.z = localTf.m[2][1];
	mainCamera.nearClip = arnCam.getNearClip();
	mainCamera.farClip = arnCam.getFarClip();
	mainCamera.angle = arnCam.getFov(); // FOV in radian
	return S_OK;
}

void
VideoMan::Close()
{
	this->closeNow = TRUE;
}

bool
VideoMan::PauseMainLoop()
{
	if (m_bRendering)
	{
		m_bAllowRenderNewFrame = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool
VideoMan::ResumeMainLoop()
{
	if (!m_bRendering)
	{
		m_bAllowRenderNewFrame = true;
		return true;
	}
	else
	{
		return false;
	}
}

HRESULT
VideoMan::InitModels()
{
	/*this->mrMan.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
		_T("man.arn"), 0 );*/
	/*V_OKAY(this->lpAnimationController->ResetTime());
	V_OKAY(this->lpAnimationController->AdvanceTime(0.0f, 0));*/

	return S_OK;
}

void
VideoMan::MoveMainCameraEye(float dx, float dy, float dz)
{
	this->mainCamera.eye.x += dx;
	this->mainCamera.eye.y += dy;
	this->mainCamera.eye.z += dz;


	/*this->cameraVector.x += dx;
	this->cameraVector.y += dy;
	this->cameraVector.z += dz;*/
}

void
VideoMan::ToggleLeftPattern()
{
	this->leftPattern = (this->leftPattern == TRUE) ? FALSE : TRUE;
}

void
VideoMan::ToggleRightPattern()
{
	this->rightPattern = (this->rightPattern == TRUE) ? FALSE : TRUE;
}

HRESULT
VideoMan::InitLight()
{
	ZeroMemory(&this->defaultLight, sizeof(ArnLightData));
	this->defaultLight.Type = ARNLIGHT_DIRECTIONAL;
	this->defaultLight.Diffuse.r = 0.5f;
	this->defaultLight.Diffuse.g = 0.5f;
	this->defaultLight.Diffuse.b = 0.5f;
	this->defaultLight.Diffuse.a = 1.0f;
	ArnVec3 dir = ArnConsts::ARNVEC3_Z;
	this->defaultLight.Direction = dir;

	/*ZeroMemory(&this->pointLight, sizeof(D3DLIGHT9));
	this->pointLight.Type = D3DLIGHT_POINT;
	this->pointLight.Diffuse.r = 0.8f;
	this->pointLight.Diffuse.g = 0.8f;
	this->pointLight.Diffuse.b = 0.8f;
	this->pointLight.Diffuse.a = 1.0f;
	this->pointLight.Ambient = this->pointLight.Diffuse;
	this->pointLight.Range = 100.0f;
	this->pointLight.Attenuation0 = 0.0f;
	this->pointLight.Attenuation1 = 0.125f;
	this->pointLight.Attenuation2 = 0.0f;*/

	ArnVec3 pos = CreateArnVec3(0.0f, 0.0f, -50.0f);
	this->pointLight.Position = pos;
	//this->pointLight.Direction = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	InitLight_Internal();

	return S_OK;
}

HRESULT
VideoMan::InitMaterial()
{
	ZeroMemory(&this->defaultMaterial, sizeof(ArnMaterialData));
	ZeroMemory(&this->rgbMaterial, sizeof(ArnMaterialData) * 3);
	SetSimpleColoredMaterial(&this->defaultMaterial, ArnColorValue4f(0.5f, 0.5f, 0.5f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[0], ArnColorValue4f(1.0f, 0.0f, 0.0f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[1], ArnColorValue4f(0.0f, 1.0f, 0.0f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[2], ArnColorValue4f(0.0f, 0.0f, 1.0f, 1.0f));
	return S_OK;
}

HRESULT
VideoMan::SetCurrentFrameIndex(int idx)
{
	ASSERTCHECK(idx >= 0);
	this->curFrameIndex = idx;
	return S_OK;
}

HRESULT
VideoMan::InitAnimationController()
{
	///////////////////////////////////////////////////////////////////////////
	// ANIMATION CONTROLLER CREATION
	///////////////////////////////////////////////////////////////////////////

	/*const UINT MaxNumMatrices = 100;
	const UINT MaxNumAnimationSets = 100;
	const UINT MaxNumTracks = 100;
	const UINT MaxNumEvents = 300;
	V_OKAY(ArnCreateAnimationController(MaxNumMatrices, MaxNumAnimationSets, MaxNumTracks, MaxNumEvents, &this->lpAnimationController));*/

	return S_OK;
}

HRESULT
VideoMan::InitFont()
{
	//V_OKAY( D3DXCreateFont( this->lpD3DDevice, 14, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_RASTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Gulim"), &this->lpFont ) );

	return S_OK;
}

void
VideoMan::ScrollBy( ArnVec3* dScroll )
{
	this->dungeonTranslation = ArnVec3Add(this->dungeonTranslation, *dScroll);
}

size_t
VideoMan::registerRenderLayer( RenderLayer* pRL )
{
	pRL->setVideoMan( this );
	//pRL->setDev( this->lpD3DDevice );

	m_renderLayers.push_back(pRL);
	return m_renderLayers.size();
}

BOOL
VideoMan::unregisterRenderLayerAt( unsigned int ui )
{
	if ( ui >= m_renderLayers.size() )
		return FALSE;

	std::list<RenderLayer*>::iterator it = m_renderLayers.begin();
	while (ui--)
		it++;
	delete *it;
	m_renderLayers.erase(it);
	return TRUE;
}

RenderLayer*
VideoMan::getRenderLayerAt( unsigned int ui )
{
	std::list<RenderLayer*>::iterator it = m_renderLayers.begin();
	while (ui--)
		it++;
	return *it;
}

size_t
VideoMan::unregisterAllRenderLayers()
{
	size_t ret = m_renderLayers.size();
	while (m_renderLayers.size())
		unregisterRenderLayerAt( 0 );
	return ret;
}

void
VideoMan::renderMeshesOnly( ArnNode* node, const ArnMatrix& globalXform )
{
	if ( node->getType() == NDT_RT_MESH )
		renderSingleMesh( static_cast<ArnMesh*>( node ), globalXform );

	unsigned int i;
	unsigned int nodeCount = node->getNodeCount();
	for (i = 0; i < nodeCount; ++i)
	{
		if (node->getNodeAt(i)->getType() == NDT_RT_MESH)
		{
			ArnMesh* mesh = static_cast<ArnMesh*>(node->getNodeAt(i));
			renderSingleMesh( mesh, globalXform );
		}
		renderMeshesOnly(node->getNodeAt(i), globalXform);
	}
}

void
VideoMan::setFrameMoveCallback( LPARNCALLBACKFRAMEMOVE pCallback )
{

}

HRESULT
VideoMan::StartMainLoop()
{
	this->m_bAllowRenderNewFrame = true;
	this->leftPattern = FALSE;
	this->rightPattern = FALSE;


	// initial camera position
	this->cameraVector.x = 0.0f;
	this->cameraVector.y = 0.0f;
	this->cameraVector.z = -100.0f;

	return S_OK;
}

void
VideoMan::resetModelArcBallRotation()
{
	ArnMatrixIdentity(&modelArcBallRotation);
}

void
VideoMan::resetWorldMatrix()
{
	ArnMatrixIdentity(&matWorld);
}

void
VideoMan::attachSceneGraph(ArnSceneGraph* sceneGraph)
{
	assert(!m_sceneGraph);
	m_sceneGraph = sceneGraph;
}

ArnSceneGraph*
VideoMan::detachSceneGraph()
{
	assert(m_sceneGraph);
	ArnSceneGraph* ret = m_sceneGraph;
	m_sceneGraph = 0;

	disableAllLights();
	return ret;
}

void
VideoMan::setupLightsFromSceneGraph()
{
	// Apply lights. This will override existing light values of OpenGL.
	if (m_sceneGraph)
	{
		unsigned int lightId = 0;
		foreach(ArnNode* sgRootNode, m_sceneGraph->getChildren())
		{
			if (sgRootNode->getType() == NDT_RT_LIGHT)
			{
				ArnLight* light = dynamic_cast<ArnLight*>(sgRootNode);
				//std::cout << "Applied light name: " << light->getName() << std::endl;
				setLight(lightId, light);
				++lightId;
			}
		}
		for (unsigned int lightId2 = lightId; lightId2 < 8; ++lightId2)
		{
			setLight(lightId2, 0);
		}
	}
}

void
VideoMan::disableAllLights()
{
	for (unsigned int lightId2 = 0; lightId2 < 8; ++lightId2)
	{
		setLight(lightId2, 0);
	}
}

bool
VideoMan::setRenderStarted()
{
	if (m_bAllowRenderNewFrame)
	{
		assert(!m_bRendering);
		m_bRendering = true;
		return true;
	}
	else
		return false;
}

void
VideoMan::setRenderFinished()
{
	assert(m_bRendering);
	m_bRendering = false;
}

void
VideoMan::updateFrame( double dTime, float fElapsedTime )
{
	(this->m_updateFrameCb)(dTime, fElapsedTime);
}

void
VideoMan::renderFrame()
{
	(this->m_renderFrameCb)();
}

void
VideoMan::updateSceneGraph(double dTime, float fElapsedTime)
{
	if (!m_sceneGraph)
		return;
	assert(dTime > 0);
	m_sceneGraph->update(dTime, fElapsedTime);

	// Show some warning messages about time control.
	if (fElapsedTime < 0)
		fprintf(stderr, "  WARN: Elapsed time is negative; value = %.3f\n", fElapsedTime);
	else if (fElapsedTime > 1.0/30)
		fprintf(stderr, "  WARN: Elapsed time is too long between frames; %.3f\n", fElapsedTime);
}

void
VideoMan::renderSceneGraph()
{
	if (getSceneGraph())
		getSceneGraph()->render();
}

//////////////////////////////////////////////////////////////////////////

ArnMatrix*
ArnGetProjectionMatrix(ArnMatrix* out, const ArnViewportData* viewportData, const ArnCamera* cam, bool rightHanded)
{
	float aspect = (float)viewportData->Width / viewportData->Height;
	if (cam->isOrtho())
	{
		return ArnMatrixOrthoRH(out, cam->getOrthoScale() * aspect, cam->getOrthoScale(), cam->getNearClip(), cam->getFarClip());
	}
	else
	{
		return ArnMatrixPerspectiveYFov(out, cam->getFov(), aspect, cam->getNearClip(), cam->getFarClip(), rightHanded);
	}
}

HRESULT
ArnIntersectGl( ArnMesh* pMesh, const ArnVec3* pRayPos, const ArnVec3* pRayDir, bool* pHit, unsigned int* pFaceIndex, FLOAT* pU, FLOAT* pV, FLOAT* pDist, ArnGenericBuffer* ppAllHits, unsigned int* pCountOfHits )
{
	const unsigned int faceGroupCount = pMesh->getFaceGroupCount();
	for (unsigned int fg = 0; fg < faceGroupCount; ++fg)
	{
		unsigned int triCount, quadCount;
		pMesh->getFaceCount(triCount, quadCount, fg);

        // Ray-intersection supported only on triangles.
		for (unsigned int tc = 0; tc < triCount; ++tc)
		{
			unsigned int totalIndex;
			unsigned int tinds[3];
			pMesh->getTriFace(totalIndex, tinds, fg, tc);
			ArnVec3 verts[3];
			for (unsigned int v = 0; v < 3; ++v)
			{
				pMesh->getVert(&verts[v], 0, 0, tinds[v], true);
			}
			float t = 0, u = 0, v = 0;
			if (ArnIntersectTriangle(&t, &u, &v, pRayPos, pRayDir, verts))
			{
				if (pFaceIndex)
					*pFaceIndex = totalIndex;
				*pHit = true;
				return S_OK;
			}
		}

		/*
		for (unsigned int qc = 0; qc < quadCount; ++qc)
		{
		unsigned int totalIndex;
		unsigned int qinds[4];
		pMesh->getQuadFace(totalIndex, qinds, fg, qc);
		ArnVec3 verts[4];
		for (unsigned int v = 0; v < 4; ++v)
		pMesh->getVert(&verts[v], 0, 0, 0, tinds[v]);
		float t = 0, u = 0, v = 0;
		if (ArnIntersectTriangle(&t, &u, &v, pRayPos, pRayDir, verts))
		{
		return S_OK;
		}
		}
		*/
	}

	*pHit = false;
	return S_OK;
}
