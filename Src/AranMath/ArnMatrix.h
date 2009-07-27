#ifndef ARNMATRIX_H
#define ARNMATRIX_H

struct D3DXMATRIX;
class ArnVec4;

class ARANMATH_API ArnMatrix
{
public:
									ArnMatrix();
									ArnMatrix(float m00, float m01, float m02, float m03,
												float m10, float m11, float m12, float m13,
												float m20, float m21, float m22, float m23,
												float m30, float m31, float m32, float m33);
#ifdef WIN32
									ArnMatrix(const D3DXMATRIX* dxmat);
#endif
									~ArnMatrix();
	ArnMatrix						transpose() const;
	ArnMatrix						operator * ( const ArnMatrix& rhs ) const;
	ArnVec4							operator * ( const ArnVec4& rhs ) const;
	ArnMatrix&						operator *= ( float f ); // Scalar multiplication
	ArnMatrix&						operator *= ( const ArnMatrix& rhs );
	ArnMatrix&						operator = (const ArnMatrix& rhs);
#ifdef WIN32
	ArnMatrix&						operator = (const D3DMATRIX& rhs);
	const D3DXMATRIX*				getConstDxPtr() const { return reinterpret_cast<const D3DXMATRIX*>(this); }
	D3DXMATRIX*						getDxPtr() { return reinterpret_cast<D3DXMATRIX*>(this); }
#endif
	void							getFormatString(char* buf) const;

	float							m[4][4]; // First index is row index, second index is column index!!!
};

typedef std::tr1::shared_ptr<ArnMatrix> ArnMatrixPtr;

ARANMATH_API ArnMatrix CreateArnMatrix(float m00, float m01, float m02, float m03,
						  float m10, float m11, float m12, float m13,
						  float m20, float m21, float m22, float m23,
						  float m30, float m31, float m32, float m33);
ARANMATH_API ArnMatrix ArnMatrixMultiply(const ArnMatrix& m0, const ArnMatrix m1);
ARANMATH_API ArnMatrix ArnMatrixMultiply(const ArnMatrix& m0, const ArnMatrix m1, const ArnMatrix m2);
ARANMATH_API ArnMatrix ArnMatrixTranspose(const ArnMatrix& m);
#ifdef WIN32
ARANMATH_API const D3DXMATRIX* ArnMatrixGetConstDxPtr(const ArnMatrix& mat);
ARANMATH_API D3DXMATRIX* ArnMatrixGetDxPtr(ArnMatrix& mat);
#endif

#endif // ARNMATRIX_H
