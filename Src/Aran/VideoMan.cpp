// VideoMan.cpp
// 2007 Geoyeob Kim
#include "stdafx.h"

#include "VideoMan.h"
#include "Character.h"
#include "RenderLayer.h"
#include "ArnCamera.h"
IMPLEMENT_SINGLETON(VideoMan);


VideoMan::VideoMan()
:lpCompiledFragments(0),
shaderFlags(D3DXSHADER_DEBUG),
lpFragmentLinker(0),
lpVertexShader(0),
lpConstantTable(0),
drawCount(0),
totalLightCount(0),
lpSkinInfo(0),
lpCustomSkinnedMesh(0),
lpTex1(0), lpBoxVB(0), lpEffect(0), lpEffectSkinning(0), lpCustomMesh(0), lpTestVB(0), lpTestVB2(0), lpAnimationController(0),
lpDefaultAnimationSet(0), lpFont(0), lpD3DDevice(0), lpD3D(0), isOkayToDestruct(FALSE),
pDrawingModel(0), curFrameIndex(0)
{
	std::cout << "Constructing VideoMan..." << std::endl;

	dungeonTranslation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

	D3DXMatrixIdentity(&this->modelArcBallRotation);

}
VideoMan::~VideoMan()
{
	std::cout << "Destructing VideoMan..." << std::endl;
	
	//OutputDebugString(_T(" - Waiting for VideoMan to destruct...\n"));


	SAFE_RELEASE(this->lpTex1);
	SAFE_RELEASE(this->lpBoxVB);
	SAFE_RELEASE(this->lpCompiledFragments);
	SAFE_RELEASE(this->lpFragmentLinker);
	SAFE_RELEASE(this->lpVertexShader);
	SAFE_RELEASE(this->lpConstantTable);
	SAFE_RELEASE(this->lpEffect);
	SAFE_RELEASE(this->lpEffectSkinning);
	SAFE_RELEASE(this->lpCustomMesh);
	SAFE_RELEASE(this->lpCustomSkinnedMesh);
	SAFE_RELEASE(this->lpTestVB);
	SAFE_RELEASE(this->lpTestVB2);
	SAFE_RELEASE(this->lpAnimationController);
	SAFE_RELEASE(this->lpDefaultAnimationSet);
	SAFE_RELEASE(this->lpSkinInfo);
	SAFE_RELEASE(this->lpFont);

	SAFE_RELEASE(this->lpD3DDevice);
	SAFE_RELEASE(this->lpD3D);

	UnregisterClass(this->szClassName, wndClass.hInstance);

	unregisterAllRenderLayers();

	//OutputDebugString(_T(" - VideoMan destruction completed.\n"));
	
}

HRESULT
VideoMan::InitShader()
{
	HRESULT hr = S_OK;

	//////////////////////////////////////////////////////////////////////////
	// Testing Shader
	//////////////////////////////////////////////////////////////////////////
	hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("Shaders\\basic.fx"), 0, 0, this->shaderFlags, 0, &this->lpEffect, 0);
	if (hr != D3D_OK)
	{
		hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("..\\Aran\\basic.fx"), 0, 0, this->shaderFlags, 0, &this->lpEffect, 0);
		if (hr != D3D_OK)
		{
			MessageBox(0, _T("Shader file not found."), _T("Error"), MB_OK | MB_ICONERROR);
			hr = -1234;
			goto e_Exit;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Skinned Mesh Shader
	//////////////////////////////////////////////////////////////////////////
	//MessageBox(0, _T("About to call D3DXCreateEffectFromFile()"), _T("Notice"), MB_OK);
	LPD3DXBUFFER lpErrorBuffer = 0;
	hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("Shaders\\vertBlendDynamic.fx"), 0, 0, D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG, 0, &this->lpEffectSkinning, &lpErrorBuffer);
	if (hr != D3D_OK)
	{
		hr = D3DXCreateEffectFromFile(this->lpD3DDevice, _T("..\\Aran\\vertBlendDynamic.fx"), 0, 0, D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG, 0, &this->lpEffectSkinning, &lpErrorBuffer);
		if (hr != D3D_OK)
		{
			MessageBox(0, _T("D3DXCreateEffectFromFile() failed"), _T("Error"), MB_OK | MB_ICONERROR);
			hr = -12345;
			goto e_Exit;
		}
	}
	if (lpErrorBuffer)
	{
		MessageBoxA(0, (char*)lpErrorBuffer->GetBufferPointer(), "D3DXCreateEffectFromFile()", MB_OK | MB_ICONERROR);
		hr = E_FAIL;
		goto e_Exit;
	}

	hr = S_OK;

e_Exit:
	SAFE_RELEASE(lpErrorBuffer);
	return hr;
}

HWND hProgress;

LRESULT CALLBACK
LoadingDialogProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&InitCtrlEx);

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

	return FALSE;
}

HRESULT
VideoMan::InitWindow(TCHAR* szClassName, WNDPROC wndProc, int width, int height)
{
	this->screenWidth = width;
	this->screenHeight = height;
	this->closeNow = FALSE;

	ZeroMemory(this->szClassName, sizeof(this->szClassName));
	size_t len = _tcslen(szClassName);
	_tcscpy_s(this->szClassName, sizeof(this->szClassName)/sizeof(wchar_t), szClassName);

	this->wndClass.style = CS_CLASSDC;
	this->wndClass.lpfnWndProc = wndProc;
	this->wndClass.cbClsExtra = 0;
	this->wndClass.cbWndExtra = 0;
	this->wndClass.hInstance = GetModuleHandle(0);
	this->wndClass.hIcon = 0;
	this->wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	this->wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	this->wndClass.lpszClassName = this->szClassName;
	this->wndClass.lpszMenuName = 0;

	if (!RegisterClass(&(this->wndClass)))
	{
		return E_FAIL;
	}
	int screenResX = GetSystemMetrics(SM_CXFULLSCREEN);
	int screenResY = GetSystemMetrics(SM_CYFULLSCREEN);
	int windowPosX = screenResX / 2 - this->screenWidth / 2;
	int windowPosY = screenResY / 2 - this->screenHeight / 2;
	DWORD windowStyle = 0; //WS_BORDER | WS_SYSMENU;
	
	this->hWnd = CreateWindow(this->szClassName, this->szClassName, WS_POPUPWINDOW, windowPosX, windowPosY, this->screenWidth, this->screenHeight, 0, 0, this->wndClass.hInstance, 0);
	if (!this->hWnd)
	{
		return E_FAIL;
	}


	// Init loading window and show it immediately
	int loadingWndWidth = 400;
	int loadingWndHeight = 100;
	windowPosX = screenResX / 2 - loadingWndWidth / 2;
	windowPosY = screenResY / 2 - loadingWndHeight / 2;


	/*INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&InitCtrlEx);*/

	//this->hLoadingWnd = CreateWindowEx(WS_EX_DLGMODALFRAME, this->szClassName, _T("Loading"), WS_BORDER, windowPosX, windowPosY, loadingWndWidth, loadingWndHeight, 0, 0, this->wndClass.hInstance, 0);
	//HWND tempHwnd = CreateWindowEx(0, PROGRESS_CLASS, 0, WS_CHILD | WS_VISIBLE, 20, 20, 200, 50, this->hLoadingWnd, 0, 0, 0);
	//ShowWindow(tempHwnd, SW_SHOW);
	
	//this->hLoadingWnd = CreateDialog(this->wndClass.hInstance, MAKEINTRESOURCE(IDD_LOADING_DIALOG), this->hWnd, reinterpret_cast<DLGPROC>(LoadingDialogProc));
	
	//DialogBox(this->wndClass.hInstance, MAKEINTRESOURCE(IDD_LOADING_DIALOG), this->hWnd, reinterpret_cast<DLGPROC>(this->LoadingDialogProc));
	
	return S_OK;
}


void
VideoMan::SetWindowSize(int w, int h)
{
	this->screenWidth = w;
	this->screenHeight = h;
}

void
VideoMan::SetHwnd(HWND hWnd)
{
	this->hWnd = hWnd;
}

HRESULT
VideoMan::InitD3D(BOOL isMultiThreaded)
{
	this->lpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (this->lpD3D == 0)
	{
		return E_FAIL;
	}

	D3DCAPS9 caps;
	this->lpD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// for debugging (PIX?)
	//vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = this->screenWidth;
	d3dpp.BackBufferHeight = this->screenHeight;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.hDeviceWindow = this->hWnd;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Windowed = TRUE;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; //D3DFMT_UNKNOWN;

	HRESULT hr;
	DWORD createFlags = vp;
	if (isMultiThreaded == TRUE)
	{
		createFlags |= D3DCREATE_MULTITHREADED;
	}
	
	//
	// Conditions to utilize PIX tool to debug shader;
	// 1. D3DCREATE_MULTITHREADED flag
	// 2. D3D Retail DLL
	// 3. 'Disable D3DX Analysis' in PIX should be checked
	//
	createFlags |= D3DCREATE_MULTITHREADED;

	hr = this->lpD3D->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->hWnd, createFlags, &d3dpp, &this->lpD3DDevice);

	return hr;
}
HRESULT
VideoMan::Show()
{
	//
	// Show the main window before and remove loading window after.
	//
	ShowCursor( TRUE );
	ShowWindow( this->hWnd, SW_SHOWDEFAULT );
	UpdateWindow( this->hWnd );
	

	DestroyWindow( this->hLoadingWnd );

	return S_OK;
}
HRESULT
VideoMan::StartMainLoop()
{
	this->rendering = TRUE;
	this->leftPattern = FALSE;
	this->rightPattern = FALSE;

	// TRUE is default when one or more light are set
	this->lpD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	//this->lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	this->lpD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);

	// set default ambient color when there is no light
	this->lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(100, 100, 100));

	this->lpD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	// TRUE is default
	//this->lpD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);


	// initial camera position
	this->cameraVector.x = 0.0f;
	this->cameraVector.y = 0.0f;
	this->cameraVector.z = -100.0f;


	//ShowWindow(this->hWnd, SW_SHOW);

	MSG msg;
	do
	{
		if (this->rendering == FALSE)
		{
			if (GetMessage(&msg, 0, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//std::cout << ".";
		}
		else
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (this->closeNow != TRUE)
			{
				if (this->Draw() < 0) // error occurred
					return E_FAIL;
			}
			//std::cout << "!";

			this->pInputMan->ProcessKeyboardInput();

		}

		if (this->closeNow)
		{
			break;
		}
	} while (msg.message != WM_QUIT);

	return S_OK;
}
HWND
VideoMan::GetWindowHandle()
{
	return this->hWnd;
}

HRESULT VideoMan::InitMainCamera()
{
	if (getMainCamera()->up == D3DXVECTOR3(0.0f, 0.0f, 0.0f))
	{
		this->mainCamera.eye = D3DXVECTOR3( 0.0f, 0.0f, -50.0f );
		this->mainCamera.at = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		this->mainCamera.up = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

		this->mainCamera.farClip = 1000.0f;
		this->mainCamera.nearClip = 1.0f;
	}
	return S_OK;
}

void VideoMan::DrawAtEditor( BOOL isReady, BOOL isRunning )
{
	this->isOkayToDestruct = !isReady || !isRunning;
	if (!isReady || !isRunning)
	{
		return;
	}
	// TODO: Keyframed Animation
	// follow the current frame number
	//this->drawCount++;

	this->lpD3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(80, 80, 80), 1.0f, 0);
	this->lpD3DDevice->BeginScene();
	

	// Drawing Job Here
	
	//////////////////////////////////////////////////////////////////////////
	// World - View - Projection
	//////////////////////////////////////////////////////////////////////////
	D3DXMatrixIdentity(&this->matWorld);
	this->lpD3DDevice->SetTransform(D3DTS_WORLD, &this->matWorld);

	//this->SetCamera(0.0f, 0.0f, -50.0f);

	D3DXMatrixLookAtLH(
		&this->matView,
		&this->mainCamera.eye,						// the camera position
		&this->mainCamera.at,						// the look-at position
		&this->mainCamera.up						// the up direction
		);
	this->lpD3DDevice->SetTransform(D3DTS_VIEW, &this->matView);
	
	D3DXMatrixPerspectiveFovLH(
		&this->matProjection,
		D3DXToRadian(45),
		(float)this->screenWidth / (float)this->screenHeight,
		this->mainCamera.nearClip,
		this->mainCamera.farClip
		);
	this->lpD3DDevice->SetTransform(D3DTS_PROJECTION, &this->matProjection);


	D3DXMATRIX matTranslation, matRotation, matScaling;
	
	
	if (this->pDrawingModel == 0)
	{
		// Sample model displaying
		D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f, 10.0f);
		D3DXMatrixScaling(&matScaling, 0.1f, 0.1f, 0.1f);
		this->RenderModel(&this->mrMan, &(matScaling * matTranslation));
	}
	else
	{
		D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f, 10.0f);
		D3DXMatrixScaling(&matScaling, 1.0f, 1.0f, 1.0f);
		this->RenderModel(this->pDrawingModel, &(matScaling * matTranslation));
		this->pDrawingModel->AdvanceTime( 0.1f );
	}
	//this->RenderModel(&this->mrMan, &(matScaling * matTranslation));
	//this->RenderModel(&this->mr1);

	this->lpAnimationController->AdvanceTime(0.1f, 0);

	this->lpD3DDevice->EndScene();

	this->lpD3DDevice->Present(0, 0, 0, 0);

}

int
VideoMan::Draw()
{
	// TODO: Keyframed Animation
	// follow the current frame number
	this->drawCount++;

	if (this->lpD3DDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(40, 40, 40), 1.0f, 0) != D3D_OK)
	{
		MessageBox(0, _T("Clear() error"), _T("Error"), MB_OK | MB_ICONERROR);
		return -2;
	}
	HRESULT hr = this->lpD3DDevice->BeginScene();
	if (FAILED(hr))
	{
		std::cout << "BeginScene() failed" << std::endl;
		return -1;
	}


	std::list<RenderLayer*>::iterator it = m_renderLayers.begin();
	std::list<RenderLayer*>::iterator itEnd = m_renderLayers.end();

	for ( ; it != itEnd; ++it )
	{
		if ((*it)->getVisible())
		{
			(*it)->render();
		}
	}


	//D3DXMATRIX matTranslation, matRotation, matScaling;



	this->lpAnimationController->AdvanceTime( 0.1f, 0 );
	this->lpD3DDevice->EndScene();


	
	// reset rotation
	if (this->GetInputMan()->IsRClicked())
	{
		D3DXMatrixIdentity(&this->modelArcBallRotation);
	}

	if (this->GetInputMan()->IsDragging() == FALSE)
	{
		if (this->GetInputMan()->IsClicked())
		{
			//OutputDebugString(_T("Drag Start"));
			this->GetInputMan()->SetDragging(TRUE);
			this->modelArcBallRotationLast = this->modelArcBallRotation;

		}
	}
	else
	{
		if (this->GetInputMan()->IsClicked())
		{
			//OutputDebugString(_T("Dragging...\n"));
			Point2Int downPos = this->GetInputMan()->GetMouseDownPos();
			Point2Int curPos = this->GetInputMan()->GetMouseCurPos();
			Point2Int diffPos;

			diffPos.x = downPos.x - curPos.x;
			diffPos.y = downPos.y - curPos.y;

			D3DXQUATERNION quat;
			D3DXVECTOR3 pV;
			pV.x = (float)diffPos.y;
			pV.y = (float)diffPos.x;
			pV.z = 0.0f;
			float angle = sqrtf((float)(diffPos.x*diffPos.x + diffPos.y*diffPos.y));
			D3DXQuaternionRotationAxis(&quat, &pV, D3DXToRadian(angle));

			D3DXMATRIX mRot;
			D3DXMatrixRotationQuaternion(&mRot, &quat);

			this->modelArcBallRotation = this->modelArcBallRotationLast * mRot;
		}
		else
		{
			//OutputDebugString(_T("Drag End"));
			this->GetInputMan()->SetDragging(FALSE);
			
		}
	}

	if (this->closeNow == FALSE)
	{
		hr = this->lpD3DDevice->Present(0, 0, 0, 0);

		if (FAILED(hr))
		{
			std::cout << "Present() failed" << std::endl;
			return -2;
		}
	}

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

	/*this->mainCamera.at.x = pCamChunk->targetPos.x;
	this->mainCamera.at.y = pCamChunk->targetPos.y;
	this->mainCamera.at.z = pCamChunk->targetPos.z;*/

	D3DXVECTOR3 lookAtVector( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 upVector( 0.0f, 1.0f, 0.0f );
	
	D3DXMATRIX upVectorRotX, upVectorRotY, upVectorRotZ, upVectorRot;
	/*
	D3DXMatrixRotationYawPitchRoll( &upVectorRot, pCamChunk->rot.y, pCamChunk->rot.x, pCamChunk->rot.z );
	D3DXMatrixRotationX( &upVectorRotX, -pCamChunk->rot.x );
	D3DXMatrixRotationY( &upVectorRotY, -pCamChunk->rot.y );
	D3DXMatrixRotationZ( &upVectorRotZ, pCamChunk->rot.z );
	upVectorRot = upVectorRotX * upVectorRotY * upVectorRotZ;
	D3DXVec3TransformCoord( &upVector, &upVector, &upVectorRot );*/

	D3DXQUATERNION quat( pCamChunk->rot.x, pCamChunk->rot.y, pCamChunk->rot.z, pCamChunk->rot.w );
	D3DXMatrixRotationQuaternion( &upVectorRot, &quat );
	
	D3DXVECTOR4 upv;
	D3DXVec3Transform( &upv, &upVector, &upVectorRot );

	D3DXVec3TransformNormal( &upVector, &upVector, &upVectorRot );
	D3DXVec3TransformNormal( &lookAtVector, &lookAtVector, &upVectorRot );

	/*this->mainCamera.at = lookAtVector;
	this->mainCamera.up = upVector;*/

	this->mainCamera.up = D3DXVECTOR3( pCamChunk->upVector.x, pCamChunk->upVector.y, pCamChunk->upVector.z );
	this->mainCamera.at = D3DXVECTOR3( pCamChunk->lookAtVector.x, pCamChunk->lookAtVector.y, pCamChunk->lookAtVector.z );
	

	//this->mainCamera.up.x = 0.0f;
	//this->mainCamera.up.y = 0.0f;
	//this->mainCamera.up.z = -1.0f;

	this->mainCamera.farClip = pCamChunk->farClip;
	this->mainCamera.nearClip = pCamChunk->nearClip;

	return S_OK;
}

HRESULT VideoMan::SetCamera( ArnCamera& arnCam )
{
	/*D3DXMATRIX* localTf = reinterpret_cast<D3DXMATRIX*>(arnCam.getOb().hdr->localTf);
	D3DXVECTOR3 scaleVec, transVec;
	D3DXQUATERNION rotQuat;
	D3DXMatrixDecompose(&scaleVec, &rotQuat, &transVec, localTf);
	
	D3DXMATRIX rotMat;
	D3DXMatrixRotationQuaternion(&rotMat, &rotQuat);
	D3DXVECTOR3 lookAt3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR4 lookAt4;
	D3DXVec3Transform(&lookAt4, &lookAt3, &rotMat);
	mainCamera.eye = transVec;
	mainCamera.at.x = lookAt4.x;
	mainCamera.at.y = lookAt4.y;
	mainCamera.at.z = lookAt4.z;

	mainCamera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	mainCamera.nearClip = arnCam.getOb().hdr->clipStart;
	mainCamera.farClip = arnCam.getOb().hdr->clipEnd;
	mainCamera.angle = D3DXToRadian(arnCam.getOb().hdr->angle);*/
	return S_OK;
}
HRESULT
VideoMan::RenderModel(const ModelReader* pMR, const D3DXMATRIX* worldTransformMatrix /* = 0 */)
{
	if (!pMR->IsInitialized())
		return E_FAIL;

	EXPORT_VERSION ev = pMR->GetExportVersion();
	if (ev == EV_ARN10 || ev == EV_ARN11)
	{
		return RenderModel1(pMR, worldTransformMatrix);
	}
	else if (ev == EV_ARN20)
	{
		return RenderModel2(pMR, worldTransformMatrix);
	}
	return E_FAIL;
}
void
VideoMan::Close()
{
	this->closeNow = TRUE;
}
BOOL
VideoMan::PauseMainLoop()
{
	if (this->rendering == TRUE)
	{
		this->rendering = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
BOOL
VideoMan::ResumeMainLoop()
{
	if (this->rendering == FALSE)
	{
		this->rendering = TRUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


HRESULT
VideoMan::InitTestVertexBuffer()
{
	MY_COLOR_VERTEX data[] = {
		/*{ 2.5f, -3.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
		{ -2.5f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },
		{ 0.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
		{ -2.5f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), },*/
		

		{ -1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
		{ 1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
		{ -1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },

		{ 1.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), },
		{ 1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 255), },
		{ -1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 255), },
	};

	int dataSize = sizeof(MY_COLOR_VERTEX)*6;

	int w = 399;
	int h = 399;
	MY_COLOR_VERTEX* dynamicData = new MY_COLOR_VERTEX[6*w*h]; // total 699*699 tiles
	int dynamicDataSize = sizeof(MY_COLOR_VERTEX)*6*w*h;


	int i, j, k;
	for (i = -(w+1)/2+1; i < (w+1)/2; i++)
	{
		for (j = -(h+1)/2+1; j < (h+1)/2; j++)
		{
			for (k = 0; k < 6; k++)
			{
				int index = k + (j+(h+1)/2-1)*6 + (i+(w+1)/2-1)*h*6;
				MY_COLOR_VERTEX* v = &dynamicData[index];
				v->x = data[k].x + (2.0f*i);
				v->y = data[k].y + (2.0f*j);
				v->z = data[k].z;
				v->color = data[k].color;
			}
		}
	}


	HRESULT hr = this->lpD3DDevice->CreateVertexBuffer(dynamicDataSize, D3DUSAGE_WRITEONLY, this->MY_COLOR_VERTEX_FVF, D3DPOOL_MANAGED, &this->lpTestVB, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at CreateVertexBuffer()"), _T("Error"), MB_OK);
		return hr;
	}
	hr = this->lpTestVB->Lock(0, 0, &this->pVBVertices, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at Lock()"), _T("Error"), MB_OK);
		return hr;
	}
	memcpy(this->pVBVertices, dynamicData, dynamicDataSize);
	this->lpTestVB->Unlock();


	hr = this->lpD3DDevice->CreateVertexBuffer(dynamicDataSize, D3DUSAGE_WRITEONLY, this->MY_COLOR_VERTEX_FVF, D3DPOOL_MANAGED, &this->lpTestVB2, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at CreateVertexBuffer()"), _T("Error"), MB_OK);
		return hr;
	}
	hr = this->lpTestVB2->Lock(0, 0, &this->pVBVertices, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at Lock()"), _T("Error"), MB_OK);
		return hr;
	}
	memcpy(this->pVBVertices, dynamicData, dynamicDataSize);
	this->lpTestVB2->Unlock();


	delete [] dynamicData;
	dynamicData = 0;


	return hr;
}
HRESULT
VideoMan::InitBoxVertexBuffer()
{
	ARN_VDD data[] = {
		{ -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255),     { 0.0f, 0.0f, }  },
		{  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255),     { 1.0f, 0.0f, }  },
		{ -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255),     { 0.0f, 1.0f, }  },

		{  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB( 50,  50,  50),     { 1.0f, 0.0f, }  },
		{  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB( 50,  50,  50),     { 1.0f, 1.0f, }  },
		{ -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, D3DCOLOR_XRGB( 50,  50,  50),     { 0.0f, 1.0f, }  },

		{ -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(255,   0,   0),     { 0.0f, 1.0f, }  },
		{  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(  0, 255,   0),     { 1.0f, 0.0f, }  },
		{ -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(  0,   0, 255),     { 0.0f, 0.0f, }  },

		{ -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(  0, 255, 255),     { 0.0f, 1.0f, }  },
		{  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(255,   0, 255),     { 1.0f, 1.0f, }  },
		{  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, D3DCOLOR_XRGB(255, 255, 255),     { 1.0f, 0.0f, }  },

		{  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(200,  10,  10),     { 0.0f, 0.0f, }  },
		{  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(100, 100, 100),     { 1.0f, 0.0f, }  },
		{  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(100, 100, 100),     { 0.0f, 1.0f, }  },

		{  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(100, 100, 100),     { 0.0f, 0.0f, }  },
		{  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(100, 100, 100),     { 1.0f, 0.0f, }  },
		{  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(100, 100, 100),     { 0.0f, 1.0f, }  },

		{ -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 200),     { 0.0f, 1.0f, }  },
		{ -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 200),     { 1.0f, 1.0f, }  },
		{ -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 200),     { 1.0f, 0.0f, }  }, 

		{ -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 255),     { 0.0f, 1.0f, }  },
		{ -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 255),     { 1.0f, 1.0f, }  },
		{ -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, D3DCOLOR_XRGB(  0,   0, 255),     { 1.0f, 0.0f, }  },

		{ -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 0.0f, }  },
		{  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 1.0f, 0.0f, }  },
		{ -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 1.0f, }  },

		{  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 0.0f, }  },
		{  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 1.0f, 0.0f, }  },
		{ -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 1.0f, }  },

		{ -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 0.0f, }  },
		{  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 1.0f, 1.0f, }  },
		{ -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 0.0f, 1.0f, }  },

		{ -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(255, 255,   0),     { 0.0f, 0.0f, }  },
		{  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(255, 255,   0),     { 1.0f, 0.0f, }  },
		{  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, D3DCOLOR_XRGB(200, 200, 200),     { 1.0f, 1.0f, }  },
	};



	int dataSize = sizeof(ARN_VDD) * 12 * 3;


	HRESULT hr = this->lpD3DDevice->CreateVertexBuffer(dataSize, D3DUSAGE_WRITEONLY, ARN_VDD::ARN_VDD_FVF, D3DPOOL_MANAGED, &this->lpBoxVB, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at CreateVertexBuffer()"), _T("Error"), MB_OK);
		return hr;
	}
	hr = this->lpBoxVB->Lock(0, 0, &this->pVBVertices, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at Lock()"), _T("Error"), MB_OK);
		return hr;
	}
	memcpy(this->pVBVertices, data, dataSize);
	this->lpBoxVB->Unlock();

	return hr;
}

HRESULT
VideoMan::InitModelsAtEditor()
{
	//
	// This is called when the Engine is used by editor module
	//
	//this->mr1.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd, _T("box1.arn"), this->lpAnimationController );
	
	this->mrMan.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd, _T("man.arn"), this->lpAnimationController );
	return S_OK;
}

HRESULT
VideoMan::InitModels()
{
	this->mrMan.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
		_T("man.arn"), 0 );

	//this->mrDungeon.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("dun.arn"), 0 );

	//this->mr1.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("box1.arn"), 0 ); // local AC test case

	//this->mr2.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("box2.arn"), 0 );

	//this->mr3.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("box3.arn"), 0 );

	//this->mrRocky.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("rocky.arn"), 0 );

	//this->mrHouse.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	//	_T("bighouse.arn"), this->lpAnimationController );
	

	//
	// 10 MB ~ 20 MB ARN format (experimental)
	//
	/*this->highpoly.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
		_T("highpoly.arn"), 0 );*/

	/*this->gamebryo.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	_T("gb.arn"), 0 );

	this->middlesnake.Initialize( this->lpD3DDevice, ARN_VDD::ARN_VDD_FVF, this->hLoadingWnd,
	_T("middlesnake.arn"), 0 );*/


	V_OKAY(this->lpAnimationController->ResetTime());
	V_OKAY(this->lpAnimationController->AdvanceTime(0.0f, 0));
	
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
VideoMan::ChangeInTestVB(D3DCOLOR color)
{
	HRESULT hr = this->lpTestVB->Lock(0, 0, &this->pVBVertices, 0);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Error at Lock()"), _T("Error"), MB_OK);
		return;
	}
	MY_COLOR_VERTEX* data = (MY_COLOR_VERTEX*)this->pVBVertices;
	data[0].color = color;
	this->lpTestVB->Unlock();
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
	ZeroMemory(&this->defaultLight, sizeof(D3DLIGHT9));
	this->defaultLight.Type = D3DLIGHT_DIRECTIONAL;
	this->defaultLight.Diffuse.r = 0.5f;
	this->defaultLight.Diffuse.g = 0.5f;
	this->defaultLight.Diffuse.b = 0.5f;
	this->defaultLight.Diffuse.a = 1.0f;
	D3DVECTOR dir = { 0.0f, 0.0f, 1.0f };
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

	D3DVECTOR pos = { 0.0f, 0.0f, -50.0f };
	this->pointLight.Position = pos;
	//this->pointLight.Direction = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	this->lpD3DDevice->SetLight(0, &this->defaultLight);
	//this->lpD3DDevice->SetLight(0, &this->pointLight);
	this->lpD3DDevice->LightEnable(0, TRUE);

	return S_OK;
}

HRESULT
VideoMan::InitMaterial()
{
	ZeroMemory(&this->defaultMaterial, sizeof(D3DMATERIAL9));
	ZeroMemory(&this->rgbMaterial, sizeof(D3DXMATERIAL) * 3);
	SetSimpleColoredMaterial(&this->defaultMaterial, D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[0].MatD3D, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[1].MatD3D, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));
	SetSimpleColoredMaterial(&this->rgbMaterial[2].MatD3D, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f));
	return S_OK;
}
void SetSimpleColoredMaterial(D3DMATERIAL9* material, D3DXCOLOR color)
{
	material->Ambient.r = material->Diffuse.r = material->Emissive.r = material->Specular.r = color.r;
	material->Ambient.g = material->Diffuse.g = material->Emissive.g = material->Specular.g = color.g;
	material->Ambient.b = material->Diffuse.b = material->Emissive.b = material->Specular.b = color.b;
	material->Power = 1.0f;
}
HRESULT
VideoMan::InitTexture()
{
	HRESULT hr;
	hr = D3DXCreateTextureFromFile(this->lpD3DDevice, _T("tex1.png"), &this->lpTex1);
	if (hr != D3D_OK)
	{
		hr = D3DXCreateTextureFromFile(this->lpD3DDevice, _T("..\\Aran\\tex1.png"), &this->lpTex1);
		if (hr != D3D_OK)
		{
			return -1;
		}
	}
	return hr;
}

HRESULT
VideoMan::TurnModelLightOn(const ModelReader *pMR, D3DXMATRIX* worldTransformMatrix)
{
	int i;
	int modelLightCount = pMR->GetLightCount();

	for (i = 0; i < modelLightCount; i++)
	{
		D3DLIGHT9 light = pMR->GetLight(i);

		//light.Phi = D3DXToRadian(100.0f);
		//light.Theta = D3DXToRadian(20.0f);
		light.Falloff = 1.0f;
		light.Ambient.r = light.Diffuse.r / 2;
		light.Ambient.g = light.Diffuse.g / 2;
		light.Ambient.b = light.Diffuse.b / 2;
		light.Ambient.a = light.Diffuse.a / 2;
		//light.Position.z = -20.0f;

		if (worldTransformMatrix != 0)
		{
			D3DXVECTOR3 lightPos(light.Position), lightDir(light.Direction); // original
			D3DXVECTOR4 lightPosition, lightDirection; // transformed
			D3DXVECTOR3 scaling, translation;
			D3DXQUATERNION rotation;

			D3DXMatrixDecompose(&scaling, &rotation, &translation, worldTransformMatrix);
			
			lightPosition.x = light.Position.x + translation.x;
			lightPosition.y = light.Position.y + translation.y;
			lightPosition.z = light.Position.z + translation.z;

			//D3DXVec3Transform(&lightPosition, &lightPos, worldTransformMatrix);
			D3DXMATRIX matRot;
			D3DXMatrixRotationQuaternion(&matRot, &rotation);
			D3DXVec3Transform(&lightDirection, &lightDir, &matRot);

			light.Position.x = lightPosition.x;
			light.Position.y = lightPosition.y;
			light.Position.z = lightPosition.z;

			light.Direction.x = lightDirection.x;
			light.Direction.y = lightDirection.y;
			light.Direction.z = lightDirection.z;
		}

		this->lpD3DDevice->SetLight(this->totalLightCount, &light);
		this->lpD3DDevice->LightEnable(this->totalLightCount, TRUE);
		
		this->totalLightCount++;
	}

	return S_OK;
}

HRESULT
VideoMan::RenderModel1(const ModelReader *pMR, const D3DXMATRIX* worldTransformMatrix)
{
	//
	// SHOULD BE CALLED BETWEEN BeginScene() & EndScene()
	//
	HRESULT hr = S_OK;

	if (pMR->GetExportVersion() != EV_ARN10 && pMR->GetExportVersion() != EV_ARN11)
	{
		MessageBox(0, _T("RenderModel() call corrupted."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	int i, j, vertexOffset = 0;

	int meshCount = pMR->GetNotIndMeshCount();

	this->lpD3DDevice->SetFVF(pMR->GetFVF());
	this->lpD3DDevice->SetStreamSource(0, pMR->GetVB(), 0, sizeof(ARN_VDD));

	// An ARN model is stored with 'front view' aspect.
	// 'Top view' aspect is preferable to make consistant x- and y-axis orientation
	// between 3ds Max and Aran Rendering window.
	// To make this work, we define -90 degree x-axis rotation transform matrix.
	D3DXMATRIX matRotX90;
	D3DXMatrixRotationX(&matRotX90, D3DXToRadian(-90.0f));

	D3DXMATRIX matFinalTransform;

	for (i = 0; i < meshCount; i++)
	{
		// TODO: keyframed animation
		const D3DXMATRIX* pAnimMat = pMR->GetAnimMatControlledByAC(i);
		matFinalTransform = (*pAnimMat) * matRotX90;
		
		if (worldTransformMatrix != 0)
		{
			matFinalTransform = matFinalTransform * (*worldTransformMatrix);
		}
		this->lpD3DDevice->SetTransform(D3DTS_WORLD, &matFinalTransform);

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		// get the number of materials of this mesh
		int materialCount = pMR->GetMaterialCount(i);
		int materialOffset = 0;
		// find for material offset of this mesh (accumulation)
		for (j = 0; j < i; j++)
		{
			materialOffset += pMR->GetMaterialCount(j);
		}

		// draw primitives with material and texture
		D3DMATERIAL9* matRef = 0;
		LPDIRECT3DTEXTURE9 texRef = 0;


		if (materialCount > 0)
		{
			for (j = 0; j < materialCount; j++)
			{
				// get the real material(w/ texture) offset
				int ref = materialOffset + j;

				if (ref < 0)
				{
					matRef = &this->defaultMaterial;
					texRef = 0;
				}
				else
				{
					matRef = (D3DMATERIAL9*)pMR->GetMaterial(ref);
					// if there is texture information...
					if (pMR->GetTexture(ref) != 0)
					{
						// ..set it
						texRef = pMR->GetTexture(ref);
					}
					else
					{
						texRef = 0;
					}

				}
				this->lpD3DDevice->SetMaterial(matRef);
				this->lpD3DDevice->SetTexture(0, texRef);

				//
				// calculate current mesh's face(triangle) count
				//
				// i; current mesh index
				// j; current mesh's current material index
				//
				int faceStartOffset = pMR->GetMaterialReferenceFast(i, j);
				int faceCount = -1;
				if (j == pMR->GetMaterialCount(i) - 1)
				{
					// if this material is last one...
					faceCount = pMR->GetFaceCount(i) - pMR->GetMaterialReferenceFast(i, j);
				}
				else
				{
					// ...or not
					faceCount = pMR->GetMaterialReferenceFast(i, j+1) - faceStartOffset;
				}
				this->lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, vertexOffset + faceStartOffset*3, faceCount);
			}
		}
		else if (materialCount == 0) // use default material and texture when there is no material defined explicitly
		{
			matRef = &this->defaultMaterial;
			this->lpD3DDevice->SetMaterial(matRef);
			this->lpD3DDevice->SetTexture(0, 0);
			int faceStartOffset = pMR->GetMaterialReferenceFast(i, 0); // ..Fast
			int faceCount = pMR->GetFaceCount(i) - pMR->GetMaterialReferenceFast(i, 0); //..Fast this material is last one, always.

			this->lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, vertexOffset + faceStartOffset*3, faceCount);
		}
		else
		{
			// critical error! materialCount < 0!!!
			MessageBox(0, _T("materialCount < 0 error."), _T("Error"), MB_OK);
			return E_FAIL;
		}
		// increase vertex data offset by current mesh's vertices count
		vertexOffset += pMR->GetFaceCount(i) * 3;
	}



	return hr;
}

HRESULT
VideoMan::RenderModel2(const ModelReader *pMR, const D3DXMATRIX* worldTransformMatrix)
{
	if (pMR->GetExportVersion() != EV_ARN20)
	{
		MessageBox(0, _T("RenderModel2() call corrupted."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	int i, j, k;

	int meshCount = pMR->GetIndMeshCount();
	int accum = 0;

	std::vector<int> skinnedMeshTextureIndex;

	// An ARN model is stored with 'front view' aspect.
	// 'Top view' aspect is preferable to make consistant x- and y-axis orientation
	// between 3ds Max and Aran Rendering window.
	// To make this work, we define -90 degree x-axis rotation transform matrix.
	D3DXMATRIX matRotX90;
	D3DXMatrixRotationX(&matRotX90, D3DXToRadian(-90.0f));

	D3DXMATRIX matGeneralMeshFinalTransform;
	
	for (i = 0; i < meshCount; i++)
	{
		int matCount = pMR->GetMaterialCount(i);
		
		if (pMR->GetSkeletonIndexByMeshIndex(i) < 0)
		{
			// General mesh case
			const D3DXMATRIX* pAnimMat = pMR->GetAnimMatControlledByAC(i);
			matGeneralMeshFinalTransform = (*pAnimMat) * matRotX90;

			if (worldTransformMatrix != 0)
			{
				matGeneralMeshFinalTransform = matGeneralMeshFinalTransform * (*worldTransformMatrix);
			}
			this->lpD3DDevice->SetTransform(D3DTS_WORLD, &matGeneralMeshFinalTransform);
			
			for (j = 0; j < matCount; j++)
			{
				const D3DMATERIAL9* matPointer = pMR->GetMaterial(j + accum);
				this->lpD3DDevice->SetMaterial(matPointer);

				LPDIRECT3DTEXTURE9 texPointer = pMR->GetTexture(j + accum);
				this->lpD3DDevice->SetTexture(0, texPointer);

				pMR->GetMeshPointer(i)->DrawSubset(j);
			}
		}
		else
		{
			// Skinned mesh case
			// vertex transform will be applied via vertex shader (see below)
			skinnedMeshTextureIndex.push_back(accum);
		}

		accum += matCount;
	}

	//////////////////////////////////////////////////////////////////////////
	// Skinned Mesh Rendering
	//////////////////////////////////////////////////////////////////////////
	D3DXMATRIX* offsetTemp = 0;
	D3DXMATRIX offset, offsetInverse;
	const D3DXMATRIX* combinedTemp = 0;

	static D3DXMATRIX finalTransforms[128];
	ZeroMemory(finalTransforms, sizeof(finalTransforms));

	D3DXHANDLE hTech            = this->lpEffectSkinning->GetTechniqueByName("VertexBlendingTech");

	D3DXHANDLE hWorldViewProj   = this->lpEffectSkinning->GetParameterByName(0, "WorldViewProj");
	D3DXHANDLE hFinalTransforms = this->lpEffectSkinning->GetParameterByName(0, "FinalTransforms");
	D3DXHANDLE hTex             = this->lpEffectSkinning->GetParameterByName(0, "Tex");
	D3DXHANDLE hNumVertInflu	= this->lpEffectSkinning->GetParameterByName(0, "NumVertInfluences");

	D3DXMATRIX matWorldViewProj = this->matWorld * this->matView * this->matProjection;

	int numVertInfluences = 3;
	V_OKAY( this->lpEffectSkinning->SetInt( hNumVertInflu, numVertInfluences ) );
	V_OKAY( this->lpEffectSkinning->SetMatrix( hWorldViewProj, &matWorldViewProj ) );

	V_OKAY( this->lpEffectSkinning->SetTechnique( hTech ) );
	UINT numPasses;
	this->lpEffectSkinning->Begin(&numPasses, 0);
	size_t boneCount;
	for (i = 0; i < (int)numPasses; i++)
	{
		for (j = 0; j < (int)pMR->GetSkeletonNodeSize(); j++)
		{
			//int associatedMeshIndex = pMR->GetMeshIndexBySkeletonIndex(j);
			//pMR->UpdateBoneCombinedMatrixByMeshIndex(associatedMeshIndex);

			boneCount = pMR->GetSkeletonNodePointer(j)->bones.size();
			for(k = 0; k < (int)boneCount; k++)
			{
				const char* boneName = pMR->GetSkeletonNodePointer(j)->bones[k].nameFixed;
				
				//combinedTemp = pMR->GetCombinedMatrixByBoneName(boneName); // hierarchically computed at runtime
				combinedTemp = pMR->GetTransformationMatrixByBoneName(boneName); // precomputed transforms
				offset = pMR->GetSkeletonNodePointer(j)->bones[k].offsetMatrix;
				D3DXMatrixInverse(&offsetInverse, 0, &offset);

				finalTransforms[k] = offsetInverse * (*combinedTemp) * matRotX90;
				if (worldTransformMatrix != 0)
				{
					finalTransforms[k] = finalTransforms[k] * (*worldTransformMatrix);
				}
			}
			this->lpEffectSkinning->SetMatrixArray(hFinalTransforms, &finalTransforms[0], (UINT)boneCount);

			this->lpEffectSkinning->BeginPass(i);
			this->lpEffectSkinning->CommitChanges();

			//const D3DMATERIAL9* matPointer = pMR->GetMaterial(j + accum);
			//this->lpD3DDevice->SetMaterial(matPointer);

			LPDIRECT3DTEXTURE9 texPointer = pMR->GetTexture(skinnedMeshTextureIndex[j]);
			this->lpD3DDevice->SetTexture(0, texPointer);

			//this->lpD3DDevice->SetTexture(0, pMR->GetTexture(0)); // TODO: Right texture in skinned mesh

			pMR->GetSkinnedMeshPointer(j)->DrawSubset(0); // TODO: Multiple subset in skinned mesh

			this->lpEffectSkinning->EndPass();
		}

	}
	this->lpEffectSkinning->End();

	return S_OK;
}
HRESULT
VideoMan::SetCurrentFrameIndex(int idx)
{
	ASSERTCHECK(idx >= 0);
	this->curFrameIndex = idx;
	return S_OK;
}

HRESULT VideoMan::InitAnimationController()
{
	///////////////////////////////////////////////////////////////////////////
	// ANIMATION CONTROLLER CREATION
	///////////////////////////////////////////////////////////////////////////

	V_OKAY(D3DXCreateAnimationController(100, 100, 100, 300, &this->lpAnimationController));

	return S_OK;
}

HRESULT VideoMan::InitCustomMesh()
{
	HRESULT hr = 0;
	hr = D3DXCreateMeshFVF(12, 24, D3DXMESH_MANAGED, MY_CUSTOM_MESH_VERTEX::MY_CUSTOM_MESH_VERTEX_FVF, this->lpD3DDevice, &this->lpCustomMesh);
	if (FAILED(hr))
	{
		MessageBox(0, _T("Custom Mesh creation failed."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	MY_CUSTOM_MESH_VERTEX* v = 0;
	this->lpCustomMesh->LockVertexBuffer(0, (void**)&v);

	// fill in the front face MY_CUSTOM_MESH_VERTEX data
	v[0] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[1] = MY_CUSTOM_MESH_VERTEX(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[2] = MY_CUSTOM_MESH_VERTEX( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	v[3] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	// fill in the back face MY_CUSTOM_MESH_VERTEX data
	v[4] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[5] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = MY_CUSTOM_MESH_VERTEX( 1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[7] = MY_CUSTOM_MESH_VERTEX(-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// fill in the top face MY_CUSTOM_MESH_VERTEX data
	v[8]  = MY_CUSTOM_MESH_VERTEX(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[9]  = MY_CUSTOM_MESH_VERTEX(-1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[10] = MY_CUSTOM_MESH_VERTEX( 1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	v[11] = MY_CUSTOM_MESH_VERTEX( 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	// fill in the bottom face MY_CUSTOM_MESH_VERTEX data
	v[12] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[13] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[15] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// fill in the left face MY_CUSTOM_MESH_VERTEX data
	v[16] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[17] = MY_CUSTOM_MESH_VERTEX(-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[18] = MY_CUSTOM_MESH_VERTEX(-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[19] = MY_CUSTOM_MESH_VERTEX(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// fill in the right face MY_CUSTOM_MESH_VERTEX data
	v[20] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[21] = MY_CUSTOM_MESH_VERTEX( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[22] = MY_CUSTOM_MESH_VERTEX( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[23] = MY_CUSTOM_MESH_VERTEX( 1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	this->lpCustomMesh->UnlockVertexBuffer();

	WORD* ind = 0;
	this->lpCustomMesh->LockIndexBuffer(0, (void**)&ind);
	
	// fill in the front face index data
	ind[0] = 0; ind[1] = 1; ind[2] = 2;
	ind[3] = 0; ind[4] = 2; ind[5] = 3;

	// fill in the back face index data
	ind[6] = 4; ind[7]  = 5; ind[8]  = 6;
	ind[9] = 4; ind[10] = 6; ind[11] = 7;

	// fill in the top face index data
	ind[12] = 8; ind[13] =  9; ind[14] = 10;
	ind[15] = 8; ind[16] = 10; ind[17] = 11;

	// fill in the bottom face index data
	ind[18] = 12; ind[19] = 13; ind[20] = 14;
	ind[21] = 12; ind[22] = 14; ind[23] = 15;

	// fill in the left face index data
	ind[24] = 16; ind[25] = 17; ind[26] = 18;
	ind[27] = 16; ind[28] = 18; ind[29] = 19;

	// fill in the right face index data
	ind[30] = 20; ind[31] = 21; ind[32] = 22;
	ind[33] = 20; ind[34] = 22; ind[35] = 23;

	this->lpCustomMesh->UnlockIndexBuffer();

	// 日本語で書きます
	
	DWORD* attributeBuffer = 0;
	this->lpCustomMesh->LockAttributeBuffer(0, &attributeBuffer);
	int i;

	attributeBuffer[ 0] = 1;
	attributeBuffer[ 1] = 0;
	attributeBuffer[ 2] = 1;
	attributeBuffer[ 3] = 0;
	attributeBuffer[ 4] = 0;
	attributeBuffer[ 5] = 1;
	attributeBuffer[ 6] = 0;
	attributeBuffer[ 7] = 1;
	attributeBuffer[ 8] = 2;
	attributeBuffer[ 9] = 2;
	attributeBuffer[10] = 1;
	attributeBuffer[11] = 1;

	this->lpCustomMesh->UnlockAttributeBuffer();

	

	/*hr = this->lpCustomMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE, &adjacencyBuffer[0], &newAdjBuffer[0], 0, 0);
	if (hr != D3D_OK)
	{
		MessageBox(0, _T("OptimizeInplace() error"), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}*/



	//delete [] adjacencyBuffer;
	//adjacencyBuffer = 0;

	//ZeroMemory(&this->meshContainer, sizeof(D3DXMESHCONTAINER));
	//this->meshContainer.MeshData.pMesh = this->lpCustomMesh;
	//this->meshContainer.MeshData.Type = D3DXMESHTYPE_MESH;
	//
	//this->meshContainer.Name = "Simple Box";
	//this->meshContainer.NumMaterials = 3;
	//this->meshContainer.pAdjacency = adjacencyBuffer;
	//this->meshContainer.pEffects = 0;
	//this->meshContainer.pMaterials = this->rgbMaterial;
	//this->meshContainer.pNextMeshContainer = 0;
	//this->meshContainer.pSkinInfo = 

	this->frame1.Name = "Custom Bone 1";
	this->frame1.pFrameFirstChild = &this->frame2;
	this->frame1.pFrameSibling = 0;
	this->frame1.pMeshContainer = 0; //&this->meshContainer;
	D3DXMatrixIdentity(&this->frame1.TransformationMatrix);

	this->frame2.Name = "Custom Bone 2";
	this->frame2.pFrameFirstChild = 0;
	this->frame2.pFrameSibling = 0;
	this->frame2.pMeshContainer = 0;
	D3DXMatrixIdentity(&this->frame2.TransformationMatrix);


	//D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

	//MessageBox(0, _T("About to call D3DXCreateSkinInfoFVF()"), _T("Notice"), MB_OK);
	if (FAILED(D3DXCreateSkinInfoFVF(24, MY_CUSTOM_MESH_VERTEX::MY_CUSTOM_MESH_VERTEX_FVF, 2, &this->lpSkinInfo)))
	{
		MessageBox(0, _T("D3DXCreateSkinInfoFVF() failed."), _T("Error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	D3DXMATRIX boneMat1, boneMat2;
	D3DXMatrixIdentity(&boneMat1);
	D3DXMatrixIdentity(&boneMat2);



	V_OKAY(this->lpSkinInfo->SetBoneName(0, "Custom Bone 1"));
	V_OKAY(this->lpSkinInfo->SetBoneName(1, "Custom Bone 2"));
	V_OKAY(this->lpSkinInfo->SetBoneOffsetMatrix(0, &boneMat1));
	V_OKAY(this->lpSkinInfo->SetBoneOffsetMatrix(1, &boneMat2));
	
	DWORD* bone1Vertices = 0;
	DWORD* bone2Vertices = 0;
	float* bone1Weights = 0;
	float* bone2Weights = 0;
	try
	{
		bone1Vertices = new DWORD[24];
		bone2Vertices = new DWORD[12];
		bone1Weights = new float[24];
		bone2Weights = new float[12];
	}
	catch (...)
	{
		SAFE_DELETE_ARRAY(bone1Vertices);
		SAFE_DELETE_ARRAY(bone2Vertices);
		SAFE_DELETE_ARRAY(bone1Weights);
		SAFE_DELETE_ARRAY(bone2Weights);
		throw new std::runtime_error("Memory allocation failure");
	}
	
	for (i = 0; i < 24; i++)
	{
		bone1Vertices[i] = i;
		if (i >= 12)
			bone1Weights[i] = 0.5f;
		else
			bone1Weights[i] = 1.0f;
	}
	for (i = 0; i < 12; i++)
	{
		bone2Vertices[i] = i+12;
		bone2Weights[i] = 0.5f;
	}

	V_OKAY(this->lpSkinInfo->SetBoneInfluence(0, 24, bone1Vertices, bone1Weights));
	V_OKAY(this->lpSkinInfo->SetBoneInfluence(1, 12, bone2Vertices, bone2Weights));



	DWORD maxVertexInfluence, numBoneCombinations;
	LPD3DXBUFFER boneCombinations;
	
	int customMeshVerticesCount = this->lpCustomMesh->GetNumVertices();

	D3DVERTEXELEMENT9 tempDeclMesh[MAX_FVF_DECL_SIZE];
	ZeroMemory(tempDeclMesh, sizeof(tempDeclMesh));
	this->lpCustomMesh->GetDeclaration(tempDeclMesh);

	D3DVERTEXELEMENT9 tempDeclSkin[MAX_FVF_DECL_SIZE];
	ZeroMemory(tempDeclSkin, sizeof(tempDeclSkin));
	this->lpSkinInfo->GetDeclaration(tempDeclSkin);

	this->lpSkinInfo->SetDeclaration(tempDeclMesh);

	std::vector<DWORD> adjacencyBuffer, newAdjBuffer;
	int adjSize = this->lpCustomMesh->GetNumFaces() * 3;
	adjacencyBuffer.resize(adjSize);
	newAdjBuffer.resize(adjSize);
	//DWORD* adjacencyBuffer = new DWORD[this->lpCustomMesh->GetNumFaces() * 3];
	if (this->lpCustomMesh->GenerateAdjacency(0.001f, &adjacencyBuffer[0]) != D3D_OK)
	{
		MessageBox(0, _T("GenerateAdjacency() error."), _T("Error"), MB_OK | MB_ICONERROR);
	}
	
	D3DPERF_BeginEvent(0xff00ffff, _T("Convert call"));
	//MessageBox(0, _T("About to call ConvertToIndexedBlenedMesh()"), _T("Notice"), MB_OK);
	hr = this->lpSkinInfo->ConvertToIndexedBlendedMesh(
		this->lpCustomMesh,
		0,
		2,
		&adjacencyBuffer[0],
		&newAdjBuffer[0],
		0,
		0,
		&maxVertexInfluence,
		&numBoneCombinations,
		&boneCombinations,
		&this->lpCustomSkinnedMesh
		);
	
	if (hr != D3D_OK)
	{
		char errorNumber[64];
		_itoa_s(hr, errorNumber, 10);
		MessageBoxA(0, "ConvertToIndexedBlenedMesh() Failed", errorNumber, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	//MessageBox(0, _T("ConvertToIndexedBlenedMesh() Success"), _T("Notice"), MB_OK);
	D3DPERF_EndEvent();

	D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
	ZeroMemory(&declaration, sizeof(declaration));
	this->lpCustomSkinnedMesh->GetDeclaration(declaration);


	D3DXBONECOMBINATION* bcp0 = (D3DXBONECOMBINATION*)boneCombinations->GetBufferPointer();
	D3DXBONECOMBINATION* bcp1 = bcp0+1;
	D3DXBONECOMBINATION* bcp2 = bcp0+2;
	
	for (i = 0; i < (int)this->lpSkinInfo->GetNumBones(); i++)
	{
		const char* boneName = this->lpSkinInfo->GetBoneName(i);

		i++;
		i--;
	}

	boneCombinations->Release();
	boneCombinations = 0;

	DWORD skinnedMeshFVF = this->lpCustomSkinnedMesh->GetFVF();

	DWORD animIndex = 0;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 translation;
	D3DXQUATERNION rotation;

	D3DXMATRIX matOut;
	D3DXVECTOR3 vecOutScale, vecOutTranslation;
	D3DXQUATERNION qOut;

	D3DXKEY_QUATERNION* rotationKeys = 0;
	D3DXKEY_VECTOR3* translationKeys = 0;
	D3DXKEY_VECTOR3* scaleKeys = 0;


	V_OKAY(D3DXCreateKeyframedAnimationSet("Custom Snake Animation", 1, D3DXPLAY_LOOP, 2, 0, 0, &this->lpDefaultAnimationSet));

	try
	{
		rotationKeys = new D3DXKEY_QUATERNION[4];
		translationKeys = new D3DXKEY_VECTOR3[3];
		scaleKeys = new D3DXKEY_VECTOR3[4];
	}
	catch (std::bad_alloc& ba)
	{
		SAFE_DELETE_ARRAY(rotationKeys);
		SAFE_DELETE_ARRAY(translationKeys);
		SAFE_DELETE_ARRAY(scaleKeys);
		throw ba;
	}
	
	rotationKeys[0].Time = 1.0f;
	D3DXQuaternionRotationAxis(&rotationKeys[0].Value, &D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXToRadian(10));
	rotationKeys[1].Time = 2.0f;
	D3DXQuaternionRotationAxis(&rotationKeys[1].Value, &D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXToRadian(90));
	rotationKeys[2].Time = 3.0f;
	D3DXQuaternionRotationAxis(&rotationKeys[2].Value, &D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXToRadian(120));
	rotationKeys[3].Time = 4.0f;
	D3DXQuaternionRotationAxis(&rotationKeys[3].Value, &D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXToRadian(10));
	
	translationKeys[0].Time = 1.0f;
	translationKeys[0].Value = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	translationKeys[1].Time = 2.0f;
	translationKeys[1].Value = D3DXVECTOR3(1.0f, 0.0f, 1.0f);
	translationKeys[2].Time = 4.0f;
	translationKeys[2].Value = D3DXVECTOR3(0.0f, 1.0f, 5.0f);
	
	scaleKeys[0].Time = 0.0f;
	scaleKeys[0].Value = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	scaleKeys[1].Time = 2.0f;
	scaleKeys[1].Value = D3DXVECTOR3(0.3f, 0.3f, 1.0f);
	scaleKeys[2].Time = 3.5f;
	scaleKeys[2].Value = D3DXVECTOR3(1.0f, 10.0f, 1.0f);
	scaleKeys[3].Time = 4.0f;
	scaleKeys[3].Value = D3DXVECTOR3(100.0f, 100.0f, 500.0f);

	V_OKAY(this->lpDefaultAnimationSet->RegisterAnimationSRTKeys("Custom Bone 1", 0, 4, 3, 0, rotationKeys, translationKeys, &animIndex));
	V_OKAY(this->lpDefaultAnimationSet->RegisterAnimationSRTKeys("Custom Bone 2", 4, 0, 0, scaleKeys, 0, 0, &animIndex));

	DOUBLE time;
	for (time = 0.0f; time < 150.0f; time += 0.1f)
	{
		DOUBLE periodicPosition = this->lpDefaultAnimationSet->GetPeriodicPosition(time);
		V_OKAY(this->lpDefaultAnimationSet->GetSRT(periodicPosition, 0, &scale, &rotation, &translation));
	}

	
	//
	// TODO: Animation Set testing
	//
	//V_OKAY(this->lpAnimationController->RegisterAnimationSet(this->lpDefaultAnimationSet));
	//
	//// Set animation set to track
	//LPD3DXANIMATIONSET lpAnimSet = 0;
	//V_OKAY(this->lpAnimationController->GetAnimationSet(0, &lpAnimSet));
	//V_OKAY(this->lpAnimationController->SetTrackAnimationSet(0, lpAnimSet));
	//V_OKAY(this->lpAnimationController->SetTrackEnable(0, TRUE));
	//V_OKAY(this->lpAnimationController->SetTrackWeight(0, 1.0f));
	//V_OKAY(this->lpAnimationController->SetTrackSpeed(0, 1.0f));
	//V_OKAY(this->lpAnimationController->SetTrackPosition(0, 0.0f));
	//V_OKAY(this->lpAnimationController->ResetTime());
	//V_OKAY(this->lpAnimationController->SetTrackEnable(0, FALSE));
	//SAFE_RELEASE(lpAnimSet);
	//D3DXFrameRegisterNamedMatrices(&this->frame1, this->lpAnimationController);
	//V_OKAY(this->lpAnimationController->AdvanceTime(0.1f, 0));

	hr = S_OK;

	return hr;
}

InputMan* VideoMan::GetInputMan()
{
	return this->pInputMan;
}
void VideoMan::AttachInputMan(InputMan* inputMan)
{
	this->pInputMan = inputMan;
}

HRESULT VideoMan::InitFont()
{
	V_OKAY( D3DXCreateFont( this->lpD3DDevice, 14, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_RASTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Gulim"), &this->lpFont ) );

	return S_OK;
}

LPDIRECT3DDEVICE9 VideoMan::GetDev()
{
	return this->lpD3DDevice;
}
LPD3DXANIMATIONCONTROLLER VideoMan::GetAnimationController() // global animation controller
{
	return this->lpAnimationController;
}
void VideoMan::SetDrawingModelAtEditor(ModelReader* pMR)
{
	this->pDrawingModel = pMR;
}

HRESULT VideoMan::InitSkinnedMeshVertexBuffer()
{
	// Not Implemented
	
	return S_OK;
}


void VideoMan::ScrollBy( D3DXVECTOR3* dScroll )
{
	this->dungeonTranslation += *dScroll;
}

size_t VideoMan::registerRenderLayer( RenderLayer* pRL )
{
	pRL->setVideoMan( this );
	pRL->setDev( this->lpD3DDevice );

	m_renderLayers.push_back(pRL);
	return m_renderLayers.size();
}

BOOL VideoMan::unregisterRenderLayerAt( unsigned int ui )
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

RenderLayer* VideoMan::getRenderLayerAt( unsigned int ui )
{
	std::list<RenderLayer*>::iterator it = m_renderLayers.begin();
	while (ui--)
		it++;
	return *it;
}

size_t VideoMan::unregisterAllRenderLayers()
{
	size_t ret = m_renderLayers.size();
	while (m_renderLayers.size())
		unregisterRenderLayerAt( 0 );
	return ret;
}

void VideoMan::setWorldViewProjection( const D3DXMATRIX& matWorld, const D3DXMATRIX& matView, const D3DXMATRIX& matProj )
{
	this->matWorld = matWorld;
	this->matView = matView;
	this->matProjection = matProj;

	this->lpD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
	this->lpD3DDevice->SetTransform(D3DTS_VIEW, &matView);
	this->lpD3DDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
}