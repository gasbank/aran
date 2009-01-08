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
const unsigned leg_num = 2;
const unsigned link_num = 3;

MyObject g_leg[leg_num][link_num];
MyObject g_torso;


// Mass distribution [kg]
const dReal torso_m = 40.0;
const dReal groin_m = 10.0;
const dReal calf_m = 8.0;
const dReal foot_m = 2;

// Length [m]
const dReal torso_h = 0.5;
const dReal torso_w = 0.5;
const dReal groin_len = 0.4;
const dReal calf_len = 0.4;
const dReal leg_len = groin_len + calf_len;
const dReal foot_len = 0.275;
const dReal sagittal_depth = 0.25;

// Radius [m]
const dReal groin_r = 0.2;
const dReal calf_r = 0.17;
const dReal foot_r = 0.14;

// Link position x-axis
const dReal x[leg_num][link_num] = {
	{  torso_w,  torso_w,  torso_w }, // Left leg
	{ -torso_w, -torso_w, -torso_w } // Right leg
};
// Link position y-axis
const dReal y[leg_num][link_num] = {
	{ leg_len + 0.1, calf_len + 0.1, 0.1 },
	{ leg_len + 0.1, calf_len + 0.1, 0.1 }
};
// Link position z-axis
const dReal z[leg_num][link_num] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

// Center of joints x-axis
const dReal c_x[leg_num][link_num] = {
	{  torso_w,  torso_w,  torso_w }, // Left leg
	{ -torso_w, -torso_w, -torso_w } // Right leg
};
// Center of joints y-axis
const dReal c_y[leg_num][link_num] = {
	{ leg_len + 0.1, calf_len + 0.1, 0.1 },
	{ leg_len + 0.1, calf_len + 0.1, 0.1 }
};
// Center of joints z-axis
const dReal c_z[leg_num][link_num] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

const dReal r[link_num]          =  { groin_r, calf_r, foot_r };
const dReal length[link_num]     =  { groin_len, calf_len, foot_len };
const dReal weight[link_num]     =  { groin_m, calf_m, foot_m };
dReal axis_x[leg_num][link_num] = { { 0, 1, 0 }, { 0, 1, 0 } };
dReal axis_y[leg_num][link_num] = { { 1, 0, 1 }, { 1, 0, 1 } };
dReal axis_z[leg_num][link_num] = { { 0, 0, 0 }, { 0, 0, 0 } };



void renderBiped(IDirect3DDevice9* pd3dDevice)
{
	D3DXMATRIX worldMat;
	D3DXMATRIX transMat, rotMat;
	
	// Render a torso
	const dReal* torsoPos = dBodyGetPosition(g_torso.body);
	const dReal* torsoRot = dBodyGetRotation(g_torso.body);
	D3DXMatrixTranslation(&transMat, (float)torsoPos[0], (float)torsoPos[1], (float)torsoPos[2]);
	OdeMatrixToDx(rotMat, torsoRot);
	worldMat = transMat * rotMat;
	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
	g_torso.mesh->DrawSubset(0);
	
	unsigned i, j;
	dReal r,length;
	for (i = 0; i < leg_num; i++)
	{
		for (j = 0; j < link_num; j++ )
		{
			dGeomCapsuleGetParams(g_leg[i][j].geom, &r, &length);

			const dReal* pos = dBodyGetPosition(g_leg[i][j].body);
			const dReal* rot = dBodyGetRotation(g_leg[i][j].body);
			D3DXMatrixTranslation(&transMat, (float)pos[0], (float)pos[1], (float)pos[2]);
			OdeMatrixToDx(rotMat, rot);
			worldMat = rotMat * transMat;
			pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
			g_leg[i][j].mesh->DrawSubset(0);
		}
	}
}

void makeBipedMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	V(D3DXCreateBox(pd3dDevice, (float)torso_w, (float)torso_h, (float)sagittal_depth, &g_torso.mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_len, 16, 2, &g_leg[0][0].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_len, 16, 2, &g_leg[1][0].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_len, 16, 2, &g_leg[0][1].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_len, 16, 2, &g_leg[1][1].mesh, 0));

	V(D3DXCreateCylinder(pd3dDevice, (float)foot_r, (float)foot_r, (float)foot_len, 16, 2, &g_leg[0][2].mesh, 0));
	V(D3DXCreateCylinder(pd3dDevice, (float)foot_r, (float)foot_r, (float)foot_len, 16, 2, &g_leg[1][2].mesh, 0));
}
void releaseBipedMesh()
{
	SAFE_RELEASE(g_torso.mesh);

	unsigned i, j;
	for (i = 0; i < leg_num; ++i)
		for (j = 0; j < link_num; ++j)
			SAFE_RELEASE(g_leg[i][j].mesh);
}


void makeBipedBody(dWorldID world, dSpaceID space)
{
	// Crate a torso
	dMass mass;
	g_torso.body  = dBodyCreate(world);
	dMassSetZero(&mass);
	dMassSetBoxTotal(&mass, torso_m, torso_w, torso_h, sagittal_depth);
	dBodySetMass(g_torso.body, &mass);
	g_torso.geom = dCreateBox(space,torso_w, torso_h, sagittal_depth);
	dGeomSetBody(g_torso.geom, g_torso.body);
	dBodySetPosition(g_torso.body, 0, 0, leg_len + 0.1 + torso_h/2);

	// Create legs
	dMatrix3 R;
	dRFromAxisAndAngle(R, 1, 0, 0, M_PI/2);  // Rotate 90 deg about x-axis
	unsigned i, j;
	for (i = 0; i < leg_num; i++)
	{
		for (j = 0; j < link_num; j++)
		{
			g_leg[i][j].body = dBodyCreate(world);
			if (j == link_num - 1) dBodySetRotation(g_leg[i][j].body, R); // Feet are rotated
			dBodySetPosition(g_leg[i][j].body, x[i][j], y[i][j], z[i][j]);
			dMassSetZero(&mass);
			dMassSetCapsuleTotal(&mass, weight[j], 3, r[j], length[j]);
			dBodySetMass(g_leg[i][j].body, &mass);
			g_leg[i][j].geom = dCreateCapsule(space, r[j], length[j]);
			dGeomSetBody(g_leg[i][j].geom, g_leg[i][j].body);
		}
	}

	// Create links and attach legs to the torso
	for (i = 0; i < leg_num; i++)
	{
		for (j = 0; j < link_num; j++)
		{
			g_leg[i][j].joint = dJointCreateHinge(world, 0);
			if (j == 0)
				dJointAttach(g_leg[i][j].joint, g_torso.body, g_leg[i][j].body);
			else
				dJointAttach(g_leg[i][j].joint, g_leg[i][j-1].body, g_leg[i][j].body);

			dJointSetHingeAnchor(g_leg[i][j].joint, c_x[i][j], c_y[i][j], c_z[i][j]);
			dJointSetHingeAxis(g_leg[i][j].joint, axis_x[i][j], axis_y[i][j],axis_z[i][j]);
		}
	}
}
