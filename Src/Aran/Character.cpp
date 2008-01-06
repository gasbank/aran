#include "stdafx.h"
#include "Character.h"
#include "CharacterAnimationCallback.h"
#include "ModelReader.h"

Character::Character(void)
{
	this->animState = CharacterInterface::CAS_LOITER;
	this->animStateNext = CharacterInterface::CAS_UNDEFINED;

	this->translation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	this->scale = D3DXVECTOR3( 0.05f, 0.05f, 0.05f );
	this->rotation = D3DXQUATERNION( 0.0f, 0.0f, 0.0f, 0.0f );
	this->lookAt = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
	this->outLookAt = D3DXVECTOR4( 0.0f, -1.0f, 0.0f, 1.0f );
	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
}

Character::Character( D3DXVECTOR3 translation, D3DXVECTOR3 scale, D3DXQUATERNION rotation )
{
	this->translation = translation;
	this->scale = scale;
	this->rotation = rotation;
	this->lookAt = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
	this->outLookAt = D3DXVECTOR4( 0.0f, -1.0f, 0.0f, 1.0f );

	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
}
Character::~Character(void)
{
}

void Character::ChangeTranslation( float dx, float dy, float dz )
{
	D3DXMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );
	
	translation.x += dx;
	translation.y += dy;
	translation.z += dz;
	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
}

void Character::ChangeOrientation( float dx, float dy, float dz ) /* radian */
{
	D3DXMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );

	D3DXMATRIX matRot[4]; // x, y, z, x*y*z
	D3DXMatrixRotationX( &matRot[0], dx );
	D3DXMatrixRotationY( &matRot[1], dy );
	D3DXMatrixRotationZ( &matRot[2], dz );
	D3DXMATRIX matRotOriginal;
	D3DXMatrixRotationQuaternion( &matRotOriginal, &this->rotation );

	matRot[3] = matRot[0] * matRot[1] * matRot[2];
	D3DXVec3Transform( &this->outLookAt, &this->lookAt, &matRot[3] );
	this->lookAt.x = this->outLookAt.x;
	this->lookAt.y = this->outLookAt.y;
	this->lookAt.z = this->outLookAt.z;

	matRot[3] *= matRotOriginal;
	D3DXQuaternionRotationMatrix( &rotation, &matRot[3] );
	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
}

const D3DXMATRIX* Character::GetFinalTransform() const
{
	return &finalTransform;
}

void Character::SetCharacterAnimationStateNext( CharacterAnimationState nextCAS )
{
	if ( nextCAS == this->animState )
	{
		//DXTRACE_MSG( _T( "State modification duplicated" ) );
	}

	this->animStateNext = nextCAS;
	
	if (this->callbacks[nextCAS])
		this->callbacks[nextCAS]->DoCallback( (void*)( this->GetModelReader()->GetAnimationController() ), NULL );

}

HRESULT Character::RegisterCharacterAnimationCallback( CharacterAnimationState cas, CharacterAnimationCallback* pCAC )
{
	if ( this->callbacks[cas] != NULL )
		return E_FAIL; // already defined
	this->callbacks[cas] = pCAC;
	pCAC->AttachCharacter( this );

	return S_OK;
}

HRESULT Character::UnregisterCharacterAnimationCallback( CharacterAnimationState cas )
{
	if ( this->callbacks[cas] != NULL )
		return E_FAIL; // already defined
	else
	{
		this->callbacks[cas]->DoUnregisterCallback();
		this->callbacks[cas] = NULL;
		return S_OK;
	}
}

void Character::ChangeTranslationToLookAtDirection( float amount )
{
	D3DXMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );
	translation += this->lookAt * amount;
	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );


}

HRESULT Character::AttachModelReader( ModelReader* pMR )
{
	if ( this->pMR != NULL )
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

ModelReader* Character::GetModelReader()
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