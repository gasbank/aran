#include "EmptyProjectPCH.h"

//void OdeMatrixToDx(D3DXMATRIX& ret, const dReal* rotMat)
//{
//	unsigned i, j;
//	for (i = 0; i < 3; ++i)
//	{
//		for (j = 0; j < 4 /* rotMat is 4x3 matrix */; ++j)
//		{
//			ret.m[j][i] = (float)rotMat[4*i + j];
//		}
//	}
//	ret.m[0][3] = 0;
//	ret.m[1][3] = 0;
//	ret.m[2][3] = 0;
//	ret.m[3][3] = 1.0f;
//}

void OdeMatrixToDx(D3DXMATRIX& ret, const dReal* R)
{
	ret.m[0][0] = (float)R[0];
	ret.m[0][1] = (float)R[4];
	ret.m[0][2] = (float)R[8];
	ret.m[0][3] = 0;
	ret.m[1][0] = (float)R[1];
	ret.m[1][1] = (float)R[5];
	ret.m[1][2] = (float)R[9];
	ret.m[1][3] = 0;
	ret.m[2][0] = (float)R[2];
	ret.m[2][1] = (float)R[6];
	ret.m[2][2] = (float)R[10];
	ret.m[2][3] = 0;
	ret.m[3][0] = 0;
	ret.m[3][1] = 0;
	ret.m[3][2] = 0;
	ret.m[3][3] = 1.0f;
}
