#include "EmptyProjectPCH.h"
#include "UtilFunc.h"

#define NUM 4                          // リンク数

const dReal x[NUM]      = {0.00, 0.00, 0.00, 0.00};  // 重心 x
const dReal y[NUM]      = {0.00, 0.00, 0.00, 0.00};  // 重心 y
const dReal z[NUM]      = {0.05, 0.50, 1.50, 2.50};  // 重心 z
const dReal length[NUM] = {0.10, 0.90, 1.00, 1.00};  // 長さ
const dReal weight[NUM] = {9.00, 2.00, 2.00, 2.00};  // 質量
const dReal r[NUM]      = {0.20, 0.1, 0.1, 0.1};  // 半径
const dReal c_x[NUM]    = {0.00, 0.00, 0.00, 0.00};  // 関節中心点 x
const dReal c_y[NUM]    = {0.00, 0.00, 0.00, 0.00};  // 関節中心点 y
const dReal c_z[NUM]    = {0.00, 0.10, 1.00, 2.00};  // 関節中心点 z
const dReal axis_x[NUM] = {0, 0, 0, 0};              // 関節回転軸 x
const dReal axis_y[NUM] = {1, 1, 1, 1};              // 関節回転軸 y
const dReal axis_z[NUM] = {0, 0, 0, 0};              // 関節回転軸 z

MyObject rlink[NUM];                   // リンク
dReal THETA[NUM] = {0.0};             // 関節の目標角度[rad]

void renderArm(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX worldMat;
	D3DXMATRIX transMat, rotMat, scaleMat;
	D3DXMATRIX transZ;
	unsigned i;
	D3DXMatrixTranslation(&transZ, 0, 0, -0.5f);
	for (i = 0; i < NUM; i++)
	{
		const dReal* pos = dBodyGetPosition(rlink[i].body);
		const dReal* rot = dBodyGetRotation(rlink[i].body);
		D3DXMatrixTranslation(&transMat, (float)pos[0], (float)pos[1], (float)pos[2]);
		D3DXMatrixScaling(&scaleMat, 1.0f, 1.0f, -1.0f);
		OdeMatrixToDx(rotMat, rot);
		worldMat = rotMat * transMat * scaleMat;
		pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
		rlink[i].mesh->DrawSubset(0);
	}
}

HRESULT makeArmMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr = S_OK;
	unsigned i;
	for (i = 0; i < NUM; ++i)
		V(D3DXCreateBox(pd3dDevice, (float)r[i], (float)r[i], (float)length[i], &rlink[i].mesh, 0));
	return hr;
}
void releaseArmMesh()
{
	unsigned i;
	for (i = 0; i < NUM; ++i)
		SAFE_RELEASE(rlink[i].mesh);
}


/*** ロボットアームの生成 ***/
void  makeArmBody(dWorldID world, dSpaceID space)
{
	dMass mass;                                    // 質量パラメータ
	

	// リンクの生成
	for (int i = 0; i < NUM; i++) {
		rlink[i].body = dBodyCreate(world);
		dBodySetPosition(rlink[i].body, x[i], y[i], z[i]);
		dMassSetZero(&mass);
		dMassSetCapsuleTotal(&mass,weight[i],3,r[i],length[i]);
		dBodySetMass(rlink[i].body, &mass);
		rlink[i].geom = dCreateCapsule(space,r[i],length[i]);
		dGeomSetBody(rlink[i].geom,rlink[i].body);
	}

	// ジョイントの生成とリンクへの取り付け
	rlink[0].joint = dJointCreateFixed(world, 0);  // 固定ジョイント
	dJointAttach(rlink[0].joint, rlink[0].body, 0);
	dJointSetFixed(rlink[0].joint);
	for (int j = 1; j < NUM; j++) {
		rlink[j].joint = dJointCreateHinge(world, 0); // ヒンジジョイント
		dJointAttach(rlink[j].joint, rlink[j].body, rlink[j-1].body);
		dJointSetHingeAnchor(rlink[j].joint, c_x[j], c_y[j], c_z[j]);
		dJointSetHingeAxis(rlink[j].joint, axis_x[j], axis_y[j],axis_z[j]);
	}
}
/*** P制御 ***/
void PcontrolArm()
{
	dReal k =  10.0, fMax = 100.0;                   // 比例ゲイン，最大トルク
	unsigned j;
	for (j = 1; j < NUM; j++) {
		dReal tmp = dJointGetHingeAngle(rlink[j].joint);     // 関節角の取得

		dReal z = THETA[j] - tmp;                      // 残差

		dJointSetHingeParam(rlink[j].joint,dParamVel, k*z);  // 角速度の設定
		dJointSetHingeParam(rlink[j].joint,dParamFMax,fMax); // トルクの設定
	}
}

void KeyboardProcArm( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	switch (nChar) {
	case 'B': THETA[1] += M_PI/180; break;     // jキー
	case 'V': THETA[1] -= M_PI/180; break;     // fキー
	case 'N': THETA[2] += M_PI/180; break;     // kキー
	case 'C': THETA[2] -= M_PI/180; break;     // dキー
	case 'M': THETA[3] += M_PI/180; break;     // lキー
	case 'X': THETA[3] -= M_PI/180; break;     // sキー
	}
}