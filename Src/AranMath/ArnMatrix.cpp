#include "AranMathPCH.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnMatrix.h"
#include "ArnQuat.h"
#include "ArnMath.h"

ArnMatrix::ArnMatrix()
{
}

ArnMatrix::ArnMatrix( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
{
	m[0][0] = m00;		m[0][1] = m01;		m[0][2] = m02;		m[0][3] = m03;
	m[1][0] = m10;		m[1][1] = m11;		m[1][2] = m12;		m[1][3] = m13;
	m[2][0] = m20;		m[2][1] = m21;		m[2][2] = m22;		m[2][3] = m23;
	m[3][0] = m30;		m[3][1] = m31;		m[3][2] = m32;		m[3][3] = m33;
}

ArnMatrix::~ArnMatrix()
{
}

void ArnMatrix::getFormatString(char* buf) const
{
	assert(buf);
	sprintf(buf,
			"[(%.3f %.3f %.3f %.3f) (%.3f %.3f %.3f %.3f) (%.3f %.3f %.3f %.3f) (%.3f %.3f %.3f %.3f)]",
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
}

ArnMatrix ArnMatrix::operator*( const ArnMatrix& rhs) const
{
	ArnMatrix ret;
	memset(&ret, 0, sizeof(float)*4*4);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			// m[i][j] = 0; is not necessary.
			for (int k = 0; k < 4; ++k)
			{
				ret.m[i][j] += m[i][k] * rhs.m[k][j];
			}
		}
	}
	return ret;
}

ArnVec4 ArnMatrix::operator*( const ArnVec4& rhs ) const
{
	ArnVec4 ret;
	ret.x = m[0][0] * rhs.x + m[0][1] * rhs.y + m[0][2] * rhs.z + m[0][3] * rhs.w;
	ret.y = m[1][0] * rhs.x + m[1][1] * rhs.y + m[1][2] * rhs.z + m[1][3] * rhs.w;
	ret.z = m[2][0] * rhs.x + m[2][1] * rhs.y + m[2][2] * rhs.z + m[2][3] * rhs.w;
	ret.w = m[3][0] * rhs.x + m[3][1] * rhs.y + m[3][2] * rhs.z + m[3][3] * rhs.w;
	return ret;
}

ArnMatrix& ArnMatrix::operator *= ( float f )
{
	m[0][0] *= f; m[0][1] *= f; m[0][2] *= f; m[0][3] *= f;
	m[1][0] *= f; m[1][1] *= f; m[1][2] *= f; m[1][3] *= f;
	m[2][0] *= f; m[2][1] *= f; m[2][2] *= f; m[2][3] *= f;
	m[3][0] *= f; m[3][1] *= f; m[3][2] *= f; m[3][3] *= f;
	return *this;
}

ArnMatrix& ArnMatrix::operator *= ( const ArnMatrix& rhs )
{
	// Since this operator equals this = this * rhs,
	// we copy this to a local variable 'left' and accumulate
	// multiplicated values into this.
	float lhs[4][4];
	memcpy(lhs, m, sizeof(float)*4*4);
	memset(m, 0, sizeof(float)*4*4);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			// m[i][j] = 0; is not necessary.
			for (int k = 0; k < 4; ++k)
			{
				m[i][j] += lhs[i][k] * rhs.m[k][j];
			}
		}
	}
	return *this;
}

ArnMatrix ArnMatrix::transpose() const
{
	return ArnMatrix(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
}

ArnMatrix& ArnMatrix::operator = (const ArnMatrix& rhs)
{
	if (this == &rhs)
		return *this;
	else
	{
		memcpy(m, rhs.m, sizeof(float)*4*4);
		return *this;
	}
}

ArnVec3 ArnMatrix::getColumnVec3( unsigned int zeroindex ) const
{
	assert(zeroindex < 4);
	return ArnVec3(m[0][zeroindex], m[1][zeroindex], m[2][zeroindex]);
}

void ArnMatrix::printFrameInfo() const
{
	printf("%6.3f %6.3f %6.3f %6.3f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
	printf("%6.3f %6.3f %6.3f %6.3f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
	printf("%6.3f %6.3f %6.3f %6.3f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
	printf("%6.3f %6.3f %6.3f %6.3f\n", m[3][0], m[3][1], m[3][2], m[3][3]);
	ArnVec3 trans, scale;
	ArnQuat q;
	ArnMatrixDecompose(&scale, &q, &trans, this);
	ArnVec3 rotEuler(ArnQuatToEuler(&q));
	printf("Scaling             : %6.3f %6.3f %6.3f\n", scale.x, scale.y, scale.z);
	printf("Rotation in Degrees : %6.3f %6.3f %6.3f\n", ArnToDegree(rotEuler.x), ArnToDegree(rotEuler.y), ArnToDegree(rotEuler.z));
	printf("Translation         : %6.3f %6.3f %6.3f\n", trans.x, trans.y, trans.z);
}

//////////////////////////////////////////////////////////////////////////

ArnMatrix CreateArnMatrix( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 )
{
	return ArnMatrix(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

ArnMatrix ArnMatrixMultiply( const ArnMatrix& m0, const ArnMatrix m1 )
{
	return m0 * m1;
}

ArnMatrix ArnMatrixMultiply( const ArnMatrix& m0, const ArnMatrix m1, const ArnMatrix m2 )
{
	return m0 * m1 * m2;
}

ArnMatrix ArnMatrixTranspose( const ArnMatrix& m )
{
	return m.transpose();
}

void ArnMatrixIdentity(ArnMatrix* out)
{
	*out = CreateArnMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

void ArnMatrixTranslation(ArnMatrix* out, float x, float y, float z)
{
	out->m[0][0] = 1;	out->m[0][1] = 0;	out->m[0][2] = 0;	out->m[0][3] = x;
	out->m[1][0] = 0;	out->m[1][1] = 1;	out->m[1][2] = 0;	out->m[1][3] = y;
	out->m[2][0] = 0;	out->m[2][1] = 0;	out->m[2][2] = 1;	out->m[2][3] = z;
	out->m[3][0] = 0;	out->m[3][1] = 0;	out->m[3][2] = 0;	out->m[3][3] = 1;
}

void ArnMatrixScaling(ArnMatrix* out, float x, float y, float z)
{
	out->m[0][0] = x;	out->m[0][1] = 0;	out->m[0][2] = 0;	out->m[0][3] = 0;
	out->m[1][0] = 0;	out->m[1][1] = y;	out->m[1][2] = 0;	out->m[1][3] = 0;
	out->m[2][0] = 0;	out->m[2][1] = 0;	out->m[2][2] = z;	out->m[2][3] = 0;
	out->m[3][0] = 0;	out->m[3][1] = 0;	out->m[3][2] = 0;	out->m[3][3] = 1;
}
