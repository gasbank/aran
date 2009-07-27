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
	float						getLength() const;
	float						getSquaredLength() const;
	void						getRotationMatrix(ArnMatrix* pOut) const;
	void						normalize();
	void						printEuler() const;
	void						printAxisAngle() const;
#ifdef WIN32
	const D3DXQUATERNION*		getConstDxPtr() const { return reinterpret_cast<const D3DXQUATERNION*>(this); }
	D3DXQUATERNION*				getDxPtr() { return reinterpret_cast<D3DXQUATERNION*>(this); }
	D3DXQUATERNION				getDx() const { return D3DXQUATERNION(x, y, z, w); }
#endif // #ifdef WIN32

	float						x;
	float						y;
	float						z;
	float						w;
};

typedef std::tr1::shared_ptr<ArnQuat> ArnQuatPtr;

ARANMATH_API ArnQuat CreateArnQuat(float x, float y, float z, float w);

#endif // ARNQUAT_H
