#ifndef ARNQUAT_H
#define ARNQUAT_H

class ArnMatrix;

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

#endif // ARNQUAT_H
