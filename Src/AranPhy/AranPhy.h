/*!
 * @file AranPhy.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * AranPhy API 주 헤더 파일
 */
#pragma once

class GeneralBody;
class ArnPlane;

/*!
 * @brief 물리 라이브러리 초기화
 * @return 성공 시 0, 실패(재 초기화 포함) 시 음수
 */
ARANPHY_API int
ArnInitializePhysics();

/*!
 * @brief 물리 라이브러리 해제
 * @return 성공 시 0, 실패(미 초기화 후 호출 포함) 시 음수
 * @remarks 초기화가 되지 않은 상태에서 해제하는 경우 및 두 번 이상 해제하는 것도 실패로 간주
 */
ARANPHY_API int
ArnCleanupPhysics();

/*!
 * @brief 선분과 ArnPlane 사이의 교차점 계산
 *
 * 시작점과 끝점으로 정의되는 선분과 ArnPlane의 교차점을 계산합니다.
 */
ARANPHY_API void
ArnLineSegmentPlaneIntersection(
	std::list<ArnVec3>& points,
	const ArnVec3& p0,
	const ArnVec3& p1,
	const ArnPlane& plane
);

/*!
 * @brief 상자와 ArnPlane 사이의 교차점 계산
 *
 * 8개의 점으로 정의된 상자와 ArnPlane의 교차점을 계산합니다.
 */
ARANPHY_API void
ArnBoxPlaneIntersection(
	std::list<ArnVec3>& points,
	const ArnVec3 box[8],
	const ArnPlane& plane
);

/*!
 * @brief 상자와 ArnPlane 사이의 교차점 계산
 *
 * 8개의 점으로 정의된 상자와 ArnPlane의 교차점을 계산합니다.
 */
ARANPHY_API void
ArnXformedBoxPlaneIntersection(
	std::list<ArnVec3>& points,
	const ArnVec3& boxSize,
	const ArnVec3& boxPos,
	const ArnQuat& q,
	const ArnPlane& plane
);

/*!
 * @brief GeneralBody 와 ArnPlane 사이의 교차점 계산
 *
 * 다수의 교차점이 있을 수도 있으므로 교차점 리스트를 만들어 냅니다.
 */
ARANPHY_API void
ArnGeneralBodyPlaneIntersection(
	std::list<ArnVec3>& points,		///< [out] 교차점
	const GeneralBody& gb,			///< [in] 교차 테스트를 할 물리
	const ArnPlane& plane			///< [in] 교차 테스트를 할 평면
);
