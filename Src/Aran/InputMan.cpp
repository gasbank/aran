#include "stdafx.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#ifndef KEYDOWN
#define KEYDOWN(name, key) (name[key] & 0x80)
#endif


#include "InputMan.h"
#include "CharacterInterface.h"
#include "DungeonInterface.h"

InputMan::InputMan(void)
{
	
}

InputMan::~InputMan(void)
{
	/*if (this->lpDInput8 && this->lpDInputDevKeyboard)
	{
		this->lpDInputDevKeyboard->Unacquire();
	}
	SAFE_RELEASE( this->lpDInputDevKeyboard );
	SAFE_RELEASE( this->lpDInput8 );*/
}

void InputMan::WalkCharacterForward( float amount /*= 1.0f */ )
{
	this->pCharInterface->ChangeTranslationToLookAtDirection( amount );
	this->pCharInterface->SetCharacterAnimationStateNext( CharacterInterface::CAS_WALKING );
}

void InputMan::WalkCharacterBackward( float amount /*= 1.0f */ )
{
	this->pCharInterface->ChangeTranslationToLookAtDirection( -amount );
}

void InputMan::TurnCharacterLeft( float amount /*= D3DXToRadian( 10 ) */ )
{
	this->pCharInterface->ChangeOrientation( 0.0f, 0.0f, amount );
}

void InputMan::TurnCharacterRight( float amount /*= D3DXToRadian( 10 ) */ )
{
	this->pCharInterface->ChangeOrientation( 0.0f, 0.0f, -amount );
}

const D3DXMATRIX* InputMan::GetFinalTransform() const
{
	return this->pCharInterface->GetFinalTransform();
}

void InputMan::StopCharacterWalking()
{
	this->pCharInterface->SetCharacterAnimationStateNext( CharacterInterface::CAS_LOITER );
}

HRESULT InputMan::Initialize( HINSTANCE hInst, HWND hwnd )
{
	HRESULT hr = S_OK;
	//ASSERTCHECK( this->lpDInput8 == 0 && this->lpDInputDevKeyboard == 0 );

	//V_OKAY( hr = DirectInput8Create( hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&this->lpDInput8, 0 ) );
	//V_OKAY( hr = this->lpDInput8->CreateDevice( GUID_SysKeyboard, &this->lpDInputDevKeyboard, 0 ) );
	//V_OKAY( hr = this->lpDInputDevKeyboard->SetDataFormat( &c_dfDIKeyboard ) );
	//V_OKAY( hr = this->lpDInputDevKeyboard->SetCooperativeLevel( hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) );

	// To acquire keyboard properly, you should call InputMan::AcquireKeyboard()
	// **AFTER** ShowWindow() or UpdateWindow() call...!
	// note. just guessing...

	return hr;
}

HRESULT InputMan::AcquireKeyboard()
{
	//HRESULT hr;

	//
	// Acquire() should be called after calling ShowWindow() or UpdateWindow() --- maybe?
	// AND! The method call itself and Acquire() method should be not blocked by
	// any Visual Studio debugger's breakpoints since it keeps from acquire keyboard access
	// by unfocusing debugee(Aran Project)
	//
	//V_OKAY( hr = this->lpDInputDevKeyboard->Acquire() );
	
	/*while (FAILED(this->lpDInputDevKeyboard->Acquire()))
	{

	}*/

	return S_OK;
}


HRESULT WINAPI InputMan::ProcessKeyboardInput()
{
	HRESULT hr = S_OK;
	//static char buffer[256];
	
	//hr = this->lpDInputDevKeyboard->GetDeviceState( sizeof( buffer ), (LPVOID)&buffer );
	//// keyboard handle lost(maybe task-switching)
	//if ( FAILED( hr ) )
	//{
	//	// reacquiring; this request always will be failed while in debugging mode
	//	// or in focus-lost state. (so do not use ASSERTCHECK or V_OKAY macros here)
	//	hr = this->lpDInputDevKeyboard->Acquire();
	//	return hr;
	//}


	////////////////////////////////////////////////////////////////////////////
	//// Character Movement
	////////////////////////////////////////////////////////////////////////////
	//BOOL directionalKeyPressed = FALSE;
	//float walkSpeed = 0.2f;
	//float turnSpeed = D3DXToRadian( 5 );
	//if ( KEYDOWN( buffer, DIK_LSHIFT ) )
	//{
	//	walkSpeed += 0.2f;
	//	turnSpeed += D3DXToRadian( 5 );
	//}
	//if ( KEYDOWN( buffer, DIK_UP ) )
	//{
	//	this->WalkCharacterForward( walkSpeed );
	//	directionalKeyPressed = TRUE;
	//}
	//else if ( KEYDOWN( buffer, DIK_DOWN ) )
	//{
	//	this->WalkCharacterBackward( walkSpeed );
	//	directionalKeyPressed = TRUE;
	//}

	//if ( KEYDOWN( buffer, DIK_LEFT ) )
	//{
	//	this->TurnCharacterLeft( turnSpeed );
	//	directionalKeyPressed = TRUE;
	//}
	//else if ( KEYDOWN( buffer, DIK_RIGHT ) )
	//{
	//	this->TurnCharacterRight( turnSpeed );
	//	directionalKeyPressed = TRUE;
	//}

	//if ( directionalKeyPressed == FALSE )
	//{
	//	//this->StopCharacterWalking();
	//}
	//

	////////////////////////////////////////////////////////////////////////////
	//// Dungeon Scroll
	////////////////////////////////////////////////////////////////////////////
	//D3DXVECTOR3 dScroll( 0.0f, 0.0f, 0.0f );
	//float scrollSpeed = 1.0f;
	//if ( KEYDOWN( buffer, DIK_W ) )
	//{
	//	dScroll.y -= scrollSpeed;
	//	this->DungeonScrollBy( &dScroll );
	//}
	//else if ( KEYDOWN( buffer, DIK_S ) )
	//{
	//	dScroll.y += scrollSpeed;
	//	this->DungeonScrollBy( &dScroll );
	//}

	//if ( KEYDOWN( buffer, DIK_A ) )
	//{
	//	dScroll.x += scrollSpeed;
	//	this->DungeonScrollBy( &dScroll );
	//}
	//else if ( KEYDOWN( buffer, DIK_D ) )
	//{
	//	dScroll.x -= scrollSpeed;
	//	this->DungeonScrollBy( &dScroll );
	//}



	////////////////////////////////////////////////////////////////////////////
	//// Test
	////////////////////////////////////////////////////////////////////////////
	//if ( KEYDOWN( buffer, DIK_NUMPAD0 ) )
	//{
	//	OutputDebugString( _T( " . Numpad 0 pressed\n" ) );
	//}
	//else if ( KEYDOWN( buffer, DIK_NUMPAD1 ) )
	//{
	//	OutputDebugString( _T( " . Numpad 1 pressed\n" ) );
	//}
	//else if ( KEYDOWN( buffer, DIK_NUMPAD2 ) )
	//{
	//	OutputDebugString( _T( " . Numpad 2 pressed\n" ) );
	//}

	return hr;
}

void InputMan::DungeonScrollBy( D3DXVECTOR3* pDScroll )
{
	this->pDungeonInterface->ScrollBy( pDScroll );
}