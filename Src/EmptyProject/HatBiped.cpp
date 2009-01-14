#include "EmptyProjectPCH.h"
#include "UtilFunc.h"


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
enum BodyEnum { BE_HAT, BE_GROIN_R, BE_GROIN_L, BE_CALF_R, BE_CALF_L, BE_FOOT_R, BE_FOOT_L, BE_COUNT };
enum JointEnum { JE_R1, JE_L1, JE_R2, JE_L2, JE_R3, JE_L3, JE_COUNT };

MyObject g_objects[BE_COUNT];
MyObject g_joints[JE_COUNT];

// Mass distribution [kg]
const dReal torso_m = 40.0;
const dReal groin_m = 10.0;
const dReal calf_m = 8.0;
const dReal foot_m = 2;
//const dReal torso_m = 0.1;
//const dReal groin_m = 0.2;
//const dReal calf_m = 0.3;
//const dReal foot_m = 2;

// Length, radius [m]
// Torso and foot are box shaped
const dReal torso_w = 0.4;
const dReal torso_h = 0.8;
const dReal torso_d = 0.2;

const dReal foot_w = 0.15;
const dReal foot_h = 0.095; // Originally, it was 0.1. But I want provide some margin between feet and ground.
const dReal foot_d = 0.3;

// Groin and calf are capsule shaped
const dReal groin_r = 0.15/2;
const dReal groin_h = 0.5;

const dReal calf_r = 0.15/2;
const dReal calf_h = 0.5;

const dReal leg_h = groin_h + calf_h;


double bodyCenters[BE_COUNT][3] = 
{
	{ 0, 0, foot_h + leg_h + torso_h/2 }, // HAT
	{ torso_w/4, 0, foot_h + calf_h + groin_h/2 }, // GROIN_R
	{ -torso_w/4, 0, foot_h + calf_h + groin_h/2 }, // GROIN_L
	{ torso_w/4, 0, foot_h + calf_h/2 }, // CALF_R
	{ -torso_w/4, 0, foot_h + calf_h/2 }, // CALF_L
	{ torso_w/4, foot_d/2 - calf_r, foot_h/2 }, // FOOT_R
	{ -torso_w/4, foot_d/2 - calf_r, foot_h/2 } // FOOT_L
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
	{ torso_w/4, 0, foot_h + leg_h },
	{ -torso_w/4, 0, foot_h + leg_h },
	{ torso_w/4, 0, foot_h + calf_h },
	{ -torso_w/4, 0, foot_h + calf_h },
	{ torso_w/4, 0, foot_h },
	{ -torso_w/4, 0, foot_h }
};
unsigned jointLinks[JE_COUNT][2] =
{
	{ BE_HAT, BE_GROIN_R },
	{ BE_HAT, BE_GROIN_L },
	{ BE_GROIN_R, BE_CALF_R },
	{ BE_GROIN_L, BE_CALF_L },
	{ BE_CALF_R, BE_FOOT_R },
	{ BE_CALF_L, BE_FOOT_L },
};

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
		D3DXMatrixScaling(&scaleMat, 1.0f, 1.0f, -1.0f);
		OdeMatrixToDx(rotMat, rot);
		worldMat = rotMat * transMat * scaleMat;
		pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
		g_objects[i].mesh->DrawSubset(0);
	}
}

void makeBipedMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	V(D3DXCreateBox(pd3dDevice, (float)torso_w, (float)torso_d, (float)torso_h, &g_objects[BE_HAT].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_h, 16, 2, &g_objects[BE_GROIN_R].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_h, 16, 2, &g_objects[BE_GROIN_L].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_h, 16, 2, &g_objects[BE_CALF_R].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_h, 16, 2, &g_objects[BE_CALF_L].mesh, 0));

	V(D3DXCreateBox(pd3dDevice, (float)foot_w, (float)foot_d, (float)foot_h, &g_objects[BE_FOOT_R].mesh, 0));
	V(D3DXCreateBox(pd3dDevice, (float)foot_w, (float)foot_d, (float)foot_h, &g_objects[BE_FOOT_L].mesh, 0));
}
void releaseBipedMesh()
{
	unsigned i;
	for (i = 0; i < BE_COUNT; ++i)
	{
		SAFE_RELEASE(g_objects[i].mesh);
	}
}

void PcontrolHatBiped()
{
	dReal k =  10.0, fMax = 100.0;                   // 比例ゲイン，最大トルク
	unsigned i;
	for (i = 0; i < JE_COUNT; i++) {
		dReal tmp = dJointGetHingeAngle(g_joints[i].joint);     // 関節角の取得

		dReal z = 0;                      // 残差

		dJointSetHingeParam(g_joints[i].joint, dParamVel, k*z);  // 角速度の設定
		dJointSetHingeParam(g_joints[i].joint, dParamFMax, fMax); // トルクの設定
	}
}

void makeBipedBody(dWorldID world, dSpaceID space)
{
	unsigned i;
	dMass mass;

	// Crate a HAT(torso-arm-trunk)
	g_objects[BE_HAT].body = dBodyCreate(world);
	dMassSetZero(&mass);
	dMassSetBoxTotal(&mass, torso_m, torso_w, torso_d, torso_h);
	dBodySetMass(g_objects[BE_HAT].body, &mass);
	g_objects[BE_HAT].geom = dCreateBox(space, torso_w, torso_d, torso_h);
	dGeomSetBody(g_objects[BE_HAT].geom, g_objects[BE_HAT].body);
	dBodySetPosition(g_objects[BE_HAT].body, bodyCenters[BE_HAT][0], bodyCenters[BE_HAT][1], bodyCenters[BE_HAT][2]);

	// Create groins, calfs (HAT and feet are excluded)
	for (i = 1; i < BE_COUNT - 2; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);
		dMassSetZero(&mass);
		dMassSetCapsuleTotal(&mass, bodyWeights[i], 3 /* Length in z-axis */, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dBodySetMass(g_objects[i].body, &mass);
		g_objects[i].geom = dCreateCapsule(space, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dGeomSetBody(g_objects[i].geom, g_objects[i].body);
	}

	// Create feet
	dMatrix3 R;
	//dRFromAxisAndAngle(R, 1, 0, 0, M_PI/2);  // Rotate 90 deg about x-axis
	for (i = BE_COUNT - 2; i < BE_COUNT; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);
		//dBodySetRotation(g_objects[i].body, R);
		dMassSetZero(&mass);
		dMassSetBoxTotal(&mass, bodyWeights[i], foot_w, foot_d, foot_h);
		dBodySetMass(g_objects[i].body, &mass);
		g_objects[i].geom = dCreateBox(space, foot_w, foot_d, foot_h);
		dGeomSetBody(g_objects[i].geom, g_objects[i].body);
	}
	
	// Create joints and link
	for (i = 0; i < JE_COUNT; i++)
	{
		g_joints[i].joint = dJointCreateHinge(world, 0);
		dJointAttach(g_joints[i].joint, g_objects[ jointLinks[i][0] ].body, g_objects[ jointLinks[i][1] ].body);
		dJointSetHingeAnchor(g_joints[i].joint, jointCenters[i][0], jointCenters[i][1], jointCenters[i][2]);
		dJointSetHingeAxis(g_joints[i].joint, 1, 0, 0); // All hinges rotate by x-axis
	}
}
