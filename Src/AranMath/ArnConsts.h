#pragma once

struct ARANMATH_API ArnConsts
{
	static const RST_DATA				RST_DATA_IDENTITY;

	static const ArnVec3				ARNVEC3_ZERO;
	static const ArnVec3				ARNVEC3_ONE;
	static const ArnVec3				ARNVEC3_X;
	static const ArnVec3				ARNVEC3_Y;
	static const ArnVec3				ARNVEC3_Z;
	static const ArnQuat				ARNQUAT_IDENTITY;
	static const ArnMatrix				ARNMAT_IDENTITY;

	static const ArnVec4				ARNVEC4_ZERO;

	static const ArnColorValue4f		ARNCOLOR_BLACK;
	static const ArnColorValue4f		ARNCOLOR_RED;
	static const ArnColorValue4f		ARNCOLOR_GREEN;
	static const ArnColorValue4f		ARNCOLOR_BLUE;
	static const ArnColorValue4f		ARNCOLOR_YELLOW;
	static const ArnColorValue4f		ARNCOLOR_MAGENTA;
	static const ArnColorValue4f		ARNCOLOR_CYAN;
	static const ArnColorValue4f		ARNCOLOR_WHITE;
	static const ArnColorValue4f		ARNCOLOR_ORANGE;

	static const ArnMaterialData		ARNMTRLDATA_RED;
	static const ArnMaterialData		ARNMTRLDATA_WHITE;
	static const ArnMaterialData		ARNMTRLDATA_MAGENTA;
};

static const unsigned int FPS = 60;

