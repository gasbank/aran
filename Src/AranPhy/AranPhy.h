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
 * @brief GeneralBody 와 ArnPlane 사이의 교차점 계산
 *
 * 다수의 교차점이 있을 수도 있으므로 교차점 리스트를 만들어 냅니다.
 */
ARANPHY_API void
ArnGeneralBodyPlaneIntersection(
	std::vector<ArnVec3>& points,		///< [out] 교차점
	const GeneralBody& gb,			///< [in] 교차 테스트를 할 물리
	const ArnPlane& plane			///< [in] 교차 테스트를 할 평면
);

/*!
 * @brief GeneralBody 와 수직선(vertical line)과의 교차점 계산
 *
 * 다수의 교차점이 있을 수도 있으므로 교차점 리스트를 만들어 냅니다.
 */
ARANPHY_API void
ArnGeneralBodyVerticalLineIntersection(std::vector<ArnVec3>& points,	///< [out] 교차점
									const GeneralBody& gb,			///< [in] 교차 테스트를 할 물리
									const float x,					///< [in] 수직선의 X 좌표
									const float y					///< [in] 수직선의 Y 좌표
									);
