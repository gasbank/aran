#ifndef ARNMATRIX_H
#define ARNMATRIX_H

class ArnVec4;

class ARANMATH_API ArnMatrix
{
public:
									ArnMatrix();
									ArnMatrix(float m00, float m01, float m02, float m03,
												float m10, float m11, float m12, float m13,
												float m20, float m21, float m22, float m23,
												float m30, float m31, float m32, float m33);
									~ArnMatrix();
	ArnMatrix						transpose() const;
	ArnVec3							getColumnVec3(unsigned int zeroindex) const;
	ArnMatrix						operator * ( const ArnMatrix& rhs ) const;
	ArnVec4							operator * ( const ArnVec4& rhs ) const;
	ArnMatrix&						operator *= ( float f ); // Scalar multiplication
	ArnMatrix&						operator *= ( const ArnMatrix& rhs );
	ArnMatrix&						operator = (const ArnMatrix& rhs);
	void							getFormatString(char* buf) const;

	float							m[4][4]; // First index is row index, second index is column index!!!
};

ARANMATH_API ArnMatrix CreateArnMatrix(float m00, float m01, float m02, float m03,
						  float m10, float m11, float m12, float m13,
						  float m20, float m21, float m22, float m23,
						  float m30, float m31, float m32, float m33);
ARANMATH_API ArnMatrix ArnMatrixMultiply(const ArnMatrix& m0, const ArnMatrix m1);
ARANMATH_API ArnMatrix ArnMatrixMultiply(const ArnMatrix& m0, const ArnMatrix m1, const ArnMatrix m2);
ARANMATH_API ArnMatrix ArnMatrixTranspose(const ArnMatrix& m);
ARANMATH_API void ArnMatrixIdentity(ArnMatrix* out);
ARANMATH_API void ArnMatrixTranslation(ArnMatrix* out, float x, float y, float z);
ARANMATH_API void ArnMatrixScaling(ArnMatrix* out, float x, float y, float z);

#endif // ARNMATRIX_H
