static inline void
ArnMatrixRotationX( ArnMatrix* out, float rad )
{
	out = out;
	rad = rad;
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline void
ArnMatrixRotationY( ArnMatrix* out, float rad	)
{
	out = out;
	rad = rad;
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline void
ArnMatrixRotationZ( ArnMatrix* out, float rad	)
{
	out = out;
	rad = rad;
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

static inline BOOL
almostEqualFloat3(ArnVec3* pV1, ArnVec3* pV2)
{
	return ( fabsf( pV1->x - pV2->x ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( pV1->y - pV2->y ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( pV1->z - pV2->z ) < FLT_COMPARE_EPSILON );
}

static inline BOOL
almostEqualFloat3(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < FLT_COMPARE_EPSILON );
}
static inline BOOL
almostEqualFloat4(float* floatArray1, float* floatArray2)
{
	return ( fabsf( floatArray1[0] - floatArray2[0] ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( floatArray1[1] - floatArray2[1] ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( floatArray1[2] - floatArray2[2] ) < FLT_COMPARE_EPSILON )
		&& ( fabsf( floatArray1[3] - floatArray2[3] ) < FLT_COMPARE_EPSILON );
}

static inline float
ArnToRadian(float deg)
{
	return float(ARN_PI/180.0f*deg);
}

static inline double
ArnToRadian(double deg)
{
	return double(ARN_PI/180.0*deg);
}

static inline double
ArnToDegree(double rad)
{
	return double(rad * (180.0 / ARN_PI));
}

static inline float
ArnToDegree(float rad)
{
	return float(rad * (180.0 / ARN_PI));
}

static inline float
ArnVec3GetLength(const ArnVec3& v)
{
	return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline float
ArnVec3Dot(const ArnVec3& v1, const ArnVec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static inline float
ArnVec3Dot(const ArnVec3* v1, const ArnVec3* v2)
{
	return ArnVec3Dot(*v1, *v2);
}
static inline ArnVec3
ArnVec3GetCrossProduct(const ArnVec3& va, const ArnVec3& vb)
{
	return CreateArnVec3(va.y * vb.z - va.z * vb.y, va.z * vb.x - va.x * vb.z, va.x * vb.y - va.y * vb.x);
}