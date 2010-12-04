#pragma once

class ArnVec3;

class ARANMATH_API ArnVec4
{
public:
	float x, y, z, w;

	ArnVec4() : x(0), y(0), z(0), w(0) {}
	ArnVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	ArnVec4(const ArnVec3& vec3, float _w) : x(vec3.x), y(vec3.y), z(vec3.z), w(_w) {}

	float operator[](size_t i) const { switch(i) { case 0: return x; case 1: return y; case 2: return z; case 3: return w; default: ARN_THROW_UNEXPECTED_CASE_ERROR } }

	const float* getRawData() const { return (const float*)&x; }
	void printFormatString() const
	{
		printf("(x %6.3f, y %6.3f, z %6.3f, w %6.3f)\n", x, y, z, w);
	}
};

ARANMATH_API ArnVec4 CreateArnVec4(float x, float y, float z, float w);
