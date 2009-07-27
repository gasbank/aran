#pragma once

class ARANMATH_API ArnVec3
{
public:
							ArnVec3();
							ArnVec3(float _x, float _y, float _z);
#ifdef WIN32
							ArnVec3(const D3DXVECTOR3* dxvec);
#endif
							~ArnVec3();

#ifdef WIN32
	const D3DXVECTOR3*		getConstDxPtr() const { return reinterpret_cast<const D3DXVECTOR3*>(this); }
	D3DXVECTOR3*			getDxPtr() { return reinterpret_cast<D3DXVECTOR3*>(this); }
#endif

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
	float					x;
	float					y;
	float					z;
};

typedef std::tr1::shared_ptr<ArnVec3> ArnVec3Ptr;

ARANMATH_API ArnVec3 CreateArnVec3(float x, float y, float z);
ARANMATH_API void ArnVec3GetFormatString(char* buf, size_t bufSize, const ArnVec3& v);
ARANMATH_API ArnVec3 ArnVec3Add(const ArnVec3& v1, const ArnVec3& v2);
ARANMATH_API ArnVec3 ArnVec3Substract(const ArnVec3& v1, const ArnVec3& v2);
ARANMATH_API bool ArnVec3Equals(const ArnVec3& v1, const ArnVec3& v2);

#ifdef WIN32
ARANMATH_API const D3DXVECTOR3* ArnVec3GetConstDxPtr(const ArnVec3& v);
ARANMATH_API D3DXVECTOR3* ArnVec3GetDxPtr(ArnVec3& v);
#endif


#include "ArnVec3.inl"

