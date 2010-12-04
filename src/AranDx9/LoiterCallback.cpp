#include "AranDx9PCH.h"
#include "LoiterCallback.h"
#include "Character.h"
#include "Animation.h"
#include "ArnAnimationController.h"

LoiterCallback::LoiterCallback(void)
{
}

LoiterCallback::~LoiterCallback(void)
{
}


void LoiterCallback::DoCallbackFirstTimeOnly( void* pData /* in */, void* pResultData /* out */ )
{
	ArnAnimationController* lpAC = (ArnAnimationController*)pData;
	lpAC->SetTrackPosition( 1, 0.0f );

	if ( pResultData != 0 )
		*(int*)pResultData = 0;
}

void LoiterCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{
	// TODO: Start walking animation by blending with previous animation set
	ArnAnimationController* lpAC = (ArnAnimationController*)pData;

	float currentStateWeight = this->GetCharacter()->GetAnimStateWeight();



	// already FULL LOITER state... no need to adjust weights
	// turn off previous track and set position to 0.0f to interpolate smoothly
	if ( this->GetCharacter()->GetCharacterAnimationState() == CharacterInterface::CAS_LOITER
		&& currentStateWeight >= 1.0f )
	{
		this->GetCharacter()->SetAnimStateWeight( 1.0f );

		lpAC->SetTrackEnable( 1, FALSE );
		lpAC->SetTrackPosition( 1, 0.0f );

		return;
	}

	currentStateWeight -= 0.10f; // speed of mixing(blending)

	float nextStateWeight = 1.0f - currentStateWeight;

	// turn off walking anim set
	lpAC->SetTrackEnable( 1, TRUE );
	lpAC->SetTrackWeight( 1, currentStateWeight );

	// turn on loitering anim set
	lpAC->SetTrackEnable( 0, TRUE );
	lpAC->SetTrackWeight( 0, nextStateWeight );

	this->GetCharacter()->SetAnimStateWeight( currentStateWeight );
	if ( currentStateWeight < 0.0f )
	{
		// current state has no effect from now,
		// shifting next state to current space
		this->GetCharacter()->SetCharacterAnimationState( CharacterInterface::CAS_LOITER );
		this->GetCharacter()->SetAnimStateWeight( 1.0f );
		this->GetCharacter()->SetCharacterAnimationStateNext( CharacterInterface::CAS_LOITER );
	}


	/*lpAC->SetTrackEnable( 0, TRUE );
	lpAC->SetTrackPosition( 0, 0.0f );
	lpAC->SetTrackWeight( 0, 1.0f );

	lpAC->SetTrackEnable( 1, TRUE );
	lpAC->SetTrackWeight( 1, 0.0f );

	this->GetCharacter()->SetCharacterAnimationState( CharacterInterface::CAS_LOITER );
	this->GetCharacter()->SetAnimStateWeight( 1.0f );
	this->GetCharacter()->SetCharacterAnimationStateNext( CharacterInterface::CAS_UNDEFINED );*/

	if ( pResultData != 0 )
		*(int*)pResultData = 0;
}

void LoiterCallback::DoUnregisterCallback()
{

}

