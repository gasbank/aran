#include "AranPCH.h"
#include "ArnMath.h"

ArnMath::ArnMath(void)
{
}

ArnMath::~ArnMath(void)
{
}

D3DXVECTOR3 ArnMath::QuatToEuler( const D3DXQUATERNION* quat )
{
	double test = quat->x*quat->y + quat->z*quat->w;
	double heading, attitude, bank;
	if (test > 0.499)
	{
		// singularity at north pole
		heading = 2 * atan2(quat->x,quat->w);
		attitude = D3DX_PI / 2;
		bank = 0;
	}
	else if (test < -0.499)
	{
		// singularity at south pole
		heading = -2 * atan2(quat->x,quat->w);
		attitude = -D3DX_PI / 2;
		bank = 0;
	}
	else
	{
		double sqx = quat->x*quat->x;
		double sqy = quat->y*quat->y;
		double sqz = quat->z*quat->z;
		heading = atan2((double)(2*quat->y*quat->w-2*quat->x*quat->z) , double(1 - 2*sqy - 2*sqz));
		attitude = asin(2*test);
		bank = atan2((double)(2*quat->x*quat->w-2*quat->y*quat->z) , double(1 - 2*sqx - 2*sqz));
	}
	return D3DXVECTOR3((float)heading, (float)attitude, (float)bank);
}

D3DXVECTOR3 ArnMath::Vec3RadToDeg( const D3DXVECTOR3* vec3 )
{
	return D3DXVECTOR3(D3DXToDegree(vec3->x), D3DXToDegree(vec3->y), D3DXToDegree(vec3->z));
}

DWORD ArnMath::Float4ColorToDword( const D3DCOLORVALUE* cv )
{
	return ((int)(0) << 24)
		| ((int)(cv->b * 255) << 16)
		| ((int)(cv->g * 255) <<  8)
		| ((int)(cv->r * 255) <<  0);
}

D3DXQUATERNION ArnMath::EulerToQuat( const D3DXVECTOR3* vec3 )
{
	// Assuming the angles are in radians.
	double bank		= vec3->x;
	double heading	= vec3->y;
	double attitude	= vec3->z;

	double c1 = cos(heading/2);
	double s1 = sin(heading/2);
	double c2 = cos(attitude/2);
	double s2 = sin(attitude/2);
	double c3 = cos(bank/2);
	double s3 = sin(bank/2);
	double c1c2 = c1*c2;
	double s1s2 = s1*s2;
	float w = (float)(c1c2*c3 - s1s2*s3);
	float x = (float)(c1c2*s3 + s1s2*c3);
	float y = (float)(s1*c2*c3 + c1*s2*s3);
	float z = (float)(c1*s2*c3 - s1*c2*s3);

	return D3DXQUATERNION(x, y, z, w);
}