#include "AranPCH.h"
#include "Structs.h"

const POINT3FLOAT		POINT3FLOAT::ZERO				= { 0.0f, 0.0f, 0.0f };
const POINT4FLOAT		POINT4FLOAT::ZERO				= { 0.0f, 0.0f, 0.0f, 0.0f };
const RST_DATA			RST_DATA::IDENTITY				= { 0, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 0, 0, 0 };
const D3DXVECTOR3		DX_CONSTS::D3DXVEC3_ZERO		= D3DXVECTOR3(0, 0, 0);
const D3DXVECTOR3		DX_CONSTS::D3DXVEC3_ONE			= D3DXVECTOR3(1.0f, 1.0f, 1.0f);
const D3DXVECTOR3		DX_CONSTS::D3DXVEC3_X			= D3DXVECTOR3(1.0f, 0, 0);
const D3DXVECTOR3		DX_CONSTS::D3DXVEC3_Y			= D3DXVECTOR3(0, 1.0f, 0);
const D3DXVECTOR3		DX_CONSTS::D3DXVEC3_Z			= D3DXVECTOR3(0, 0, 1.0f);
const D3DXQUATERNION	DX_CONSTS::D3DXQUAT_IDENTITY	= D3DXQUATERNION(0, 0, 0, 1.0f);
const D3DXMATRIX		DX_CONSTS::D3DXMAT_IDENTITY		= D3DXMATRIX(1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f);
