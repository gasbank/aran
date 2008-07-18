// AranTest.cpp
// 2008 Geoyeob Kim (gasbank@gmail.com)

#include "AranPCH.h"

#include "UndefinedCallback.h"
#include "LoiterCallback.h"
#include "WalkCallback.h"
#include "DefaultRenderLayer.h"
#include "VideoMan.h"
#include "ResourceMan.h"
#include "Character.h"
#include "ArnFile.h"
#include "ArnSceneGraph.h"

LOGMANAGER logManager;
VideoMan videoMan;
InputMan inputMan;
ResourceMan resMan;

static WalkCallback g_walkCallback;
static LoiterCallback g_loiterCallback;
static UndefinedCallback g_undefinedCallback;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	_LogWrite(_T( "WinMain() Start ...!!" ), LOG_OKAY);
	HRESULT hr = E_FAIL;
#ifdef _DEBUG
	// Copy external resources to working directory
	if (system( "..\\Src\\Aran\\CopyResourcesToWorking.bat" ) != 0) // use of '/' instead '\\' cause error
	{
		_LogWrite(_T("Copy Resources Error"), LOG_FAIL);
		return DXTRACE_ERR_MSGBOX(_T("Copy Resources Error"), hr);
	}
#endif
	// 16:9 wide screen?! kyakyakya...
	int screenX = 16*50;
	int screenY = 9*50;
	hr = videoMan.InitWindow(_T("Aran"), MsgProc, screenX, screenY);
	if (FAILED(hr))
	{
		_LogWrite( _T("Window Initialization Error"), LOG_FAIL);
		return DXTRACE_ERR_MSGBOX(_T("Window Initialization Error"), hr);
	}

	inputMan.Initialize( hInstance, videoMan.GetWindowHandle() );
	videoMan.AttachInputMan(&inputMan);

	std::auto_ptr<Character> character(new Character());
	character->Initialize();
	character->RegisterCharacterAnimationCallback( CharacterInterface::CAS_UNDEFINED, &g_undefinedCallback );
	character->RegisterCharacterAnimationCallback( CharacterInterface::CAS_WALKING, &g_walkCallback );
	character->RegisterCharacterAnimationCallback( CharacterInterface::CAS_LOITER, &g_loiterCallback );
	character->SetCharacterAnimationState( CharacterInterface::CAS_WALKING );

	hr = videoMan.InitD3D();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Direct3D Initialization Error"), hr);
	}

	ArnFileData arnFileData;
	load_arnfile(_T("models/man.arn"), arnFileData);
	ArnSceneGraph arnSG(arnFileData);
	release_arnfile(arnFileData);


	resMan.registerModel(ResourceMan::MAN, _T("man.arn"));

	V_OKAY( videoMan.InitAnimationController() );
	V_OKAY( resMan.initializeAll() );
	//character->AttachModelReader( resMan.getModel( ResourceMan::MAN ) );
	videoMan.registerRenderLayer(new DefaultRenderLayer(character.get()));
	videoMan.registerRenderLayer(new BoxRenderLayer());


	hr = S_OK;
	
	/*hr = videoMan.InitCustomMesh();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Custom Mesh Initialization Error"), hr);
	}*/

	hr = videoMan.InitModels();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Model Loading Error"), hr);
	}

	hr = videoMan.InitShader();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Shader Initialization Error"), hr);
	}

	V_OKAY(videoMan.InitFont());

	hr = videoMan.InitLight();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Light Initialization Error"), hr);
	}

	hr = videoMan.InitMaterial();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Material Initialization Error"), hr);
	}

	hr = videoMan.InitMainCamera();
	if ( FAILED ( hr ) )
	{
		return DXTRACE_ERR_MSGBOX(_T("Main Camera Initialization Error"), hr);
	}

	hr = videoMan.Show();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Window Showing Error"), hr);
	}

	inputMan.AcquireKeyboard();
	
	//
	// Starting main loop...
	//
	hr = videoMan.StartMainLoop();
	if (FAILED(hr))
	{
		//return DXTRACE_ERR_MSGBOX(_T("Main Loop Failure"), hr);
	}

	//delete character;

	if (logManager.GetFailCount() != 0)
	{
		MessageBox(0, _T("One or more errors logged!"), _T("Check Log File"), MB_ICONEXCLAMATION);
	}

	return 0;
}




LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//HDC hdc;

	//Point2Int point;
	static TCHAR debugMessage[1024];
	//BOOL isClicked, isRClicked;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(WM_QUIT);
		break;
	case WM_CLOSE:

		break;
	case WM_KILLFOCUS:
		//videoMan.PauseMainLoop();
		break;
	case WM_SETFOCUS:
		//videoMan.ResumeMainLoop();
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{

		case VK_ESCAPE:
			PostQuitMessage(WM_QUIT);

			break;
		}
		break;
	case WM_PAINT:

		break;

		// ArcBall rotation
	case WM_MOUSEMOVE:
		inputMan.SetMouseCurPos((int)LOWORD(lParam), (int)HIWORD(lParam));
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);
		break;
	case WM_LBUTTONUP:
		inputMan.SetMouseUpPos((int)LOWORD(lParam), (int)HIWORD(lParam));
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);

		_stprintf_s(debugMessage, TCHARSIZE(debugMessage), _T("Mouse Up: (%d, %d)\n"), inputMan.GetMouseUpPos().x, inputMan.GetMouseUpPos().y);
		OutputDebugString(debugMessage);
		break;
	case WM_LBUTTONDOWN:
		inputMan.SetMouseDownPos((int)LOWORD(lParam), (int)HIWORD(lParam));
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);

		_stprintf_s(debugMessage, TCHARSIZE(debugMessage), _T("Mouse Down: (%d, %d)\n"), inputMan.GetMouseDownPos().x, inputMan.GetMouseDownPos().y);
		OutputDebugString(debugMessage);
		break;
	case WM_RBUTTONUP:
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);

		break;
	case WM_RBUTTONDOWN:
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);

		break;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
	case WM_MOUSEWHEEL:
		{
			short zDelta = (short)HIWORD(wParam);
			if (zDelta > 0)
			{
				videoMan.MoveMainCameraEye(0.0f, 0.0f, 2.0f);
			}
			else
			{
				videoMan.MoveMainCameraEye(0.0f, 0.0f, -2.0f);

			}
		}
		break;
	default:

		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}