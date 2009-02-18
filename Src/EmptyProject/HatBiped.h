#pragma once


struct SimbiconState
{
	double dt, cd, cv, tor, swh, swk, swa, stk, sta;
};

struct BipedInfo
{
	const dReal* pos;		// COM proxy's position
	const dReal* vel;		// COM proxy's velocity
	dReal ang[6];			// Angle of each joints
	dReal angVel[6];		// Anglular velocity of each joints
	dReal d;				// Distance between stance ankle and COM
	dReal v;				// Velocity of COM
	dReal torsoRot;			// Angle of torso
};

enum BodyEnum { BE_HAT, BE_GROIN_R, BE_GROIN_L, BE_CALF_R, BE_CALF_L, BE_FOOT_R, BE_FOOT_L, BE_COUNT };
enum JointEnum { JE_R1, JE_L1, JE_R2, JE_L2, JE_R3, JE_L3, JE_COUNT };
enum SwingLegEnum { SLE_RIGHT, SLE_LEFT };

void makeBipedBody(dWorldID world, dSpaceID space);
void renderBiped(IDirect3DDevice9* pd3dDevice);
void makeBipedMesh(IDirect3DDevice9* pd3dDevice);
void PcontrolHatBiped(float fElapsedTime);
void releaseBipedMesh();
void GetBipedInfo(BipedInfo* info);
