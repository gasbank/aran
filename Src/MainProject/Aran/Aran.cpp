// Aran.cpp
// 2007 Geoyeob Kim
// 프로젝트 메인 엔트리
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#include <iostream>

#include "Aran.h"
#include "VideoMan.h"
#include "InputMan.h"
#include "Character.h"

VideoMan videoMan;
InputMan inputMan;
Character character;		// player character


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//int main()
{
	// TODO: Entry Point
	std::cout << _T("Starting Aran...") << std::endl;

	inputMan.AttachCharacterInterface( &character );
	videoMan.AttachInputMan( &inputMan );
	videoMan.AttachCharacter( &character );
	
	HRESULT hr;

	hr = videoMan.InitWindow(_T("Aran"), MsgProc, 1024, 768);
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Window Initialization Error"), hr);
	}

	
	hr = videoMan.InitD3D();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Direct3D Initialization Error"), hr);
	}

	V_OKAY(videoMan.InitAnimationController());

	hr = videoMan.InitCustomMesh();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Custom Mesh Initialization Error"), hr);
	}

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

	hr = videoMan.StartMainLoop();
	if (FAILED(hr))
	{
		return DXTRACE_ERR_MSGBOX(_T("Main Loop Failure"), hr);
	}

	std::cout << _T("Terminating Aran...") << std::endl;

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
		break;
	case WM_CLOSE:
		videoMan.Close();
		PostQuitMessage(0);
		break;
	case WM_KILLFOCUS:
		videoMan.PauseMainLoop();
		break;
	case WM_SETFOCUS:
		videoMan.ResumeMainLoop();
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP: inputMan.WalkCharacterForward(); break;
		case VK_DOWN: inputMan.WalkCharacterBackward(); break;
		case VK_LEFT: inputMan.TurnCharacterLeft(); break;
		case VK_RIGHT: inputMan.TurnCharacterRight(); break;

		case VK_ESCAPE:
			videoMan.Close();
			PostQuitMessage(0);
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

		_stprintf_s(debugMessage, sizeof(debugMessage)/sizeof(TCHAR), _T("Mouse Up: (%d, %d)\n"), inputMan.GetMouseUpPos().x, inputMan.GetMouseUpPos().y);
		OutputDebugString(debugMessage);
		break;
	case WM_LBUTTONDOWN:
		inputMan.SetMouseDownPos((int)LOWORD(lParam), (int)HIWORD(lParam));
		inputMan.SetClicked((LOWORD(wParam) & MK_LBUTTON) ? TRUE : FALSE);
		inputMan.SetRClicked((LOWORD(wParam) & MK_RBUTTON) ? TRUE : FALSE);

		_stprintf_s(debugMessage, sizeof(debugMessage)/sizeof(TCHAR), _T("Mouse Down: (%d, %d)\n"), inputMan.GetMouseDownPos().x, inputMan.GetMouseDownPos().y);
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
	case WM_MOUSEWHEEL:
		{
			short zDelta = (short)HIWORD(wParam);
			if (zDelta > 0)
			{
				videoMan.MoveCamera(0.0f, 0.0f, 2.0f);
			}
			else
			{
				videoMan.MoveCamera(0.0f, 0.0f, -2.0f);
			}
		}
		break;
	default:

		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}