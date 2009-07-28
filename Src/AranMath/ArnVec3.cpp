#include "AranMathPCH.h"
#include "ArnVec3.h"

ArnVec3::ArnVec3()
: x(0)
, y(0)
, z(0)
{
}

ArnVec3::ArnVec3( float _x, float _y, float _z )
: x(_x)
, y(_y)
, z(_z)
{
}

#ifdef WIN32
ArnVec3::ArnVec3( const D3DVECTOR* dxvec )
: x(dxvec->x)
, y(dxvec->y)
, z(dxvec->z)
{
}
#endif

ArnVec3::~ArnVec3()
{
}

//////////////////////////////////////////////////////////////////////////

ArnVec3 CreateArnVec3( float x, float y, float z )
{
	return ArnVec3(x, y, z);
}

void ArnVec3GetFormatString( char* buf, size_t bufSize, const ArnVec3& v )
{
	v.getFormatString(buf, bufSize);
}

ArnVec3 ArnVec3Add( const ArnVec3& v1, const ArnVec3& v2 )
{
	return v1 + v2;
}

ArnVec3 ArnVec3Substract( const ArnVec3& v1, const ArnVec3& v2 )
{
	return v1 - v2;
}

bool ArnVec3Equals( const ArnVec3& v1, const ArnVec3& v2 )
{
	return v1 == v2;
}

#ifdef WIN32
const D3DVECTOR* ArnVec3GetConstDxPtr( const ArnVec3& v )
{
	return v.getConstDxPtr();
}

D3DVECTOR* ArnVec3GetDxPtr( ArnVec3& v )
{
	return v.getDxPtr();
}
#endif // #ifdef WIN32