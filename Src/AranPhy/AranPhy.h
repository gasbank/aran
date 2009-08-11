/*!
 * @file AranPhy.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * AranPhy API 주 헤더 파일
 */
#pragma once

/*!
 * @brief 물리 라이브러리 초기화
 * @return 성공 시 0, 실패(재 초기화 포함) 시 음수
 */
ARANPHY_API int ArnInitializePhysics();

/*!
 * @brief 물리 라이브러리 해제
 * @return 성공 시 0, 실패(미 초기화 후 호출 포함) 시 음수
 * @remark 초기화가 되지 않은 상태에서 해제하는 경우 및 두 번 이상 해제하는 것도 실패로 간주
 */
ARANPHY_API int ArnCleanupPhysics();
