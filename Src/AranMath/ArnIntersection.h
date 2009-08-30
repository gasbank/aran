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

#include "ArnPlane.h"


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
ArnProjectPointToPlane(ArnVec3& p, const ArnVec3& point, const ArnPlane& plane)
{
	assert(&p != &point);
	const ArnVec3& n = plane.getNormal();
	ArnVec3 b = point - plane.getV0();
	float k = ArnVec3Dot(b, n);
	ArnVec3 a = b - n * k;
	p = a + plane.getV0();
	return k;
}


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
ARANMATH_API int
ArnLinePlaneIntersection(ArnVec3& p, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& plane)
{
	assert(&p != &p0 && &p != &p1);
	assert(ArnVec3IsNormalized(p1));
	const ArnVec3& n = plane.getNormal();
	const float perpTest = ArnVec3Dot(n, p1);
	if (-FLT_COMPARE_EPSILON <= perpTest && perpTest <= FLT_COMPARE_EPSILON)
	{
		return 0;
	}
	ArnVec3 x;
	const float k = ArnProjectPointToPlane(x, p0, plane);
	const float q = -k / perpTest;
	const ArnVec3 c = n * (-k) + p1 * (-q);
	p = x - c;
	return 1;
}

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
ARANMATH_API int
intersect3D_2Planes( ArnVec3& p0, ArnVec3& p1, const ArnPlane& Pn1, const ArnPlane& Pn2 )
{
	const ArnVec3& n1 = Pn1.getNormal();
	const ArnVec3& n2 = Pn2.getNormal();
    ArnVec3  u  = ArnVec3GetCrossProduct(n1, n2);
    float    ax = (u.x >= 0 ? u.x : -u.x);
    float    ay = (u.y >= 0 ? u.y : -u.y);
    float    az = (u.z >= 0 ? u.z : -u.z);

    // test if the two planes are parallel
    if ((ax+ay+az) < FLT_COMPARE_EPSILON) {       // Pn1 and Pn2 are near parallel
        // test if disjoint or coincide
        ArnVec3 v = Pn2.getV0() - Pn1.getV0();

        if (ArnVec3Dot(n1, v) == 0)         // Pn2.V0 lies in Pn1
            return 1;                   // Pn1 and Pn2 coincide
        else
            return 0;                   // Pn1 and Pn2 are disjoint
    }

    // Pn1 and Pn2 intersect in a line
    // first determine max abs coordinate of cross product
    int      maxc;                      // max coordinate
    if (ax > ay) {
        if (ax > az)
             maxc = 1;
        else maxc = 3;
    }
    else {
        if (ay > az)
             maxc = 2;
        else maxc = 3;
    }

    // next, to get a point on the intersect line
    // zero the max coord, and solve for the other two
    ArnVec3 iP;               // intersect point
    float    d1, d2;           // the constants in the 2 plane equations
    d1 = -ArnVec3Dot(n1, Pn1.getV0());  // note: could be pre-stored with plane
    d2 = -ArnVec3Dot(n2, Pn2.getV0());  // ditto

    switch (maxc) {            // select max coordinate
    case 1:                    // intersect with x=0
        iP.x = 0;
        iP.y = (d2*n1.z - d1*n2.z) / u.x;
        iP.z = (d1*n2.y - d2*n1.y) / u.x;
        break;
    case 2:                    // intersect with y=0
        iP.x = (d1*n2.z - d2*n1.z) / u.y;
        iP.y = 0;
        iP.z = (d2*n1.x - d1*n2.x) / u.y;
        break;
    case 3:                    // intersect with z=0
        iP.x = (d2*n1.y - d1*n2.y) / u.z;
        iP.y = (d1*n2.x - d2*n1.x) / u.z;
        iP.z = 0;
    }
    p0 = iP;
    p1 = iP + u;
    return 2;
}

// intersect3D_SegmentPlane(): intersect a segment and a plane
//    Input:  S = a segment, and Pn = a plane = {Point V0; Vector n;}
//    Output: *I0 = the intersect point (when it exists)
//    Return: 0 = disjoint (no intersection)
//            1 = intersection in the unique point *I0
//            2 = the segment lies in the plane
ARANMATH_API int
intersect3D_SegmentPlane(ArnVec3& I, const ArnVec3& p0, const ArnVec3& p1, const ArnPlane& Pn)
{
	ArnVec3		u = p1 - p0;
	ArnVec3    w = p0 - Pn.getV0();

	float     D = ArnVec3Dot(Pn.getNormal(), u);
	float     N = -ArnVec3Dot(Pn.getNormal(), w);

	if (fabs(D) < FLT_COMPARE_EPSILON) {          // segment is parallel to plane
		if (N == 0)                     // segment lies in plane
			return 2;
		else
			return 0;                   // no intersection
	}
	// they are not parallel
	// compute intersect param
	float sI = N / D;
	if (sI < 0 || sI > 1)
		return 0;                       // no intersection

	I = p0 + u*sI;                 // compute segment intersect point
	return 1;
}
