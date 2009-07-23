#include "AranPCH.h"
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
ArnVec3::ArnVec3( const D3DXVECTOR3* dxvec )
: x(dxvec->x)
, y(dxvec->y)
, z(dxvec->z)
{
}
#endif

ArnVec3::~ArnVec3()
{
}

