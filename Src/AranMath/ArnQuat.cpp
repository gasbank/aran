#include "AranMathPCH.h"
#include "LinearR3.h"
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


namespace AranMath
{ 
  Quaternion::Quaternion()
    : w(1), x(0), y(0), z(0) {
  }
  Quaternion::Quaternion(double _w, double _x, double _y, double _z)
    : w(_w), x(_x), y(_y), z(_z) {
  }

  Quaternion::Quaternion( const ArnVec3 &axis, double angle )
  {
    ArnVec3 axis_normalized;
    ArnVec3Normalize(&axis_normalized, &axis);
    double s = sin(angle/2);
    w = cos(angle/2);
    x = axis_normalized.x * s;
    y = axis_normalized.y * s;
    z = axis_normalized.z * s;
  }

  Quaternion::Quaternion( const VectorR3 &axis, double angle )
  {
    VectorR3 axis_normalized(axis);
    axis_normalized.Normalize();
    double s = sin(angle/2);
    w = cos(angle/2);
    x = axis_normalized.x * s;
    y = axis_normalized.y * s;
    z = axis_normalized.z * s;
  }

  void Quaternion::normalize() {
    const double len = length();
    w /= len;
    x /= len;
    y /= len;
    z /= len;
  }
  // Matrix version of quaternion multiplication
  // q*v4 --> [q]v4
  ArnMatrix Quaternion::get_matrix_mult() const {
    return ArnMatrix(
      w, -z, y, x,
      z, w, -x, y,
      -y, x, w, z,
      -x, -y, -z, w);
  }

  Quaternion Quaternion::inverse() const {
    Quaternion q = get_conj();
    const double lensq = q.length_squared();
    q.w /= lensq;
    q.x /= lensq;
    q.y /= lensq;
    q.z /= lensq;
    return q;
  }

  Quaternion operator * (const Quaternion &lhs, const Quaternion &rhs) {
    const double w = lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z;
    const double x = lhs.w*rhs.x + lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y;
    const double y = lhs.w*rhs.y + lhs.y*rhs.w + lhs.z*rhs.x - lhs.x*rhs.z;
    const double z = lhs.w*rhs.z + lhs.z*rhs.w + lhs.x*rhs.y - lhs.y*rhs.x;
    return Quaternion(w, x, y, z);
  }

  Quaternion& operator*=( Quaternion &lhs, const double s )
  {
    lhs.w *= s;
    lhs.x *= s;
    lhs.y *= s;
    lhs.z *= s;
    return lhs;
  }
  Quaternion& operator/=( Quaternion &lhs, const double s )
  {
    lhs.w /= s;
    lhs.x /= s;
    lhs.y /= s;
    lhs.z /= s;
    return lhs;
  }
  Quaternion operator + (const Quaternion &lhs, const Quaternion &rhs)
  {
    return Quaternion(lhs.w+rhs.w, lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
  }
  Quaternion operator - (const Quaternion &lhs, const Quaternion &rhs)
  {
    return Quaternion(lhs.w-rhs.w, lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
  }
  Quaternion operator * (const Quaternion &lhs, const double &s)
  {
    return Quaternion(lhs.w*s, lhs.x*s, lhs.y*s, lhs.z*s);
  }
  Quaternion operator / (const Quaternion &lhs, const double &s)
  {
    return Quaternion(lhs.w/s, lhs.x/s, lhs.y/s, lhs.z/s);
  }

  std::ostream &operator <<(std::ostream &s, const Quaternion &q)
  {
    s << q.w << " [" << q.x << ", " << q.y << ", " << q.z << "]";
    return s;
  }
}
