#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
#include <tr1/memory>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <list>

//
// Boost C++
//
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#define foreach BOOST_FOREACH

#include "Macros.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnMath.h"
#include "ArnPlane.h"
#include "ArnIntersection.h"
#include "ArnCommonTypes.h"
#include "ArnConsts.h"


using namespace std;


void QuaternionTest()
{
	ArnQuat q;
	ArnVec3 euler;
	const float angles[] = { ARN_PI/4, -ARN_PI/4, ARN_PI/2, -ARN_PI/2, ARN_PI, -ARN_PI };
	const size_t angleCount = sizeof(angles) / sizeof(angles[0]);

	for (size_t i = 0; i < angleCount; ++i)
	{
		const float ang = angles[i];

		printf("-------- X axis %.3f rotation --------\n", ang);
		q = ArnQuat::createFromEuler(ang, 0, 0);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- Y axis %.3f rotation --------\n", ang);
		q = ArnQuat::createFromEuler(0, ang, 0);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- Z axis %.3f rotation --------\n", ang);
		q = ArnQuat::createFromEuler(0, 0, ang);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- X axis %.3f --> Y axis %.3f rotation --------\n", ang, ang);
		q = ArnQuat::createFromEuler(ang, ang, 0);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- Y axis %.3f --> Z axis %.3f rotation --------\n", ang, ang);
		q = ArnQuat::createFromEuler(0, ang, ang);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- X axis %.3f --> Z axis %.3f rotation --------\n", ang, ang);
		q = ArnQuat::createFromEuler(ang, 0, ang);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();

		printf("-------- X axis %.3f --> Y axis %.3f --> Z axis %.3f rotation --------\n", ang, ang, ang);
		q = ArnQuat::createFromEuler(ang, ang, ang);
		q.printAxisAngle();
		euler = ArnQuatToEuler(&q);
		euler.printFormatString();
	}
}

void PlaneIntersectionTest()
{
	ArnPlane p1(ArnVec3(0, 0, 0), ArnVec3(0, 1, 1), ArnVec3(0, 1, 9));
	ArnPlane p2(ArnVec3(0, 0, 0), ArnVec3(0, 1, 0), ArnVec3(0, 1, 1));

	ArnVec3 v0, v1;
	int c = intersect3D_2Planes(v0, v1, p1, p2);
	if (c == 0)
	{
		printf("Parallel planes - no intersection points.\n");
	}
	else if (c == 1)
	{
		printf("Almost(completely) the sasme planes - no intersection points defined.\n");
	}
	else if (c == 2)
	{
		printf("Line intersection with v0(%.2f, %.2f, %.2f) and v1(%.2f, %.2f, %.2f).\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

void PointToPlaneProjectionTest()
{
	ArnPlane p1 = ArnPlane(ArnVec3(1, 0, 0), ArnVec3(0, 1, 0), ArnVec3(0, 0, 1));
	ArnVec3 point(5, 5, 5);
	ArnVec3 p;
	ArnProjectPointToPlane(p, point, p1);
	printf("Projected point %.2f, %.2f, %.2f\n", p.x, p.y, p.z);

	ArnVec3 line(5, 5, 5);
	ArnVec3 linedir(-1, -1, -1);
	linedir /= ArnVec3Length(linedir);
	ArnVec3 rayPlaneIn;
	int intersected = ArnLinePlaneIntersection(rayPlaneIn, line, linedir, p1);
	if (intersected)
		printf("Ray-plane intersection point %.2f, %.2f, %.2f\n", rayPlaneIn.x, rayPlaneIn.y, rayPlaneIn.z);
}

void Frame()
{
	ArnMatrix A, AB_T, AC_T;
	ArnMatrixIdentity(&A);
	A.printFrameInfo();

	ArnQuat qB(ArnQuat::createFromEuler(0, 0, 0));
	ArnVec3 tB(0, 0, 5);
	ArnMatrixTransformation(&AB_T, 0, 0, &ArnConsts::ARNVEC3_ONE, 0, &qB, &tB);
	AB_T.printFrameInfo();

	ArnQuat qC(ArnQuat::createFromEuler(ArnToRadian(45.0), 0, 0));
	ArnVec3 tC(2, 3, 4);
	ArnMatrixTransformation(&AC_T, 0, 0, &ArnConsts::ARNVEC3_ONE, 0, &qC, &tC);
	AC_T.printFrameInfo();

	ArnVec3 P_A(10, 20, 30);
	ArnVec3 P_B, P_C;
	ArnMatrix BA_T, CA_T;
	ArnMatrixInverse(&BA_T, 0, &AB_T);
	ArnMatrixInverse(&CA_T, 0, &AC_T);
	ArnVec3TransformCoord(&P_B, &P_A, &BA_T);
	ArnVec3TransformCoord(&P_C, &P_A, &CA_T);

	P_B.printFormatString();
	P_C.printFormatString();

}

int main()
{
	//QuaternionTest();
	//PlaneIntersectionTest();
	//PointToPlaneProjectionTest();
	Frame();
    cout << "The end of Aran library math test." << endl;
    return 0;
}
