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

// Length, radius [m]
// Torso and foot are box shaped
const dReal torso_w = 0.4;
const dReal torso_h = 0.8;
const dReal torso_d = 0.2;

const dReal foot_w = 0.15;
const dReal foot_h = 0.1;
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
	{ torso_w/2, 0, foot_h + calf_h + groin_h/2 }, // GROIN_R
	{ -torso_w/2, 0, foot_h + calf_h + groin_h/2 }, // GROIN_L
	{ torso_w/2, 0, foot_h + calf_h/2 }, // CALF_R
	{ -torso_w/2, 0, foot_h + calf_h/2 }, // CALF_L
	{ torso_w/2, foot_d/2 - calf_r, foot_h/2 }, // FOOT_R
	{ -torso_w/2, foot_d/2 - calf_r, foot_h/2 } // FOOT_L
};
double bodyRadiusAndHeight[BE_COUNT][2] =
{
	{ -1, -1 },
	{ groin_r, groin_h },
	{ groin_r, groin_h },
	{ calf_r, calf_h },
	{ calf_r, calf_h },
	{ -1, -1 },
	{ -1, -1 }
};
double bodyWeights[BE_COUNT] = { torso_m, groin_m, groin_m, calf_m, calf_m, foot_m, foot_m };
double jointCenters[JE_COUNT][3] =
{
	{ torso_w/2, 0, foot_h + leg_h },
	{ -torso_w/2, 0, foot_h + leg_h },
	{ torso_w/2, 0, foot_h + calf_h },
	{ -torso_w/2, 0, foot_h + calf_h },
	{ torso_w/2, 0, foot_h },
	{ -torso_w/2, 0, foot_h }
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
//
//// Link position y-axis
//const dReal y[leg_num][link_num] = {
//	{ leg_len + 0.1, calf_len + 0.1, 0.1 },
//	{ leg_len + 0.1, calf_len + 0.1, 0.1 }
//};
//// Link position z-axis
//const dReal z[leg_num][link_num] = {
//	{ 0, 0, 0 },
//	{ 0, 0, 0 }
//};
//
//// Center of joints x-axis
//const dReal c_x[leg_num][link_num] = {
//	{  torso_w,  torso_w,  torso_w }, // Left leg
//	{ -torso_w, -torso_w, -torso_w } // Right leg
//};
//// Center of joints y-axis
//const dReal c_y[leg_num][link_num] = {
//	{ leg_len + 0.1, calf_len + 0.1, 0.1 },
//	{ leg_len + 0.1, calf_len + 0.1, 0.1 }
//};
//// Center of joints z-axis
//const dReal c_z[leg_num][link_num] = {
//	{ 0, 0, 0 },
//	{ 0, 0, 0 }
//};
//
//const dReal r[link_num]          =  { groin_r, calf_r, foot_r };
//const dReal length[link_num]     =  { groin_len, calf_len, foot_len };
//const dReal weight[link_num]     =  { groin_m, calf_m, foot_m };
//dReal axis_x[leg_num][link_num] = { { 0, 1, 0 }, { 0, 1, 0 } };
//dReal axis_y[leg_num][link_num] = { { 1, 0, 1 }, { 1, 0, 1 } };
//dReal axis_z[leg_num][link_num] = { { 0, 0, 0 }, { 0, 0, 0 } };



void renderBiped(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX worldMat;
	D3DXMATRIX transMat, rotMat;
	
	unsigned i;
	for (i = 0; i < BE_COUNT; ++i)
	{
		const dReal* pos = dBodyGetPosition(g_objects[i].body);
		const dReal* rot = dBodyGetRotation(g_objects[i].body);

		D3DXMatrixTranslation(&transMat, (float)pos[0], (float)pos[1], (float)pos[2]);
		OdeMatrixToDx(rotMat, rot);
		worldMat = transMat * rotMat;
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


void makeBipedBody(dWorldID world, dSpaceID space)
{
	// Crate a HAT(torso-arm-trunk)
	dMass mass;
	g_objects[BE_HAT].body = dBodyCreate(world);
	dMassSetZero(&mass);
	dMassSetBoxTotal(&mass, torso_m, torso_w, torso_d, torso_h);
	dBodySetMass(g_objects[BE_HAT].body, &mass);
	g_objects[BE_HAT].geom = dCreateBox(space, torso_w, torso_d, torso_h);
	dGeomSetBody(g_objects[BE_HAT].geom, g_objects[BE_HAT].body);
	dBodySetPosition(g_objects[BE_HAT].body, bodyCenters[BE_HAT][0], bodyCenters[BE_HAT][1], bodyCenters[BE_HAT][2]);

	// Create groins, calfs (HAT and feet are excluded)
	unsigned i;
	for (i = 1; i < BE_COUNT - 2; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][0]);
		dMassSetZero(&mass);
		dMassSetCapsuleTotal(&mass, bodyWeights[i], 3 /* Length in z-axis */, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dBodySetMass(g_objects[i].body, &mass);
		g_objects[i].geom = dCreateCapsule(space, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);
		dGeomSetBody(g_objects[i].geom, g_objects[i].body);
	}
	// Create feet
	dMatrix3 R;
	dRFromAxisAndAngle(R, 1, 0, 0, M_PI/2);  // Rotate 90 deg about x-axis
	for (i = BE_COUNT - 2; i < BE_COUNT; i++)
	{
		g_objects[i].body = dBodyCreate(world);
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][0]);
		dBodySetRotation(g_objects[i].body, R);
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
