#include "AranPCH.h"
#include "CharacterAnimationCallback.h"

CharacterAnimationCallback::CharacterAnimationCallback(void)
{
}

CharacterAnimationCallback::~CharacterAnimationCallback(void)
{
}


void CharacterAnimationCallback::DoCallbackFirstTimeOnly( void* pData /* in */, void* pResultData /* out */ )
{
	UNREFERENCED_PARAMETER(pData);
	
	if ( pResultData != 0 )
		*(int*)pResultData = 0;
}

void CharacterAnimationCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{
	UNREFERENCED_PARAMETER(pData);

	if ( pResultData != 0 )
		*(int*)pResultData = 0;
}

void CharacterAnimationCallback::DoUnregisterCallback()
{

}

void CharacterAnimationCallback::AttachCharacter( Aran::Character* pCharacter )
{
	this->pCharacter = pCharacter;
}

Aran::Character* CharacterAnimationCallback::GetCharacter()
{
	return this->pCharacter;
}