#include "Character.h"

Character::Character(void)
{
	this->translation = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	this->scale = D3DXVECTOR3( 0.05f, 0.05f, 0.05f );
	this->rotation = D3DXQUATERNION( 0.0f, 0.0f, 0.0f, 0.0f );
	this->lookAt = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
	this->outLookAt = D3DXVECTOR4( 0.0f, -1.0f, 0.0f, 1.0f );
	D3DXMatrixTransformation( &this->finalTransform, NULL, NULL, &this->scale, NULL, &this->rotation, &this->translation );
}

Character::~Character(void)
{
}
