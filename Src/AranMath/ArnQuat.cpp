#include "AranMathPCH.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnMatrix.h"
#include "ArnMath.h"
#include "ArnQuat.h"

ArnQuat
ArnQuat::createFromEuler(float rx, float ry, float rz)
{
	/*
	const double bank = rx;
	const double heading = ry;
	const double attitude = rz;
	const double c1 = cos(heading/2);
    const double s1 = sin(heading/2);
    const double c2 = cos(attitude/2);
    const double s2 = sin(attitude/2);
    const double c3 = cos(bank/2);
    const double s3 = sin(bank/2);
    const double c1c2 = c1*c2;
    const double s1s2 = s1*s2;

    const float _w = (float)(c1c2*c3 - s1s2*s3);
  	const float _x = (float)(c1c2*s3 + s1s2*c3);
	const float _y = (float)(s1*c2*c3 + c1*s2*s3);
	const float _z = (float)(c1*s2*c3 - s1*c2*s3);
	return ArnQuat(_x, _y, _z, _w);
	*/

	ArnVec3 v3(rx, ry, rz);
	return ArnEulerToQuat(&v3);
}

ArnQuat
ArnQuat::createFromRotAxis(float angle, float rx, float ry, float rz)
{
	ArnQuat q;
	ArnVec3 r(rx, ry, rz);
	r /= ArnVec3Length(r);
	ArnQuaternionRotationAxis(&q, &r, angle);
	return q;
}

void
ArnQuat::getRotationMatrix(ArnMatrix* pOut) const
{
	ArnMatrixRotationQuaternion(pOut, this);
	return;
}

float
ArnQuat::getSquaredLength() const
{
	return w*w + x*x + y*y+ z*z;
}

float
ArnQuat::getLength() const
{
	return sqrtf(getSquaredLength());
}

ArnQuat
ArnQuat::computeInverse() const
{
	float len = getSquaredLength();
	return ArnQuat(-x/len, -y/len, -z/len, w/len);
}

void
ArnQuat::normalize()
{
	float len = getLength();
	w /= len;
	x /= len;
	y /= len;
	z /= len;
}

void
ArnQuat::printEuler() const
{
	ARN_THROW_REMOVE_FUNCTION_ERROR
	/*
	ArnVec3 v = QuatToEuler(this);
	printf("Quat: euler "); v.printFormatString();
	*/
}

void
ArnQuat::printAxisAngle() const
{
	ArnVec3 axis;
	float angle;
	ArnQuatToAxisAngle(&axis, &angle, this);
	printf("Quat: angle/axis %.3f", angle); axis.printFormatString();
}

ArnQuat
ArnQuat::operator * ( const ArnQuat& rhs) const
{
	ArnQuat ret;
	ArnQuat lhs(*this);
	/*
	ret.x = lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z - lhs.w*rhs.w;
	ret.y = lhs.x*rhs.y + rhs.x*lhs.y + lhs.z*rhs.w - lhs.w*rhs.z;
	ret.z = lhs.x*rhs.z - lhs.y*rhs.w + lhs.z*rhs.x + lhs.w*rhs.y;
	ret.w = lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y + lhs.w*rhs.x;
	*/
	ret.w = lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z;
	ret.x = lhs.w*rhs.x + lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y;
	ret.y = lhs.w*rhs.y + lhs.y*rhs.w + lhs.z*rhs.x - lhs.x*rhs.z;
	ret.z = lhs.w*rhs.z + lhs.z*rhs.w + lhs.x*rhs.y - lhs.y*rhs.x;
	return ret;
}

bool
ArnQuat::operator == ( const ArnQuat& rhs) const
{
	return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
}

bool
ArnQuat::operator != ( const ArnQuat& rhs) const
{
	return (!operator == (rhs));
}

ArnQuat&
ArnQuat::operator /= ( float f )
{
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////

ARANMATH_API ArnQuat CreateArnQuat( float x, float y, float z, float w )
{
	return ArnQuat(x, y, z, w);
}

ARANMATH_API ArnQuat* ArnQuatAssign_ScalarLast( ArnQuat* q, const float* v )
{
	q->x = v[0]; // Scalar component
	q->y = v[1];
	q->z = v[2];
	q->w = v[3];
	return q;
}

ARANMATH_API ArnQuat* ArnQuatAssign_ScalarLast( ArnQuat* q, const double* v )
{
	q->x = static_cast<float>(v[0]); // Scalar component
	q->y = static_cast<float>(v[1]);
	q->z = static_cast<float>(v[2]);
	q->w = static_cast<float>(v[3]);
	return q;
}

ARANMATH_API ArnQuat* ArnQuatAssign_ScalarFirst( ArnQuat* q, const float* v )
{
	q->x = v[1];
	q->y = v[2];
	q->z = v[3];
	q->w = v[0]; // Scalar component
	return q;
}

ARANMATH_API ArnQuat* ArnQuatAssign_ScalarFirst( ArnQuat* q, const double* v )
{
	q->x = static_cast<float>(v[1]);
	q->y = static_cast<float>(v[2]);
	q->z = static_cast<float>(v[3]);
	q->w = static_cast<float>(v[0]); // Scalar component
	return q;
}
