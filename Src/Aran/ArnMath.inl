static inline void
ArnMatrixRotationX( ArnMatrix* out, float rad	)
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline void
ArnMatrixRotationY( ArnMatrix* out, float rad	)
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline void
ArnMatrixRotationZ( ArnMatrix* out, float rad	)
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline BOOL
almostEqualFloat3(ArnVec3* pV1, ArnVec3* pV2)
{
	return ( fabsf( pV1->x - pV2->x ) < COMPARE_EPSILON )
		&& ( fabsf( pV1->y - pV2->y ) < COMPARE_EPSILON )
		&& ( fabsf( pV1->z - pV2->z ) < COMPARE_EPSILON );
}

static inline BOOL
almostEqualFloat3(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < COMPARE_EPSILON );
}
static inline BOOL
almostEqualFloat4(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < COMPARE_EPSILON )
		&& ( fabsf( floatArray1[3] - floatArray2[3] ) < COMPARE_EPSILON );
}

static inline void
ArnMatrixIdentity(ArnMatrix* out)
{
	*out = ArnMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

static inline void
ArnMatrixTranslation(ArnMatrix* out, float x, float y, float z)
{
	out->m[0][0] = 1;	out->m[0][1] = 0;	out->m[0][2] = 0;	out->m[0][3] = x;
	out->m[1][0] = 0;	out->m[1][1] = 1;	out->m[1][2] = 0;	out->m[1][3] = y;
	out->m[2][0] = 0;	out->m[2][1] = 0;	out->m[2][2] = 1;	out->m[2][3] = z;
	out->m[3][0] = 0;	out->m[3][1] = 0;	out->m[3][2] = 0;	out->m[3][3] = 1;
}

static inline void
ArnMatrixScaling(ArnMatrix* out, float x, float y, float z)
{
	out->m[0][0] = x;	out->m[0][1] = 0;	out->m[0][2] = 0;	out->m[0][3] = 0;
	out->m[1][0] = 0;	out->m[1][1] = y;	out->m[1][2] = 0;	out->m[1][3] = 0;
	out->m[2][0] = 0;	out->m[2][1] = 0;	out->m[2][2] = z;	out->m[2][3] = 0;
	out->m[3][0] = 0;	out->m[3][1] = 0;	out->m[3][2] = 0;	out->m[3][3] = 1;
}

static inline float
ArnToRadian(float deg)
{
	return float(ARN_PI/180*deg);
}

static inline float
ArnToDegree(float rad)
{
	return float(rad * (180.0 / ARN_PI));
}

static inline void
ArnVec3Transform(ArnVec4* out, const ArnVec3* vec, const ArnMatrix* mat)
{
#ifdef WIN32
	D3DXVec3Transform(reinterpret_cast<D3DXVECTOR4*>(out), reinterpret_cast<const D3DXVECTOR3*>(vec), reinterpret_cast<const D3DXMATRIX*>(mat));
#else
	out->x = mat->m[0][0] * vec->x + mat->m[0][1] * vec->y + mat->m[0][2] * vec->z + mat->m[0][3];
	out->y = mat->m[1][0] * vec->x + mat->m[1][1] * vec->y + mat->m[1][2] * vec->z + mat->m[1][3];
	out->z = mat->m[2][0] * vec->x + mat->m[2][1] * vec->y + mat->m[2][2] * vec->z + mat->m[2][3];
	out->w = mat->m[3][0] * vec->x + mat->m[3][1] * vec->y + mat->m[3][2] * vec->z + mat->m[3][3];
#endif
}

