/*!
 * @file ArnIntersection.h
 * @author softSurfer (www.softsurfer.com)
 * @author Geoyeob Kim
 * @date 2001, 2009
 */
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test equality
//            != to test inequality
//            Point  = Point ± Vector
//            Vector = Point - Point
//            Vector = Scalar * Vector    (scalar product)
//            Vector = Vector * Vector    (3D cross product)
//    Line and Ray and Segment with defining points {Point P0, P1;}
//        (a Line is infinite, Rays and Segments start at P0)
//        (a Ray extends beyond P1, but a Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector n;}
//===================================================================
#pragma once

class ArnVec3;
class ArnPlane;
class ArnQuat;

/*!
 * @brief 한 점의 어떤 평면에 대한 투영된 위치를 반환
 * @param [out] p 투영된 점
 * @param [in] point 투영 될 점
 * @param [in] plane 평면
 * @return \a plane 과 \a point 사이의 거리
 * @remarks \a p 와 \a point 는 다른 객체여야 합니다.
 *
 * 3차원 공간에서 법선과 한 점으로 정의된 평면 \a plane 에 대해
 * 한 점 \a point 의 투영된 점 \a p 를 계산합니다.
 * 이러한 점은 유일하게 반드시 존재합니다.
 */
ARANMATH_API float
ArnProjectPointToPlane(ArnVec3& p, const ArnVec3& point, const ArnPlane& plane);

/*!
 * @brief 벡터를 벡터에 투영시킨 벡터를 반환
 * @return 투영된 벡터의 길이(양수면 투영시킬 벡터와 같은 방향, 음수면 반대 방향임)
 */
ARANMATH_API float
ArnProjectVector(ArnVec3& out,					///< [out] 투영된 벡터
				const ArnVec3& toBeProjected,	///< [in] 투영될 벡터
				const ArnVec3& vec				///< [in] 투영시킬 벡터
				);

/*!
 * @brief 패치와 Z축과 평행인 선과의 교차점 반환
 * @return 교차점이 있으면 1, 없으면 0
 *
 * 패치는 한 점과 그 점에서 뻗어나가는 두 변으로 정의되는 유한한 넓이의 사각형입니다.
 */
ARANMATH_API int
ArnPatchVerticalLineIntersection(ArnVec3& ret,			///< [out] 교차점(반환값이 1인 경우에만 유효)
								 const ArnVec3& p0,		///< [in] 패치의 한 점
								 const ArnVec3& v1,		///< [in] 패치의 첫 번째 변
								 const ArnVec3& v2,		///< [in] 패치의 두 번째 변
								 const float x,			///< [in] Z축과 평행인 선의 X 좌표
								 const float y			///< [in] Z축과 평행인 선의 Y 좌표
								 );

/*
 *
ARANMATH_API int
ArnPatchVerticalLineIntersection(ArnVec3& ret, const ArnVec3& v0, const ArnVec3& v1, const ArnVec3& v2, const int x, const int y);

/*!
 * @brief Line-Plane 교차점 반환
 * @param [out] p 교차점
 * @param [in] p0 직선위의 한 점
 * @param [in] p1 직선의 정규화된 방향
 * @param [in] plane 평면
 * @return 0: 교차점이 없는 경우\n
 *         1: 교차점이 있는 경우
 *
 * 직선과 Plane 사이의 교차점을 계산하여 반환합니다.
 * 평면의 법선 벡터와 직선 방향이 직교하는 경우에는 교차점이 없는데,
 * 이 경우 0이 반환되며 \c p 는 정의되지 않습니다.
 * 직선은 시작점과 끝점이 정의되지 않고 무한히 긴 선이며 시작점과 방향이 존재하는 ray와는 다릅니다.
 * 이러한 특성 때문에 \a p1 대신에 \a -p1 를 넣어도 결과는 바뀌지 않습니다.
 */
ARANMATH_API int ArnLinePlaneIntersection(ArnVec3& p, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& plane);

/*!
 * @brief 두 평면의 교차 테스트
 * @param [out] p0 직선 위의 한 점
 * @param [out] p1 직선의 정규화된 방향
 * @param [in] Pn1 평면 1
 * @param [in] Pn2 평면 2
 * @return 0: 교차하지 않음\n
 *         1: 두 평면이 겹쳐있음\n
 *         2: 유일한 교차선(intersection line)이 있음
 */
ARANMATH_API int intersect3D_2Planes( ArnVec3& p0, ArnVec3& p1, const ArnPlane& Pn1, const ArnPlane& Pn2 );

// intersect3D_SegmentPlane(): intersect a segment and a plane
//    Input:  S = a segment, and Pn = a plane = {Point V0; Vector n;}
//    Output: *I0 = the intersect point (when it exists)
//    Return: 0 = disjoint (no intersection)
//            1 = intersection in the unique point *I0
//            2 = the segment lies in the plane
ARANMATH_API int intersect3D_SegmentPlane(ArnVec3& I, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& Pn);

/*!
* @brief 선분과 ArnPlane 사이의 교차점 계산
*
* 시작점과 끝점으로 정의되는 선분과 ArnPlane의 교차점을 계산합니다.
*/
ARANMATH_API void
ArnLineSegmentPlaneIntersection(
								std::vector<ArnVec3>& points,
								const ArnVec3& p0,
								const ArnVec3& p1,
								const ArnPlane& plane
								);

/*!
* @brief 상자와 ArnPlane 사이의 교차점 계산
*
* 8개의 점으로 정의된 상자와 ArnPlane의 교차점을 계산합니다.
*/
ARANMATH_API void
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
ARANMATH_API void
ArnXformedBoxPlaneIntersection(
							   std::vector<ArnVec3>& points,
							   const ArnVec3& boxSize,
							   const ArnVec3& boxPos,
							   const ArnQuat& q,
							   const ArnPlane& plane
							   );

/*!
 * @brief 상자와 수직선 사이의 교차점 계산
 */
ARANMATH_API float
ArnXformedBoxVerticalLineIntersection(std::vector<ArnVec3>& points,
									  const ArnVec3& boxSize,
									  const ArnVec3& boxPos,
									  const ArnQuat& q,
									  const float vx,
									  const float vy
									  );
