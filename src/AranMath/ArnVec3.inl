float
ArnVec3::operator[]( int i ) const
{
	if (i == 0) return x;
	else if (i == 1) return y;
	else if (i == 2) return z;
	else
	{
		abort();
		return 0;
	}
}

ArnVec3
ArnVec3::operator*( float f ) const
{
	return ArnVec3(x * f, y * f, z * f);
}

ArnVec3&
ArnVec3::operator/=( float f )
{
	x /= f; y /= f; z /= f;
	return *this;
}

ArnVec3
ArnVec3::operator-( const ArnVec3& v ) const
{
	return ArnVec3(x - v.x, y - v.y, z - v.z);
}

ArnVec3&
ArnVec3::operator+=( const ArnVec3& v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

ArnVec3&
ArnVec3::operator-=( const ArnVec3& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

bool
ArnVec3::operator == ( const ArnVec3& v ) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool
ArnVec3::operator != ( const ArnVec3& v ) const
{
	return !(this->operator ==(v));
}

void
ArnVec3::getFormatString( char* buf, size_t bufSize ) const
{
	assert(buf);
	sprintf(buf, "(x %6.3f, y %6.3f, z %6.3f)", x, y, z);
	assert(strlen(buf) < bufSize);
}

void
ArnVec3::printFormatString() const
{
	printf("(x %6.3f, y %6.3f, z %6.3f)\n", x, y, z);
}

ArnVec3
ArnVec3::operator / (float f) const
{
	return ArnVec3(x/f, y/f, z/f);
}

ArnVec3&
ArnVec3::operator*=( float f )
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

template <typename T> ArnVec3
ArnVec3::operator + ( const T& v ) const
{
	return ArnVec3(x + v.x, y + v.y, z + v.z);
}

template <typename T> ArnVec3
ArnVec3::operator - ( const T& v ) const
{
	return ArnVec3(x - v.x, y - v.y, z - v.z);
}

template <typename T> ArnVec3&
ArnVec3::operator = ( const T& v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

inline void
ArnVec3::set(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

template<typename V1, typename V2> void
ArnVec3Assign(V1& v1, const V2& v2)
{
	v1.x = v2.x;
	v1.y = v2.y;
	v1.z = v2.z;
}
