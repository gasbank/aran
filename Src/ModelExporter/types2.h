#pragma once

//////////////////////////////////////////////////////////////////////////
// Structs
//////////////////////////////////////////////////////////////////////////
// ModelExporter 2 Spec

struct ArColorValue
{
	ArColorValue() {}
	ArColorValue(float r, float g, float b, float a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	ArColorValue(float r, float g, float b) // alpha=1.0f
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 1.0f;
	}
	~ArColorValue() {}

	float r, g, b, a;

	static const ArColorValue ZERO;
	static const ArColorValue RED;
};

struct ArPoint3
{
	float x, y, z;
};

struct ArMatrix4
{
	ArMatrix4() {}
	ArMatrix4(const GMatrix& gm)
	{
		m[0][0] = gm[0][0]; m[0][1] = gm[0][1]; m[0][2] = gm[0][2]; m[0][3] = gm[0][3];
		m[1][0] = gm[1][0]; m[1][1] = gm[1][1]; m[1][2] = gm[1][2]; m[1][3] = gm[1][3];
		m[2][0] = gm[2][0]; m[2][1] = gm[2][1]; m[2][2] = gm[2][2]; m[2][3] = gm[2][3];
		m[3][0] = gm[3][0]; m[3][1] = gm[3][1]; m[3][2] = gm[3][2]; m[3][3] = gm[3][3];
	}
	float m[4][4];
};

struct ArFullVertex
{
	int origVert;
	ArPoint3 pos;
	ArPoint3 normal;
	ArPoint3 tangent;
	ArPoint3 binormal;
	ArPoint3 uvw;
};


