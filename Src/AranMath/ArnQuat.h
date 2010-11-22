#ifndef ARNQUAT_H
#define ARNQUAT_H

class ArnMatrix;
class VectorR3;

class ARANMATH_API ArnQuat
{
public:
								ArnQuat() : x(0), y(0), z(0), w(1.0f) {}
								ArnQuat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
								ArnQuat(const ArnQuat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
								~ArnQuat() {}
	ArnQuat						operator * ( const ArnQuat& rhs) const;
	ArnQuat&					operator /= ( float f );
	bool						operator == ( const ArnQuat& rhs) const;
	bool						operator != ( const ArnQuat& rhs) const;
	static ArnQuat				createFromEuler(float rx, float ry, float rz);
	static ArnQuat				createFromRotAxis(float angle, float rx, float ry, float rz);
	float						getLength() const;
	float						getSquaredLength() const;
	void						getRotationMatrix(ArnMatrix* pOut) const;
	void						normalize();
	void						printEuler() const;
	void						printAxisAngle() const;
	ArnQuat						computeInverse() const;

	float						x;
	float						y;
	float						z;
	float						w;
};

ARANMATH_API ArnQuat CreateArnQuat(float x, float y, float z, float w);
ARANMATH_API ArnQuat* ArnQuatAssign_ScalarLast(ArnQuat* q, const float* v);
ARANMATH_API ArnQuat* ArnQuatAssign_ScalarLast(ArnQuat* q, const double* v);
ARANMATH_API ArnQuat* ArnQuatAssign_ScalarFirst(ArnQuat* q, const float* v);
ARANMATH_API ArnQuat* ArnQuatAssign_ScalarFirst(ArnQuat* q, const double* v);

namespace AranMath
{
  struct ARANMATH_API Quaternion {
    Quaternion();
    Quaternion(double _w, double _x, double _y, double _z);
    Quaternion(const ArnVec3 &axis, double angle);
    Quaternion(const VectorR3 &axis, double angle);
    void normalize();
    double Quaternion::length() const {
      return sqrt(length_squared());
    }
    double Quaternion::length_squared() const {
      return w*w + x*x + y*y + z*z;
    }
    Quaternion inverse() const;
    Quaternion Quaternion::get_conj() const {
      return Quaternion(w, -x, -y, -z);
    }    // Matrix version of quaternion multiplication
    // q*v4 --> [q]v4
    ArnMatrix get_matrix_mult() const;
    double w, x, y, z;
  };
  ARANMATH_API Quaternion operator * (const Quaternion &lhs, const Quaternion &rhs);
  ARANMATH_API Quaternion& operator *= (Quaternion &lhs, const double s);
  ARANMATH_API Quaternion& operator /= (Quaternion &lhs, const double s);
  ARANMATH_API Quaternion operator + (const Quaternion &lhs, const Quaternion &rhs);
  ARANMATH_API Quaternion operator - (const Quaternion &lhs, const Quaternion &rhs);
  ARANMATH_API Quaternion operator * (const Quaternion &lhs, const double &s);
  ARANMATH_API Quaternion operator / (const Quaternion &lhs, const double &s);
  ARANMATH_API std::ostream &operator <<(std::ostream &s, const Quaternion &q);
}

#endif // ARNQUAT_H
