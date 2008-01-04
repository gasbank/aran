#include "InputMan.h"
#include "CharacterInterface.h"

InputMan::InputMan(void)
{
}

InputMan::~InputMan(void)
{
}

void InputMan::WalkCharacterForward( float amount /*= 1.0f */ )
{
	this->charInterface->ChangeTranslation( amount, 0.0f, 0.0f );
}

void InputMan::WalkCharacterBackward( float amount /*= 1.0f */ )
{
	this->charInterface->ChangeTranslation( -amount, 0.0f, 0.0f );
}

void InputMan::TurnCharacterLeft( float amount /*= D3DXToRadian( 10 ) */ )
{
	this->charInterface->ChangeOrientation( 0.0f, 0.0f, amount );
}

void InputMan::TurnCharacterRight( float amount /*= D3DXToRadian( 10 ) */ )
{
	this->charInterface->ChangeOrientation( 0.0f, 0.0f, -amount );
}

const D3DXMATRIX* InputMan::GetFinalTransform() const
{
	return this->charInterface->GetFinalTransform();
}