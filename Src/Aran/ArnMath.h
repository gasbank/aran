#pragma once

class ArnMath
{
public:
	static D3DXVECTOR3 QuatToEuler(const D3DXQUATERNION* quat);
	static D3DXVECTOR3 Vec3RadToDeg(const D3DXVECTOR3* vec3);
private:
	ArnMath(void);
	~ArnMath(void);
};