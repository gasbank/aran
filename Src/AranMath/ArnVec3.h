#pragma once

class ARANMATH_API ArnVec3
{
public:
							ArnVec3();
							ArnVec3(float _x, float _y, float _z);
							~ArnVec3();

	template <typename T>
	inline ArnVec3			operator + (const T& v) const;
	template <typename T>
	inline ArnVec3			operator - (const T& v) const;
	template <typename T>
	inline ArnVec3&			operator = (const T& v);

	inline float			operator [] (int i) const;
	inline ArnVec3			operator * ( float f ) const;
	inline ArnVec3&			operator *= ( float f );
	inline ArnVec3			operator / (float f) const;
	inline ArnVec3&			operator /= (float f);
	inline ArnVec3			operator - ( const ArnVec3& v ) const;
	inline ArnVec3&			operator += ( const ArnVec3& v );
	inline ArnVec3&			operator -= ( const ArnVec3& v );
	inline bool				operator == ( const ArnVec3& v ) const;
	inline bool				operator != ( const ArnVec3& v ) const;
	inline void				getFormatString(char* buf, size_t bufSize) const;
	inline void				printFormatString() const;
	float					compare(const ArnVec3& v) const;
	float					x;
	float					y;
	float					z;
};

ARANMATH_API ArnVec3 CreateArnVec3(float x, float y, float z);
ARANMATH_API void ArnVec3GetFormatString(char* buf, size_t bufSize, const ArnVec3& v);
ARANMATH_API ArnVec3 ArnVec3Add(const ArnVec3& v1, const ArnVec3& v2);
ARANMATH_API ArnVec3 ArnVec3Substract(const ArnVec3& v1, const ArnVec3& v2);
ARANMATH_API bool ArnVec3Equals(const ArnVec3& v1, const ArnVec3& v2);
ARANMATH_API void ArnVec3DimensionFromBounds(ArnVec3* out, const boost::array<ArnVec3, 8>& bb);
ARANMATH_API ArnVec3* ArnVec3Assign(ArnVec3* v1, const float* v2);
ARANMATH_API ArnVec3* ArnVec3Assign(ArnVec3* v1, const double* v2);
#include "ArnVec3.inl"

