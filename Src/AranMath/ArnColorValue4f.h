#pragma once

class ARANMATH_API ArnColorValue4f
{
public:
	ArnColorValue4f() : r(0), g(0), b(0), a(1.0f)
	{
	}
	ArnColorValue4f(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a)
	{
	}
	void printFormatString() const
	{
		printf("(r %6.3f, g %6.3f, b %6.3f, a %6.3f)\n", r, g, b, a);
	}
#ifdef WIN32
	D3DXCOLOR getDx() const { return D3DXCOLOR(r, g, b, a); }
#endif

	float r;
	float g;
	float b;
	float a;
};
