#pragma once
struct RST_DATA
{
	float x, y, z, w; // rotation
	float sx, sy, sz; // scaling
	float tx, ty, tz; // translation

	static const RST_DATA IDENTITY;
};
