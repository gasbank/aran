#pragma once

#include <d3dx9.h>

#include "CharacterInterface.h"

class Character:
	public CharacterInterface
{
public:
	Character(void);
	Character(D3DXVECTOR3 translation, D3DXVECTOR3 scale, D3DXQUATERNION rotation)
	{
		this->translation = translation;
		this->scale = scale;
		this->rotation = rotation;
		this->lookAt = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
		this->outLookAt = D3DXVECTOR4( 0.0f, -1.0f, 0.0f, 1.0f );

		D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
	}
	~Character(void);

	virtual void ChangeTranslation( float dx, float dy, float dz )
	{
		D3DXMatrixDecompose( &this->scale, &this->rotation, &this->translation, &this->finalTransform );
		translation += this->lookAt;
		/*translation.x += dx;
		translation.y += dy;
		translation.z += dz;*/
		D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );

	}
	virtual void ChangeOrientation( float dx, float dy, float dz ) /* radian */
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
	virtual const D3DXMATRIX* GetFinalTransform() const
	{
		return &finalTransform;
	}

private:
	D3DXMATRIX finalTransform;
	D3DXVECTOR3 translation, scale;
	D3DXQUATERNION rotation;

	D3DXVECTOR3 lookAt; // character's eye(font) direction
	D3DXVECTOR4 outLookAt;
	


};
