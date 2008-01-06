#include "stdafx.h"
#include "CharacterAnimationCallback.h"

CharacterAnimationCallback::CharacterAnimationCallback(void)
{
}

CharacterAnimationCallback::~CharacterAnimationCallback(void)
{
}

void CharacterAnimationCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{
	if ( pResultData != NULL )
		*(int*)pResultData = 0;
}

void CharacterAnimationCallback::DoUnregisterCallback()
{

}

void CharacterAnimationCallback::AttachCharacter( Character* pCharacter )
{
	this->pCharacter = pCharacter;
}

Character* CharacterAnimationCallback::GetCharacter()
{
	return this->pCharacter;
}