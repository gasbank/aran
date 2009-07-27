#include <iostream>
#include <stdexcept>
#include <tr1/memory>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "Macros.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnMath.h"

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

int main()
{
	QuaternionTest();
    cout << "The end of Aran library math test." << endl;
    return 0;
}
