#include "AranMathPCH.h"
#include "ArnIntersection.h"
#include "ArnPlane.h"
#include "ArnMath.h"

float
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

int
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

int
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

int
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
