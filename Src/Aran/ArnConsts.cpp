#include "AranPCH.h"
#include "ArnConsts.h"

const RST_DATA					RST_DATA::IDENTITY				= { 0, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 0, 0, 0 };
const ArnVec3					ArnConsts::D3DXVEC3_ZERO		= ArnVec3(0, 0, 0);
const ArnVec3					ArnConsts::D3DXVEC3_ONE			= ArnVec3(1.0f, 1.0f, 1.0f);
const ArnVec3					ArnConsts::D3DXVEC3_X			= ArnVec3(1.0f, 0, 0);
const ArnVec3					ArnConsts::D3DXVEC3_Y			= ArnVec3(0, 1.0f, 0);
const ArnVec3					ArnConsts::D3DXVEC3_Z			= ArnVec3(0, 0, 1.0f);
const ArnQuat					ArnConsts::D3DXQUAT_IDENTITY	= ArnQuat(0, 0, 0, 1.0f);
const ArnMatrix					ArnConsts::D3DXMAT_IDENTITY		= ArnMatrix(1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f);
const ArnColorValue4f			ArnConsts::D3DXCOLOR_BLACK		= ArnColorValue4f( 0.0f, 0.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_RED		= ArnColorValue4f( 1.0f, 0.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_GREEN		= ArnColorValue4f( 0.0f, 1.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_BLUE		= ArnColorValue4f( 0.0f, 0.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_YELLOW		= ArnColorValue4f( 1.0f, 1.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_MAGENTA	= ArnColorValue4f( 1.0f, 0.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_CYAN		= ArnColorValue4f( 0.0f, 1.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::D3DXCOLOR_WHITE		= ArnColorValue4f( 1.0f, 1.0f, 1.0f, 1.0f );
