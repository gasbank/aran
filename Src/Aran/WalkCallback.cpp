#include "stdafx.h"
#include "WalkCallback.h"
#include "Character.h"

WalkCallback::WalkCallback(void)
{
}

WalkCallback::~WalkCallback(void)
{
}



void WalkCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{

	// TODO: Start walking animation by blending with previous animation set
	LPD3DXANIMATIONCONTROLLER lpAC = (LPD3DXANIMATIONCONTROLLER)pData;

	float currentStateWeight = this->GetCharacter()->GetAnimStateWeight();
	

	// already FULL LOITER state... no need to adjust weights
	if ( this->GetCharacter()->GetCharacterAnimationState() == CharacterInterface::CAS_WALKING
		&& currentStateWeight >= 1.0f )
	{
		this->GetCharacter()->SetAnimStateWeight( 1.0f );

		lpAC->SetTrackEnable( 0, FALSE );
		lpAC->SetTrackPosition( 0, 0.0f );
		return;
	}

	currentStateWeight -= 0.1f;

	float nextStateWeight = 1.0f - currentStateWeight;

	// turn off loiter anim set
	lpAC->SetTrackEnable( 0, TRUE );
	lpAC->SetTrackWeight( 0, currentStateWeight );
	

	// turn on walking anim set
	lpAC->SetTrackEnable( 1, TRUE );
	lpAC->SetTrackWeight( 1, nextStateWeight );

	this->GetCharacter()->SetAnimStateWeight( currentStateWeight );
	if ( currentStateWeight < 0.0f )
	{
		// current state has no effect from now,
		// shifting next state to current space

		this->GetCharacter()->SetCharacterAnimationState( CharacterInterface::CAS_WALKING );
		this->GetCharacter()->SetAnimStateWeight( 1.0f ); // current; Next weight will be automatically calculated during runtime
		this->GetCharacter()->SetCharacterAnimationStateNext( CharacterInterface::CAS_UNDEFINED );
	}

	
	if ( pResultData != NULL )
		*(int*)pResultData = 0;
}

void WalkCallback::DoUnregisterCallback()
{

}