#include "AranMathPCH.h"
#include "ArnPlane.h"
#include "ArnMath.h"
#include "ArnConsts.h"

ArnPlane::ArnPlane()
: m_normal(ArnConsts::ARNVEC3_Z)
, m_v0(ArnConsts::ARNVEC3_ZERO)
{
}

ArnPlane::ArnPlane(const ArnVec3& normal, const ArnVec3& v0)
: m_normal(normal)
, m_v0(v0)
{
	assert(ArnVec3IsNormalized(normal));
}

ArnPlane::ArnPlane(const ArnVec3& v0, const ArnVec3& v1, const ArnVec3& v2)
: m_v0(v0)
{
	ArnVec3 a1 = v1 - v0;
	ArnVec3 a2 = v2 - v0;
	ArnVec3 normal = ArnVec3GetCrossProduct(a1, a2);
	m_normal = normal / ArnVec3GetLength(normal);
}

const ArnVec3&
ArnPlane::getNormal() const
{
	assert(ArnVec3IsNormalized(m_normal));
	return m_normal;
}
