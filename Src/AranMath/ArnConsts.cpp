#include "AranMathPCH.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnColorValue4f.h"
#include "ArnConsts.h"

const RST_DATA					ArnConsts::RST_DATA_IDENTITY	= { 0, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 0, 0, 0 };
const ArnVec3					ArnConsts::ARNVEC3_ZERO			= ArnVec3(0, 0, 0);
const ArnVec3					ArnConsts::ARNVEC3_ONE			= ArnVec3(1.0f, 1.0f, 1.0f);
const ArnVec3					ArnConsts::ARNVEC3_X			= ArnVec3(1.0f, 0, 0);
const ArnVec3					ArnConsts::ARNVEC3_Y			= ArnVec3(0, 1.0f, 0);
const ArnVec3					ArnConsts::ARNVEC3_Z			= ArnVec3(0, 0, 1.0f);
const ArnVec4					ArnConsts::ARNVEC4_ZERO			= ArnVec4(0, 0, 0, 0);
const ArnQuat					ArnConsts::ARNQUAT_IDENTITY		= ArnQuat(0, 0, 0, 1.0f);
const ArnMatrix					ArnConsts::ARNMAT_IDENTITY		= ArnMatrix(1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f,0,0,0,0,1.0f);
const ArnColorValue4f			ArnConsts::ARNCOLOR_BLACK		= ArnColorValue4f( 0.0f, 0.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_RED			= ArnColorValue4f( 1.0f, 0.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_GREEN		= ArnColorValue4f( 0.0f, 1.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_BLUE		= ArnColorValue4f( 0.0f, 0.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_YELLOW		= ArnColorValue4f( 1.0f, 1.0f, 0.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_MAGENTA		= ArnColorValue4f( 1.0f, 0.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_CYAN		= ArnColorValue4f( 0.0f, 1.0f, 1.0f, 1.0f );
const ArnColorValue4f			ArnConsts::ARNCOLOR_WHITE		= ArnColorValue4f( 1.0f, 1.0f, 1.0f, 1.0f );
const ArnMaterialData			ArnConsts::ARNMTRLDATA_RED		= { ArnConsts::ARNCOLOR_RED, ArnConsts::ARNCOLOR_RED, ArnConsts::ARNCOLOR_RED, ArnConsts::ARNCOLOR_RED, 1.0f };
const ArnMaterialData			ArnConsts::ARNMTRLDATA_WHITE	= { ArnConsts::ARNCOLOR_WHITE, ArnConsts::ARNCOLOR_WHITE, ArnConsts::ARNCOLOR_WHITE, ArnConsts::ARNCOLOR_WHITE, 1.0f };
