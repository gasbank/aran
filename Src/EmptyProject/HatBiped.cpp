#include "EmptyProjectPCH.h"
#include "HatBiped.h"
#include "UtilFunc.h"
#include "ArnMath.h"

/*

/-----------\
|           |
|           |  Head/Arms/Torso [HAT]
|           |
|           |
\-----------/
  (*)   (*)
 |---| |---|
 |   | |   |
 |   | |   |   Groin
 |   | |   |
 |---| |---|
  (*)   (*)
 |---| |---|
 |   | |   |   Calf
 |   | |   |
 |---| |---|
  (*)   (*)
  /-\   /-\    Foot
  \-/   \-/

*/

SwingLegEnum g_swingLeg = SLE_RIGHT;
unsigned int g_stateStep = 0;
extern int g_stepBiped[2];

MyObject g_objects[BE_COUNT];
MyObject g_joints[JE_COUNT];
MyObject g_comProxy;

dJointID			g_trunkFixJoint;
// Mass distribution [kg]
const dReal torso_m = 70.0;
const dReal groin_m = 5.0;
const dReal calf_m = 4.0;
const dReal foot_m = 1.0;
//const dReal torso_m = 0.1;
//const dReal groin_m = 0.2;
//const dReal calf_m = 0.3;
//const dReal foot_m = 2;

// Length, radius [m]
// Torso and foot are box shaped
const dReal torso_w = 0.48;
const dReal torso_h = 0.8;
const dReal torso_d = 0.2;

const dReal foot_w = 0.15;
const dReal foot_h = 0.095; // Originally, it was 0.1. But I want provide some margin between feet and ground.
const dReal foot_d = 0.3;

// Groin and calf are capsule shaped
const dReal groin_r = 0.15/2;
const dReal groin_h = 0.45;

const dReal calf_r = 0.15/2;
const dReal calf_h = 0.45;

const dReal leg_h = groin_h + calf_h;

extern dGeomID      					g_wall, g_wall2;
dJointID						g_wallTrunkJoint[2];
extern bool g_footContact[2];
double bodyCenters[BE_COUNT][3] = 
{
	{ 0,           foot_h + leg_h + torso_h/2,  0                  }, // HAT
	{ torso_w/4,   foot_h + calf_h + groin_h/2, 0                  }, // GROIN_R
	{ -torso_w/4,  foot_h + calf_h + groin_h/2, 0                  }, // GROIN_L
	{ torso_w/4,   foot_h + calf_h/2,           0                  }, // CALF_R
	{ -torso_w/4,  foot_h + calf_h/2,           0                  }, // CALF_L
	{ torso_w/4,   foot_h/2,                    foot_d/2 - calf_r  }, // FOOT_R
	{ -torso_w/4,  foot_h/2,                    foot_d/2 - calf_r  } // FOOT_L
};
double bodyRadiusAndHeight[BE_COUNT][2] =
{
	{ -1, -1 },
	{ groin_r, groin_h - groin_r*2 },
	{ groin_r, groin_h - groin_r*2 },
	{ calf_r, calf_h - calf_r*2 },
	{ calf_r, calf_h - calf_r*2 },
	{ -1, -1 },
	{ -1, -1 }
};
double bodyWeights[BE_COUNT] = { torso_m, groin_m, groin_m, calf_m, calf_m, foot_m, foot_m };
double jointCenters[JE_COUNT][3] =
{
	{ torso_w/4,  foot_h + leg_h,  0   },
	{ -torso_w/4, foot_h + leg_h,  0   },
	{ torso_w/4,  foot_h + calf_h, 0,  },
	{ -torso_w/4, foot_h + calf_h, 0,  },
	{ torso_w/4,  foot_h,          0,  },
	{ -torso_w/4, foot_h,          0,  }
};
unsigned jointLinks[JE_COUNT][2] =
{
	{ BE_HAT, BE_GROIN_R },
	{ BE_HAT, BE_GROIN_L },
	{ BE_GROIN_R, BE_CALF_R },
	{ BE_GROIN_L, BE_CALF_L },
	{ BE_CALF_R, BE_FOOT_R },
	{ BE_CALF_L, BE_FOOT_L }
};

//SimbiconState g_simbiconStates[2] =
//{
//	{     0.3,  0.0, 0.2, 0,  0.4, -1.10, 0.2, -0.05, 0.2 },
//	{ -9999.0,  2.2, 0.0, 0, -0.7, -0.05, 0.2, -0.10, 0.2 }
//};

const SimbiconState g_simbiconStates[2] =
{
	/*     dt,   cd,  cv,     tor,    swh,   swk, swa,   stk, sta*/
	{     0.3,  0.0, 0.02,    -0.01,    0.7,  -1.05, 0.3, -0.05, 0.2 },
	{     9999,  1.2, 0.0,     0,   -0.7,  -0.05, 0.2, -0.10, 0.2 }
};
SimbiconState g_simbiconStatesFB[2];

void renderBiped(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX worldMat;
	D3DXMATRIX transMat, rotMat, scaleMat;
	
	unsigned i;
	for (i = 0; i < 7; ++i)
	{
		const dReal* pos = dBodyGetPosition(g_objects[i].body);
		const dReal* rot = dBodyGetRotation(g_objects[i].body);
		D3DXMatrixTranslation(&transMat, (float)pos[0], (float)pos[1], (float)pos[2]);
		D3DXMatrixScaling(&scaleMat, 2.0f, 2.0f, 2.0f);
		OdeMatrixToDx(rotMat, rot);
		if (i >= 1 && i <=4)
		{
			D3DXMATRIX rot;
			D3DXMatrixRotationX(&rot, D3DX_PI / 2);
			rotMat = rot * rotMat;
		}
		worldMat = rotMat * transMat * scaleMat;
		pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
		g_objects[i].mesh->DrawSubset(0);
	}
}

void makeBipedMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	V(D3DXCreateBox(pd3dDevice, (float)torso_w, (float)torso_h, (float)torso_d, &g_objects[BE_HAT].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_h, 16, 2, &g_objects[BE_GROIN_R].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_h, 16, 2, &g_objects[BE_GROIN_L].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_h, 16, 2, &g_objects[BE_CALF_R].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_h, 16, 2, &g_objects[BE_CALF_L].mesh, 0));

	V(D3DXCreateBox(pd3dDevice, (float)foot_w, (float)foot_h, (float)foot_d, &g_objects[BE_FOOT_R].mesh, 0));
	V(D3DXCreateBox(pd3dDevice, (float)foot_w, (float)foot_h, (float)foot_d, &g_objects[BE_FOOT_L].mesh, 0));
}
void releaseBipedMesh()
{
	unsigned i;
	for (i = 0; i < BE_COUNT; ++i)
	{
		SAFE_RELEASE(g_objects[i].mesh);
	}
}

float g_fStateElapsedTime = 0;
void PcontrolHatBiped(float fElapsedTime)
{
	g_fStateElapsedTime += fElapsedTime;

	//dReal k =  20.0, fMax = 115;                   // 比例ゲイン，最大トルク
	dReal k =  30.0, fMax = 500;                   // 比例ゲイン，最大トルク
	unsigned i;
	for (i = 0; i < JE_COUNT; i++) {
		dReal currentAngle = dJointGetHingeAngle(g_joints[i].joint);     // 関節角の取得
		dReal targetAngle = currentAngle; // Try to stay still :)
		dReal z = targetAngle - currentAngle;                      // 残差

		dJointSetHingeParam(g_joints[i].joint, dParamVel, k*z);  // 角速度の設定
		dJointSetHingeParam(g_joints[i].joint, dParamFMax, fMax); // トルクの設定
	}


	//dReal currentAngle = dJointGetHingeAngle(g_joints[1].joint);     // 関節角の取得
	//dReal targetAngle = g_stepBipedRight*(D3DX_PI/180); // Try to stay still :)
	//dReal z = targetAngle - currentAngle;                      // 残差


	// Balance feedback
	dReal d = 0; // distance from stance ankle to COM
	dReal v = 0; // velocity of COM
	for (i = 0; i < 2; i++)
	{
		const dReal balanceFB = g_simbiconStates[0].cd * d + g_simbiconStates[0].cv * v;
		g_simbiconStatesFB[i].tor = g_simbiconStates[i].tor + balanceFB;
		g_simbiconStatesFB[i].swh = g_simbiconStates[i].swh + balanceFB;
		g_simbiconStatesFB[i].swk = g_simbiconStates[i].swk + balanceFB;
		g_simbiconStatesFB[i].swa = g_simbiconStates[i].swa + balanceFB;
		g_simbiconStatesFB[i].stk = g_simbiconStates[i].stk + balanceFB;
		g_simbiconStatesFB[i].sta = g_simbiconStates[i].sta + balanceFB;
	}

	dJointID swh = 0, swk = 0, swa = 0, sth = 0, stk = 0, sta = 0;
	if (g_swingLeg == SLE_RIGHT)
	{
		swh = g_joints[JE_R1].joint;
		swk = g_joints[JE_R2].joint;
		swa = g_joints[JE_R3].joint;
		sth = g_joints[JE_L1].joint;
		stk = g_joints[JE_L2].joint;
		sta = g_joints[JE_L3].joint;
	}
	else if (g_swingLeg == SLE_LEFT)
	{
		swh = g_joints[JE_L1].joint;
		swk = g_joints[JE_L2].joint;
		swa = g_joints[JE_L3].joint;
		sth = g_joints[JE_R1].joint;
		stk = g_joints[JE_R2].joint;
		sta = g_joints[JE_R3].joint;
	}
	else
	{
		assert(!"What the!");
	}

	assert(g_stateStep == 0 || g_stateStep == 1);

	dReal swh_d = g_simbiconStatesFB[g_stateStep].swh - dJointGetHingeAngle(swh);
	dReal swk_d = g_simbiconStatesFB[g_stateStep].swk - dJointGetHingeAngle(swk);
	dReal swa_d = g_simbiconStatesFB[g_stateStep].swa - dJointGetHingeAngle(swa);
	dReal sth_d = g_simbiconStatesFB[g_stateStep].tor - dJointGetHingeAngle(sth);
	dReal stk_d = g_simbiconStatesFB[g_stateStep].stk - dJointGetHingeAngle(stk);
	dReal sta_d = g_simbiconStatesFB[g_stateStep].sta - dJointGetHingeAngle(sta);

	const dReal* bodyRot = dBodyGetRotation(g_objects[BE_HAT].body);
	D3DXMATRIX bodyRotMat;
	D3DXQUATERNION qRot;
	D3DXVECTOR3 scale, translation;
	OdeMatrixToDx(bodyRotMat, bodyRot);
	D3DXMatrixDecompose(&scale, &qRot, &translation, &bodyRotMat);
	D3DXVECTOR3 eulerRot = ArnMath::QuatToEuler(&qRot);
	
	/*
	dJointSetHingeParam(swh, dParamVel, k*swh_d);
	dJointSetHingeParam(swh, dParamFMax, fMax);
	dJointSetHingeParam(swk, dParamVel, k*swk_d);
	dJointSetHingeParam(swk, dParamFMax, fMax);
	dJointSetHingeParam(swa, dParamVel, k*swa_d);
	dJointSetHingeParam(swa, dParamFMax, fMax);
	dJointSetHingeParam(sth, dParamVel, k*sth_d);
	dJointSetHingeParam(sth, dParamFMax, fMax);
	dJointSetHingeParam(stk, dParamVel, k*stk_d);
	dJointSetHingeParam(stk, dParamFMax, fMax);
	dJointSetHingeParam(sta, dParamVel, k*sta_d);
	dJointSetHingeParam(sta, dParamFMax, fMax);
	*/

	

	dJointAddHingeTorque(swh, k*swh_d);
	dJointSetHingeParam(swh, dParamFMax, fMax);
	dJointAddHingeTorque(swk, k*swk_d);
	dJointSetHingeParam(swk, dParamFMax, fMax);
	dJointAddHingeTorque(swa, k*swa_d);
	dJointSetHingeParam(swa, dParamFMax, fMax);
	dJointAddHingeTorque(sth, k*sth_d);
	dJointSetHingeParam(sth, dParamFMax, fMax);
	dJointAddHingeTorque(stk, k*stk_d);
	dJointSetHingeParam(stk, dParamFMax, fMax);
	dJointAddHingeTorque(sta, k*sta_d);
	dJointSetHingeParam(sta, dParamFMax, fMax);

	//assert(g_footContact[0] || g_footContact[1]);

	//if (swh_d*swh_d + swk_d*swk_d + swa_d*swa_d + stk_d*stk_d + sta_d*sta_d < 0.7)
	if (g_stateStep == 0 && g_fStateElapsedTime > g_simbiconStates[g_stateStep].dt)
	{
		if (g_stateStep == 0)
		{
			g_stateStep = 1;

			OutputDebugStringA("Desired angle achieved!\n");
		}
		
		g_fStateElapsedTime = 0;
	}
	else if (g_stateStep == 1 && g_footContact[!g_stateStep] == true)
	{
		g_footContact[!g_stateStep] = false;

		if (g_swingLeg == SLE_RIGHT)
			g_swingLeg = SLE_LEFT;
		else
			g_swingLeg = SLE_RIGHT;
		g_stateStep = 0;

		OutputDebugStringA("Desired angle achieved! (Swing leg changed)\n");
	}
	
	dJointGetHingeAngleRate()
	
	/*dJointSetHingeParam(g_joints[JE_R2].joint, dParamVel, -k*z);
	dJointSetHingeParam(g_joints[JE_R2].joint, dParamFMax, fMax);
	dJointSetHingeParam(g_joints[JE_R3].joint, dParamVel, -k*z/4);
	dJointSetHingeParam(g_joints[JE_R3].joint, dParamFMax, fMax);


	z = D3DX_PI/180*20*g_stepBiped[1];

	dJointSetHingeParam(g_joints[JE_L1].joint, dParamVel, -k*z);
	dJointSetHingeParam(g_joints[JE_L1].joint, dParamFMax, fMax);
	dJointSetHingeParam(g_joints[JE_L2].joint, dParamVel, -k*z);
	dJointSetHingeParam(g_joints[JE_L2].joint, dParamFMax, fMax);
	dJointSetHingeParam(g_joints[JE_L3].joint, dParamVel, k*z/4);
	dJointSetHingeParam(g_joints[JE_L3].joint, dParamFMax, fMax);*/

	/*dReal currentAngle = dJointGetHingeAngle(g_joints[JE_R1].joint) - dJointGetHingeAngle(g_joints[JE_L1].joint);

	z = D3DX_PI/180;
	dJointSetHingeParam(g_joints[JE_R1].joint, dParamVel, -k*currentAngle);
	dJointSetHingeParam(g_joints[JE_L1].joint, dParamVel, -k*currentAngle);*/


	g_stepBiped[0] = 0;
	g_stepBiped[1] = 0;	
}

void makeBipedBody(dWorldID world, dSpaceID space)
{
	unsigned i;
	dMass mass;

	// Crate a HAT(torso-arm-trunk)
	g_objects[BE_HAT].body = dBodyCreate(world);
	dMassSetZero(&mass);
	dMassSetBoxTotal(&mass, torso_m, torso_w, torso_h, torso_d);
	dBodySetMass(g_objects[BE_HAT].body, &mass);
	g_objects[BE_HAT].geom = dCreateBox(space, torso_w, torso_h, torso_d);
	dGeomSetBody(g_objects[BE_HAT].geom, g_objects[BE_HAT].body);
	dBodySetPosition(g_objects[BE_HAT].body, bodyCenters[BE_HAT][0], bodyCenters[BE_HAT][1], bodyCenters[BE_HAT][2]);

	// Create groins, calves (HAT and feet are excluded)
	for (i = 1; i < BE_COUNT - 2; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);
		dMassSetZero(&mass);
		dMassSetCapsuleTotal(&mass, bodyWeights[i], 2 /* Length in y-axis */, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dBodySetMass(g_objects[i].body, &mass);
		g_objects[i].geom = dCreateCapsule(space, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dGeomSetBody(g_objects[i].geom, g_objects[i].body);
	}

	// Create feet
	for (i = BE_COUNT - 2; i < BE_COUNT; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);
		dMassSetZero(&mass);
		dMassSetBoxTotal(&mass, bodyWeights[i], foot_w, foot_h, foot_d);
		dBodySetMass(g_objects[i].body, &mass);
		g_objects[i].geom = dCreateBox(space, foot_w, foot_h, foot_d);
		dGeomSetBody(g_objects[i].geom, g_objects[i].body);
	}

	// Create com(center of mass) proxy on the center of leg joints
	g_comProxy.body = dBodyCreate(world);
	dBodySetPosition(g_comProxy.body, 0, leg_h + foot_h, 0);
	dMassSetZero(&mass);
	const dReal comProxyTotalMass = 0.0000001;
	const dReal comProxyRadius = 0.0000001;
	dMassSetSphereTotal(&mass, comProxyTotalMass, comProxyRadius);
	dBodySetMass(g_comProxy.body, &mass);
	g_comProxy.geom = dCreateSphere(space, comProxyRadius);
	dGeomSetBody(g_comProxy.geom, g_comProxy.body);
	g_comProxy.joint = dJointCreateHinge(world, 0);
	dJointAttach(g_comProxy.joint, g_comProxy.body, g_objects[BE_HAT].body);
	dJointSetHingeAnchor(g_comProxy.joint, 0, leg_h + foot_h, 0);
	dJointSetHingeAxis(g_comProxy.joint, 1, 0, 0); // All hinges rotate by x-axis
	//dJointSetFixed(g_comProxy.joint);
	
	// Create joints and link
	for (i = 0; i < JE_COUNT; i++)
	{
		g_joints[i].joint = dJointCreateHinge(world, 0);
		dJointAttach(g_joints[i].joint, g_objects[ jointLinks[i][0] ].body, g_objects[ jointLinks[i][1] ].body);
		dJointSetHingeAnchor(g_joints[i].joint, jointCenters[i][0], jointCenters[i][1], jointCenters[i][2]);
		dJointSetHingeAxis(g_joints[i].joint, 1, 0, 0); // All hinges rotate by x-axis
	}

	/* Fix trunk */
	//g_trunkFixJoint = dJointCreateFixed(world, 0);  // 固定ジョイント
	//dJointAttach(g_trunkFixJoint, g_objects[BE_HAT].body, 0);
	//dJointSetFixed(g_trunkFixJoint);

	/*g_wallTrunkJoint[0] = dJointCreatePlane2D(world, 0);
	dJointAttach(g_wallTrunkJoint[0], g_objects[BE_HAT].body, 0);
	dJointSetPlane2DXParam(g_wallTrunkJoint[0], dParamFMax, 10);
	g_wallTrunkJoint[1] = dJointCreatePlane2D(world, 0);
	dJointAttach(g_wallTrunkJoint[1], g_objects[BE_HAT].body, 0);
	dJointSetPlane2DXParam(g_wallTrunkJoint[1], dParamFMax, 10);*/
}

void GetBipedInfo(BipedInfo* info)
{
	info->pos = dBodyGetPosition(g_comProxy.body);
	info->vel = dBodyGetLinearVel(g_comProxy.body);

	const dReal* stanceAnklePos = 0;
	const dReal* stanceAnkleVel = 0;
	dBodyID stanceFootBody = 0;
	if (g_swingLeg == SLE_RIGHT)
		stanceFootBody = g_objects[BE_FOOT_L].body;
	else
		stanceFootBody = g_objects[BE_FOOT_R].body;

	stanceAnklePos = dBodyGetPosition(stanceFootBody);
	stanceAnkleVel = dBodyGetLinearVel(stanceFootBody);

	dReal diff[3] = { info->pos[0] - stanceAnklePos[0], info->pos[1] - stanceAnklePos[1], info->pos[2] - stanceAnklePos[2] };
	info->d = sqrt( diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2] );
	dReal diffVel[3] = { info->vel[0] - stanceAnkleVel[0], info->vel[1] - stanceAnkleVel[1], info->vel[2] - stanceAnkleVel[2] };
	info->v = sqrt( diffVel[0]*diffVel[0] + diffVel[1]*diffVel[1] + diffVel[2]*diffVel[2] );
	
	info->ang[0] = dJointGetHingeAngle(g_joints[JE_R1].joint);
	info->ang[1] = dJointGetHingeAngle(g_joints[JE_L1].joint);
	info->ang[2] = dJointGetHingeAngle(g_joints[JE_R2].joint);
	info->ang[3] = dJointGetHingeAngle(g_joints[JE_L2].joint);
	info->ang[4] = dJointGetHingeAngle(g_joints[JE_R3].joint);
	info->ang[5] = dJointGetHingeAngle(g_joints[JE_L3].joint);

	info->angVel[0] = dJointGetHingeParam(g_joints[JE_R1].joint, dParamVel);
	info->angVel[1] = dJointGetHingeParam(g_joints[JE_L1].joint, dParamVel);
	info->angVel[2] = dJointGetHingeParam(g_joints[JE_R2].joint, dParamVel);
	info->angVel[3] = dJointGetHingeParam(g_joints[JE_L2].joint, dParamVel);
	info->angVel[4] = dJointGetHingeParam(g_joints[JE_R3].joint, dParamVel);
	info->angVel[5] = dJointGetHingeParam(g_joints[JE_L3].joint, dParamVel);

	const dReal* bodyRot = dBodyGetRotation(g_objects[BE_HAT].body);
	D3DXMATRIX bodyRotMat;
	D3DXQUATERNION qRot;
	D3DXVECTOR3 scale, translation;
	OdeMatrixToDx(bodyRotMat, bodyRot);
	D3DXMatrixDecompose(&scale, &qRot, &translation, &bodyRotMat);
	D3DXVECTOR3 eulerRot = ArnMath::QuatToEuler(&qRot);

	int a = 10;
}