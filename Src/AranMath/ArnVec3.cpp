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

void ArnVec3DimensionFromBounds( ArnVec3* out, const boost::array<ArnVec3, 8>& bb )
{
	// In sequence of ---, --+, -++, -+-, +--, +-+, +++, ++-.
	out->x = abs(bb[0].x - bb[6].x);
	out->y = abs(bb[0].y - bb[6].y);
	out->z = abs(bb[0].z - bb[6].z);
}

ArnVec3* ArnVec3Assign(ArnVec3* v1, const float* v2)
{
	assert(v1);
	v1->x = v2[0];
	v1->y = v2[1];
	v1->z = v2[2];
	return v1;
}

ArnVec3* ArnVec3Assign(ArnVec3* v1, const double* v2)
{
	assert(v1);
	v1->x = static_cast<float>(v2[0]);
	v1->y = static_cast<float>(v2[1]);
	v1->z = static_cast<float>(v2[2]);
	return v1;
}
