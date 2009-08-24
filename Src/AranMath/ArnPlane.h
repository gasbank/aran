/*!
 * @file ArnPlane.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

#include "ArnVec3.h"

/*!
 * @brief 3차원 공간 평면
 *
 * 평면 위의 한 점과 법선 벡터로 정의되는 평면 클래스입니다.
 */
class ArnPlane
{
public:
						ArnPlane();
						ArnPlane(const ArnVec3& normal, const ArnVec3& v0);
						ArnPlane(const ArnVec3& v0, const ArnVec3& v1, const ArnVec3& v2);
						~ArnPlane() {}
	const ArnVec3&		getV0() const { return m_v0; }
	const ArnVec3&		getNormal() const;
private:
	ArnVec3				m_v0;			///< A point in the plane
	ArnVec3				m_normal;		///< Plane normal (should be normalized)
};
