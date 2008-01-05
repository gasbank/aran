#include <d3dx9.h>
#include "LoiterCallback.h"
#include "Character.h"

LoiterCallback::LoiterCallback(void)
{
}

LoiterCallback::~LoiterCallback(void)
{
}


void LoiterCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{
	// TODO: Start walking animation by blending with previous animation set
	LPD3DXANIMATIONCONTROLLER lpAC = (LPD3DXANIMATIONCONTROLLER)pData;

	//float currentStateWeight = this->GetCharacter()->GetAnimStateWeight();
	//float nextStateWeight = 1.0f - currentStateWeight;

	//// turn off walking anim set
	//lpAC->SetTrackEnable( 1, TRUE );
	//lpAC->SetTrackWeight( 1, currentStateWeight );

	//// turn on loitering anim set
	//lpAC->SetTrackEnable( 0, TRUE );
	//lpAC->SetTrackWeight( 0, nextStateWeight );

	//currentStateWeight -= 0.01f;
	//this->GetCharacter()->SetAnimStateWeight( currentStateWeight );
	//if ( currentStateWeight < 0.0f )
	//{
	//	// current state has no effect from now,
	//	// shifting next state to current space
	//	this->GetCharacter()->SetCharacterAnimationState( CharacterInterface::CAS_LOITER );
	//	this->GetCharacter()->SetAnimStateWeight( 1.0f );
	//	this->GetCharacter()->SetCharacterAnimationStateNext( CharacterInterface::CAS_UNDEFINED );
	//}


	lpAC->SetTrackEnable( 0, TRUE );
	lpAC->SetTrackWeight( 0, 1.0f );

	lpAC->SetTrackEnable( 1, TRUE );
	lpAC->SetTrackWeight( 1, 0.0f );

	this->GetCharacter()->SetCharacterAnimationState( CharacterInterface::CAS_LOITER );
	this->GetCharacter()->SetAnimStateWeight( 1.0f );
	this->GetCharacter()->SetCharacterAnimationStateNext( CharacterInterface::CAS_UNDEFINED );

	if ( pResultData != NULL )
		*(int*)pResultData = 0;
}

void LoiterCallback::DoUnregisterCallback()
{

}