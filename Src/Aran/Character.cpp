#include "AranPCH.h"
#include "Character.h"
#include "CharacterAnimationCallback.h"
#include "ModelReader.h"
#include "Animation.h"
#include "ArnMath.h"

namespace Aran
{
	Character::Character(void)
	{
		this->pMR = 0;
		this->animState = CharacterInterface::CAS_LOITER;
		this->animStateNext = CharacterInterface::CAS_UNDEFINED;

		this->translation = ArnVec3( 0.0f, 0.0f, 0.0f );
		this->scale = ArnVec3( 1.0f, 1.0f, 1.0f );
		this->rotation = ArnQuat( 0.0f, 0.0f, 0.0f, 0.0f );
		this->lookAt = ArnVec3( 0.0f, -1.0f, 0.0f );
		this->outLookAt = ArnVec4( 0.0f, -1.0f, 0.0f, 1.0f );
		ArnMatrixTransformation( &this->finalTransform, 0, 0, &this->scale, 0, &this->rotation, &this->translation );
	}

	Character::Character( ArnVec3 translation, ArnVec3 scale, ArnQuat rotation )
	{
		this->pMR = 0;
		this->translation = translation;
		this->scale = scale;
		this->rotation = rotation;
		this->lookAt = ArnVec3( 0.0f, -1.0f, 0.0f );
		this->outLookAt = ArnVec4( 0.0f, -1.0f, 0.0f, 1.0f );

		ArnMatrixTransformation( &this->finalTransform, 0, 0, &this->scale, 0, &this->rotation, &this->translation );
	}
	Character::~Character(void)
	{
	}

	void Character::ChangeTranslation( float dx, float dy, float dz )
	{
		ArnMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );

		translation.x += dx;
		translation.y += dy;
		translation.z += dz;
		ArnMatrixTransformation( &this->finalTransform, 0, 0, &this->scale, 0, &this->rotation, &this->translation );
	}

	void Character::ChangeOrientation( float dx, float dy, float dz ) /* radian */
	{
		ArnMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );

		ArnMatrix matRot[4]; // x, y, z, x*y*z
		ArnMatrixRotationX( &matRot[0], dx );
		ArnMatrixRotationY( &matRot[1], dy );
		ArnMatrixRotationZ( &matRot[2], dz );
		ArnMatrix matRotOriginal;
		ArnMatrixRotationQuaternion( &matRotOriginal, &this->rotation );

		matRot[3] = matRot[0] * matRot[1] * matRot[2];
		ArnVec3Transform( &this->outLookAt, &this->lookAt, &matRot[3] );
		this->lookAt.x = this->outLookAt.x;
		this->lookAt.y = this->outLookAt.y;
		this->lookAt.z = this->outLookAt.z;

		matRot[3] *= matRotOriginal;
		ArnQuaternionRotationMatrix( &rotation, &matRot[3] );
		ArnMatrixTransformation( &this->finalTransform, 0, 0, &this->scale, 0, &this->rotation, &this->translation );
	}

	const ArnMatrix* Character::GetFinalTransform() const
	{
		return &finalTransform;
	}

	void Character::SetCharacterAnimationStateNext( CharacterAnimationState nextCAS )
	{
		if (nextCAS >= CAS_SIZE)
			throw new std::runtime_error("Animation callback not defined");

		if ( nextCAS != this->animStateNext )
		{
			// State changed first time
			if (this->callbacks[nextCAS])
				this->callbacks[nextCAS]->DoCallbackFirstTimeOnly( (void*)( this->pMR->GetAnimationController() ), 0 );
		}

		this->animStateNext = nextCAS;

		if (this->callbacks[nextCAS])
			this->callbacks[nextCAS]->DoCallback( (void*)( this->GetModelReader()->GetAnimationController() ), 0 );
	}

	HRESULT Character::RegisterCharacterAnimationCallback( CharacterAnimationState cas, CharacterAnimationCallback* pCAC )
	{
		if ( this->callbacks[cas] != 0 )
			return E_FAIL; // already defined
		this->callbacks[cas] = pCAC;
		pCAC->AttachCharacter( this );

		return S_OK;
	}

	HRESULT Character::UnregisterCharacterAnimationCallback( CharacterAnimationState cas )
	{
		if ( this->callbacks[cas] != 0 )
			return E_FAIL; // already defined
		else
		{
			this->callbacks[cas]->DoUnregisterCallback();
			this->callbacks[cas] = 0;
			return S_OK;
		}
	}

	void Character::ChangeTranslationToLookAtDirection( float amount )
	{
		ArnMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );
		translation += this->lookAt * amount;
		ArnMatrixTransformation( &this->finalTransform, 0, 0, &this->scale, 0, &this->rotation, &this->translation );


	}

	HRESULT Character::AttachModelReader( const ModelReader* pMR )
	{
		ASSERTCHECK( pMR );

		if ( this->pMR != 0 )
		{
			OutputDebugStringA( " ! You should detach model reader before attach new one\n" );
			return E_FAIL;
		}
		else
		{
			this->pMR = pMR;
			return S_OK;
		}
	}

	const ModelReader* Character::GetModelReader() const
	{
		return pMR;
	}

	void Character::Initialize()
	{
		this->animState = CAS_LOITER;
		this->animStateWeight = 1.0f;
		this->animStateNext = CAS_UNDEFINED;

		ZeroMemory( this->callbacks, sizeof( this->callbacks[0] ) * CAS_SIZE );
	}

	void Character::SetCharacterAnimationState( CharacterAnimationState cas )
	{
		this->animState = cas;
	}
}
