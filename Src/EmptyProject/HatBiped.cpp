#include "EmptyProjectPCH.h"਍⌀椀渀挀氀甀搀攀 ∀䠀愀琀䈀椀瀀攀搀⸀栀∀ഀഀ
#include "UtilFunc.h"਍⌀椀渀挀氀甀搀攀 ∀䄀爀渀䴀愀琀栀⸀栀∀ഀഀ
਍⼀⨀ഀഀ
਍⼀ⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀⴀ尀ഀഀ
|           |਍簀           簀  䠀攀愀搀⼀䄀爀洀猀⼀吀漀爀猀漀 嬀䠀䄀吀崀ഀഀ
|           |਍簀           簀ഀഀ
\-----------/਍  ⠀⨀⤀   ⠀⨀⤀ഀഀ
 |---| |---|਍ 簀   簀 簀   簀ഀഀ
 |   | |   |   Groin਍ 簀   簀 簀   簀ഀഀ
 |---| |---|਍  ⠀⨀⤀   ⠀⨀⤀ഀഀ
 |---| |---|਍ 簀   簀 簀   簀   䌀愀氀昀ഀഀ
 |   | |   |਍ 簀ⴀⴀⴀ簀 簀ⴀⴀⴀ簀ഀഀ
  (*)   (*)਍  ⼀ⴀ尀   ⼀ⴀ尀    䘀漀漀琀ഀഀ
  \-/   \-/਍ഀഀ
*/਍ഀഀ
SwingLegEnum g_swingLeg = SLE_RIGHT;਍甀渀猀椀最渀攀搀 椀渀琀 最开猀琀愀琀攀匀琀攀瀀 㴀 　㬀ഀഀ
extern int g_stepBiped[2];਍ഀഀ
MyObject g_objects[BE_COUNT];਍䴀礀伀戀樀攀挀琀 最开樀漀椀渀琀猀嬀䨀䔀开䌀伀唀一吀崀㬀ഀഀ
MyObject g_comProxy;਍ഀഀ
dJointID			g_trunkFixJoint;਍⼀⼀ 䴀愀猀猀 搀椀猀琀爀椀戀甀琀椀漀渀 嬀欀最崀ഀഀ
const dReal torso_m = 70.0;਍挀漀渀猀琀 搀刀攀愀氀 最爀漀椀渀开洀 㴀 㔀⸀　㬀ഀഀ
const dReal calf_m = 4.0;਍挀漀渀猀琀 搀刀攀愀氀 昀漀漀琀开洀 㴀 ㄀⸀　㬀ഀഀ
//const dReal torso_m = 0.1;਍⼀⼀挀漀渀猀琀 搀刀攀愀氀 最爀漀椀渀开洀 㴀 　⸀㈀㬀ഀഀ
//const dReal calf_m = 0.3;਍⼀⼀挀漀渀猀琀 搀刀攀愀氀 昀漀漀琀开洀 㴀 ㈀㬀ഀഀ
਍⼀⼀ 䰀攀渀最琀栀Ⰰ 爀愀搀椀甀猀 嬀洀崀ഀഀ
// Torso and foot are box shaped਍挀漀渀猀琀 搀刀攀愀氀 琀漀爀猀漀开眀 㴀 　⸀㐀㠀㬀ഀഀ
const dReal torso_h = 0.8;਍挀漀渀猀琀 搀刀攀愀氀 琀漀爀猀漀开搀 㴀 　⸀㈀㬀ഀഀ
਍挀漀渀猀琀 搀刀攀愀氀 昀漀漀琀开眀 㴀 　⸀㄀㔀㬀ഀഀ
const dReal foot_h = 0.095; // Originally, it was 0.1. But I want provide some margin between feet and ground.਍挀漀渀猀琀 搀刀攀愀氀 昀漀漀琀开搀 㴀 　⸀㌀㬀ഀഀ
਍⼀⼀ 䜀爀漀椀渀 愀渀搀 挀愀氀昀 愀爀攀 挀愀瀀猀甀氀攀 猀栀愀瀀攀搀ഀഀ
const dReal groin_r = 0.15/2;਍挀漀渀猀琀 搀刀攀愀氀 最爀漀椀渀开栀 㴀 　⸀㐀㔀㬀ഀഀ
਍挀漀渀猀琀 搀刀攀愀氀 挀愀氀昀开爀 㴀 　⸀㄀㔀⼀㈀㬀ഀഀ
const dReal calf_h = 0.45;਍ഀഀ
const dReal leg_h = groin_h + calf_h;਍ഀഀ
extern dGeomID      					g_wall, g_wall2;਍搀䨀漀椀渀琀䤀䐀ऀऀऀऀऀऀ最开眀愀氀氀吀爀甀渀欀䨀漀椀渀琀嬀㈀崀㬀ഀഀ
extern bool g_footContact[2];਍搀漀甀戀氀攀 戀漀搀礀䌀攀渀琀攀爀猀嬀䈀䔀开䌀伀唀一吀崀嬀㌀崀 㴀 ഀഀ
{਍ऀ笀 　Ⰰ           昀漀漀琀开栀 ⬀ 氀攀最开栀 ⬀ 琀漀爀猀漀开栀⼀㈀Ⰰ  　                  紀Ⰰ ⼀⼀ 䠀䄀吀ഀഀ
	{ torso_w/4,   foot_h + calf_h + groin_h/2, 0                  }, // GROIN_R਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ  昀漀漀琀开栀 ⬀ 挀愀氀昀开栀 ⬀ 最爀漀椀渀开栀⼀㈀Ⰰ 　                  紀Ⰰ ⼀⼀ 䜀刀伀䤀一开䰀ഀഀ
	{ torso_w/4,   foot_h + calf_h/2,           0                  }, // CALF_R਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ  昀漀漀琀开栀 ⬀ 挀愀氀昀开栀⼀㈀Ⰰ           　                  紀Ⰰ ⼀⼀ 䌀䄀䰀䘀开䰀ഀഀ
	{ torso_w/4,   foot_h/2,                    foot_d/2 - calf_r  }, // FOOT_R਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ  昀漀漀琀开栀⼀㈀Ⰰ                    昀漀漀琀开搀⼀㈀ ⴀ 挀愀氀昀开爀  紀 ⼀⼀ 䘀伀伀吀开䰀ഀഀ
};਍搀漀甀戀氀攀 戀漀搀礀刀愀搀椀甀猀䄀渀搀䠀攀椀最栀琀嬀䈀䔀开䌀伀唀一吀崀嬀㈀崀 㴀ഀഀ
{਍ऀ笀 ⴀ㄀Ⰰ ⴀ㄀ 紀Ⰰഀഀ
	{ groin_r, groin_h - groin_r*2 },਍ऀ笀 最爀漀椀渀开爀Ⰰ 最爀漀椀渀开栀 ⴀ 最爀漀椀渀开爀⨀㈀ 紀Ⰰഀഀ
	{ calf_r, calf_h - calf_r*2 },਍ऀ笀 挀愀氀昀开爀Ⰰ 挀愀氀昀开栀 ⴀ 挀愀氀昀开爀⨀㈀ 紀Ⰰഀഀ
	{ -1, -1 },਍ऀ笀 ⴀ㄀Ⰰ ⴀ㄀ 紀ഀഀ
};਍搀漀甀戀氀攀 戀漀搀礀圀攀椀最栀琀猀嬀䈀䔀开䌀伀唀一吀崀 㴀 笀 琀漀爀猀漀开洀Ⰰ 最爀漀椀渀开洀Ⰰ 最爀漀椀渀开洀Ⰰ 挀愀氀昀开洀Ⰰ 挀愀氀昀开洀Ⰰ 昀漀漀琀开洀Ⰰ 昀漀漀琀开洀 紀㬀ഀഀ
double jointCenters[JE_COUNT][3] =਍笀ഀഀ
	{ torso_w/4,  foot_h + leg_h,  0   },਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ 昀漀漀琀开栀 ⬀ 氀攀最开栀Ⰰ  　   紀Ⰰഀഀ
	{ torso_w/4,  foot_h + calf_h, 0,  },਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ 昀漀漀琀开栀 ⬀ 挀愀氀昀开栀Ⰰ 　Ⰰ  紀Ⰰഀഀ
	{ torso_w/4,  foot_h,          0,  },਍ऀ笀 ⴀ琀漀爀猀漀开眀⼀㐀Ⰰ 昀漀漀琀开栀Ⰰ          　Ⰰ  紀ഀഀ
};਍甀渀猀椀最渀攀搀 樀漀椀渀琀䰀椀渀欀猀嬀䨀䔀开䌀伀唀一吀崀嬀㈀崀 㴀ഀഀ
{਍ऀ笀 䈀䔀开䠀䄀吀Ⰰ 䈀䔀开䜀刀伀䤀一开刀 紀Ⰰഀഀ
	{ BE_HAT, BE_GROIN_L },਍ऀ笀 䈀䔀开䜀刀伀䤀一开刀Ⰰ 䈀䔀开䌀䄀䰀䘀开刀 紀Ⰰഀഀ
	{ BE_GROIN_L, BE_CALF_L },਍ऀ笀 䈀䔀开䌀䄀䰀䘀开刀Ⰰ 䈀䔀开䘀伀伀吀开刀 紀Ⰰഀഀ
	{ BE_CALF_L, BE_FOOT_L }਍紀㬀ഀഀ
਍⼀⼀匀椀洀戀椀挀漀渀匀琀愀琀攀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀㈀崀 㴀ഀഀ
//{਍⼀⼀ऀ笀     　⸀㌀Ⰰ  　⸀　Ⰰ 　⸀㈀Ⰰ 　Ⰰ  　⸀㐀Ⰰ ⴀ㄀⸀㄀　Ⰰ 　⸀㈀Ⰰ ⴀ　⸀　㔀Ⰰ 　⸀㈀ 紀Ⰰഀഀ
//	{ -9999.0,  2.2, 0.0, 0, -0.7, -0.05, 0.2, -0.10, 0.2 }਍⼀⼀紀㬀ഀഀ
਍挀漀渀猀琀 匀椀洀戀椀挀漀渀匀琀愀琀攀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀㈀崀 㴀ഀഀ
{਍ऀ⼀⨀     搀琀Ⰰ   挀搀Ⰰ  挀瘀Ⰰ     琀漀爀Ⰰ    猀眀栀Ⰰ   猀眀欀Ⰰ 猀眀愀Ⰰ   猀琀欀Ⰰ 猀琀愀⨀⼀ഀഀ
	{     0.3,  0.0, 0.02,    -0.01,    0.7,  -1.05, 0.3, -0.05, 0.2 },਍ऀ笀     㤀㤀㤀㤀Ⰰ  ㄀⸀㈀Ⰰ 　⸀　Ⰰ     　Ⰰ   ⴀ　⸀㜀Ⰰ  ⴀ　⸀　㔀Ⰰ 　⸀㈀Ⰰ ⴀ　⸀㄀　Ⰰ 　⸀㈀ 紀ഀഀ
};਍匀椀洀戀椀挀漀渀匀琀愀琀攀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀㈀崀㬀ഀഀ
਍瘀漀椀搀 爀攀渀搀攀爀䈀椀瀀攀搀⠀䤀䐀椀爀攀挀琀㌀䐀䐀攀瘀椀挀攀㤀⨀ 瀀搀㌀搀䐀攀瘀椀挀攀⤀ഀഀ
{਍ऀ䐀㌀䐀堀䴀䄀吀刀䤀堀 眀漀爀氀搀䴀愀琀㬀ഀഀ
	D3DXMATRIX transMat, rotMat, scaleMat;਍ऀഀഀ
	unsigned i;਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 㜀㬀 ⬀⬀椀⤀ഀഀ
	{਍ऀऀ挀漀渀猀琀 搀刀攀愀氀⨀ 瀀漀猀 㴀 搀䈀漀搀礀䜀攀琀倀漀猀椀琀椀漀渀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀⤀㬀ഀഀ
		const dReal* rot = dBodyGetRotation(g_objects[i].body);਍ऀऀ䐀㌀䐀堀䴀愀琀爀椀砀吀爀愀渀猀氀愀琀椀漀渀⠀☀琀爀愀渀猀䴀愀琀Ⰰ ⠀昀氀漀愀琀⤀瀀漀猀嬀　崀Ⰰ ⠀昀氀漀愀琀⤀瀀漀猀嬀㄀崀Ⰰ ⠀昀氀漀愀琀⤀瀀漀猀嬀㈀崀⤀㬀ഀഀ
		D3DXMatrixScaling(&scaleMat, 2.0f, 2.0f, 2.0f);਍ऀऀ伀搀攀䴀愀琀爀椀砀吀漀䐀砀⠀爀漀琀䴀愀琀Ⰰ 爀漀琀⤀㬀ഀഀ
		if (i >= 1 && i <=4)਍ऀऀ笀ഀഀ
			D3DXMATRIX rot;਍ऀऀऀ䐀㌀䐀堀䴀愀琀爀椀砀刀漀琀愀琀椀漀渀堀⠀☀爀漀琀Ⰰ 䐀㌀䐀堀开倀䤀 ⼀ ㈀⤀㬀ഀഀ
			rotMat = rot * rotMat;਍ऀऀ紀ഀഀ
		worldMat = rotMat * transMat * scaleMat;਍ऀऀ瀀搀㌀搀䐀攀瘀椀挀攀ⴀ㸀匀攀琀吀爀愀渀猀昀漀爀洀⠀䐀㌀䐀吀匀开圀伀刀䰀䐀Ⰰ ☀眀漀爀氀搀䴀愀琀⤀㬀ഀഀ
		g_objects[i].mesh->DrawSubset(0);਍ऀ紀ഀഀ
}਍ഀഀ
void makeBipedMesh(IDirect3DDevice9* pd3dDevice)਍笀ഀഀ
	HRESULT hr;਍ऀ嘀⠀䐀㌀䐀堀䌀爀攀愀琀攀䈀漀砀⠀瀀搀㌀搀䐀攀瘀椀挀攀Ⰰ ⠀昀氀漀愀琀⤀琀漀爀猀漀开眀Ⰰ ⠀昀氀漀愀琀⤀琀漀爀猀漀开栀Ⰰ ⠀昀氀漀愀琀⤀琀漀爀猀漀开搀Ⰰ ☀最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀洀攀猀栀Ⰰ 　⤀⤀㬀ഀഀ
਍ऀ嘀⠀䐀㌀䐀堀䌀爀攀愀琀攀䌀礀氀椀渀搀攀爀⠀瀀搀㌀搀䐀攀瘀椀挀攀Ⰰ ⠀昀氀漀愀琀⤀最爀漀椀渀开爀Ⰰ ⠀昀氀漀愀琀⤀最爀漀椀渀开爀Ⰰ ⠀昀氀漀愀琀⤀最爀漀椀渀开栀Ⰰ ㄀㘀Ⰰ ㈀Ⰰ ☀最开漀戀樀攀挀琀猀嬀䈀䔀开䜀刀伀䤀一开刀崀⸀洀攀猀栀Ⰰ 　⤀⤀㬀ഀഀ
	V(D3DXCreateCylinder(pd3dDevice, (float)groin_r, (float)groin_r, (float)groin_h, 16, 2, &g_objects[BE_GROIN_L].mesh, 0));਍ഀഀ
	V(D3DXCreateCylinder(pd3dDevice, (float)calf_r, (float)calf_r, (float)calf_h, 16, 2, &g_objects[BE_CALF_R].mesh, 0));਍ऀ嘀⠀䐀㌀䐀堀䌀爀攀愀琀攀䌀礀氀椀渀搀攀爀⠀瀀搀㌀搀䐀攀瘀椀挀攀Ⰰ ⠀昀氀漀愀琀⤀挀愀氀昀开爀Ⰰ ⠀昀氀漀愀琀⤀挀愀氀昀开爀Ⰰ ⠀昀氀漀愀琀⤀挀愀氀昀开栀Ⰰ ㄀㘀Ⰰ ㈀Ⰰ ☀最开漀戀樀攀挀琀猀嬀䈀䔀开䌀䄀䰀䘀开䰀崀⸀洀攀猀栀Ⰰ 　⤀⤀㬀ഀഀ
਍ऀ嘀⠀䐀㌀䐀堀䌀爀攀愀琀攀䈀漀砀⠀瀀搀㌀搀䐀攀瘀椀挀攀Ⰰ ⠀昀氀漀愀琀⤀昀漀漀琀开眀Ⰰ ⠀昀氀漀愀琀⤀昀漀漀琀开栀Ⰰ ⠀昀氀漀愀琀⤀昀漀漀琀开搀Ⰰ ☀最开漀戀樀攀挀琀猀嬀䈀䔀开䘀伀伀吀开刀崀⸀洀攀猀栀Ⰰ 　⤀⤀㬀ഀഀ
	V(D3DXCreateBox(pd3dDevice, (float)foot_w, (float)foot_h, (float)foot_d, &g_objects[BE_FOOT_L].mesh, 0));਍紀ഀഀ
void releaseBipedMesh()਍笀ഀഀ
	unsigned i;਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 䈀䔀开䌀伀唀一吀㬀 ⬀⬀椀⤀ഀഀ
	{਍ऀऀ匀䄀䘀䔀开刀䔀䰀䔀䄀匀䔀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀洀攀猀栀⤀㬀ഀഀ
	}਍紀ഀഀ
਍昀氀漀愀琀 最开昀匀琀愀琀攀䔀氀愀瀀猀攀搀吀椀洀攀 㴀 　㬀ഀഀ
void PcontrolHatBiped(float fElapsedTime)਍笀ഀഀ
	g_fStateElapsedTime += fElapsedTime;਍ഀഀ
	//dReal k =  20.0, fMax = 115;                   // 比例ゲイン，最大トルク਍ऀ搀刀攀愀氀 欀 㴀  ㌀　⸀　Ⰰ 昀䴀愀砀 㴀 㔀　　㬀                   ⼀⼀ 퐀譫뉏ꐰరÿ❧졙꼰രഀ
	unsigned i;਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 䨀䔀开䌀伀唀一吀㬀 椀⬀⬀⤀ 笀ഀഀ
		dReal currentAngle = dJointGetHingeAngle(g_joints[i].joint);     // 関節角の取得਍ऀऀ搀刀攀愀氀 琀愀爀最攀琀䄀渀最氀攀 㴀 挀甀爀爀攀渀琀䄀渀最氀攀㬀 ⼀⼀ 吀爀礀 琀漀 猀琀愀礀 猀琀椀氀氀 㨀⤀ഀഀ
		dReal z = targetAngle - currentAngle;                      // 残差਍ഀഀ
		dJointSetHingeParam(g_joints[i].joint, dParamVel, k*z);  // 角速度の設定਍ऀऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀椀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀 ⼀⼀ 저꼰渰ⴰ骊൛ഀ
	}਍ഀഀ
਍ऀ⼀⼀搀刀攀愀氀 挀甀爀爀攀渀琀䄀渀最氀攀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀㄀崀⸀樀漀椀渀琀⤀㬀     ⼀⼀ ꈀ삕퉻溉혰靓ൟഀ
	//dReal targetAngle = g_stepBipedRight*(D3DX_PI/180); // Try to stay still :)਍ऀ⼀⼀搀刀攀愀氀 稀 㴀 琀愀爀最攀琀䄀渀最氀攀 ⴀ 挀甀爀爀攀渀琀䄀渀最氀攀㬀                      ⼀⼀ 謀൝ഀ
਍ഀഀ
	// Balance feedback਍ऀ搀刀攀愀氀 搀 㴀 　㬀 ⼀⼀ 搀椀猀琀愀渀挀攀 昀爀漀洀 猀琀愀渀挀攀 愀渀欀氀攀 琀漀 䌀伀䴀ഀഀ
	dReal v = 0; // velocity of COM਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 ㈀㬀 椀⬀⬀⤀ഀഀ
	{਍ऀऀ挀漀渀猀琀 搀刀攀愀氀 戀愀氀愀渀挀攀䘀䈀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀　崀⸀挀搀 ⨀ 搀 ⬀ 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀　崀⸀挀瘀 ⨀ 瘀㬀ഀഀ
		g_simbiconStatesFB[i].tor = g_simbiconStates[i].tor + balanceFB;਍ऀऀ最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀椀崀⸀猀眀栀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀椀崀⸀猀眀栀 ⬀ 戀愀氀愀渀挀攀䘀䈀㬀ഀഀ
		g_simbiconStatesFB[i].swk = g_simbiconStates[i].swk + balanceFB;਍ऀऀ最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀椀崀⸀猀眀愀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀椀崀⸀猀眀愀 ⬀ 戀愀氀愀渀挀攀䘀䈀㬀ഀഀ
		g_simbiconStatesFB[i].stk = g_simbiconStates[i].stk + balanceFB;਍ऀऀ最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀椀崀⸀猀琀愀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀嬀椀崀⸀猀琀愀 ⬀ 戀愀氀愀渀挀攀䘀䈀㬀ഀഀ
	}਍ഀഀ
	dJointID swh = 0, swk = 0, swa = 0, sth = 0, stk = 0, sta = 0;਍ऀ椀昀 ⠀最开猀眀椀渀最䰀攀最 㴀㴀 匀䰀䔀开刀䤀䜀䠀吀⤀ഀഀ
	{਍ऀऀ猀眀栀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开刀㄀崀⸀樀漀椀渀琀㬀ഀഀ
		swk = g_joints[JE_R2].joint;਍ऀऀ猀眀愀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开刀㌀崀⸀樀漀椀渀琀㬀ഀഀ
		sth = g_joints[JE_L1].joint;਍ऀऀ猀琀欀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开䰀㈀崀⸀樀漀椀渀琀㬀ഀഀ
		sta = g_joints[JE_L3].joint;਍ऀ紀ഀഀ
	else if (g_swingLeg == SLE_LEFT)਍ऀ笀ഀഀ
		swh = g_joints[JE_L1].joint;਍ऀऀ猀眀欀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开䰀㈀崀⸀樀漀椀渀琀㬀ഀഀ
		swa = g_joints[JE_L3].joint;਍ऀऀ猀琀栀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开刀㄀崀⸀樀漀椀渀琀㬀ഀഀ
		stk = g_joints[JE_R2].joint;਍ऀऀ猀琀愀 㴀 最开樀漀椀渀琀猀嬀䨀䔀开刀㌀崀⸀樀漀椀渀琀㬀ഀഀ
	}਍ऀ攀氀猀攀ഀഀ
	{਍ऀऀ愀猀猀攀爀琀⠀℀∀圀栀愀琀 琀栀攀℀∀⤀㬀ഀഀ
	}਍ഀഀ
	assert(g_stateStep == 0 || g_stateStep == 1);਍ഀഀ
	dReal swh_d = g_simbiconStatesFB[g_stateStep].swh - dJointGetHingeAngle(swh);਍ऀ搀刀攀愀氀 猀眀欀开搀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀最开猀琀愀琀攀匀琀攀瀀崀⸀猀眀欀 ⴀ 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀猀眀欀⤀㬀ഀഀ
	dReal swa_d = g_simbiconStatesFB[g_stateStep].swa - dJointGetHingeAngle(swa);਍ऀ搀刀攀愀氀 猀琀栀开搀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀最开猀琀愀琀攀匀琀攀瀀崀⸀琀漀爀 ⴀ 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀猀琀栀⤀㬀ഀഀ
	dReal stk_d = g_simbiconStatesFB[g_stateStep].stk - dJointGetHingeAngle(stk);਍ऀ搀刀攀愀氀 猀琀愀开搀 㴀 最开猀椀洀戀椀挀漀渀匀琀愀琀攀猀䘀䈀嬀最开猀琀愀琀攀匀琀攀瀀崀⸀猀琀愀 ⴀ 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀猀琀愀⤀㬀ഀഀ
਍ऀ挀漀渀猀琀 搀刀攀愀氀⨀ 戀漀搀礀刀漀琀 㴀 搀䈀漀搀礀䜀攀琀刀漀琀愀琀椀漀渀⠀最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀⤀㬀ഀഀ
	D3DXMATRIX bodyRotMat;਍ऀ䐀㌀䐀堀儀唀䄀吀䔀刀一䤀伀一 焀刀漀琀㬀ഀഀ
	D3DXVECTOR3 scale, translation;਍ऀ伀搀攀䴀愀琀爀椀砀吀漀䐀砀⠀戀漀搀礀刀漀琀䴀愀琀Ⰰ 戀漀搀礀刀漀琀⤀㬀ഀഀ
	D3DXMatrixDecompose(&scale, &qRot, &translation, &bodyRotMat);਍ऀ䐀㌀䐀堀嘀䔀䌀吀伀刀㌀ 攀甀氀攀爀刀漀琀 㴀 䄀爀渀䴀愀琀栀㨀㨀儀甀愀琀吀漀䔀甀氀攀爀⠀☀焀刀漀琀⤀㬀ഀഀ
	਍ऀ⼀⨀ഀഀ
	dJointSetHingeParam(swh, dParamVel, k*swh_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀栀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(swk, dParamVel, k*swk_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀欀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(swa, dParamVel, k*swa_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀愀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(sth, dParamVel, k*sth_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀栀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(stk, dParamVel, k*stk_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀欀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(sta, dParamVel, k*sta_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀愀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	*/਍ഀഀ
	਍ഀഀ
	dJointAddHingeTorque(swh, k*swh_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀栀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointAddHingeTorque(swk, k*swk_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀欀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointAddHingeTorque(swa, k*swa_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀眀愀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointAddHingeTorque(sth, k*sth_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀栀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointAddHingeTorque(stk, k*stk_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀欀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointAddHingeTorque(sta, k*sta_d);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀猀琀愀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
਍ऀ⼀⼀愀猀猀攀爀琀⠀最开昀漀漀琀䌀漀渀琀愀挀琀嬀　崀 簀簀 最开昀漀漀琀䌀漀渀琀愀挀琀嬀㄀崀⤀㬀ഀഀ
਍ऀ⼀⼀椀昀 ⠀猀眀栀开搀⨀猀眀栀开搀 ⬀ 猀眀欀开搀⨀猀眀欀开搀 ⬀ 猀眀愀开搀⨀猀眀愀开搀 ⬀ 猀琀欀开搀⨀猀琀欀开搀 ⬀ 猀琀愀开搀⨀猀琀愀开搀 㰀 　⸀㜀⤀ഀഀ
	if (g_stateStep == 0 && g_fStateElapsedTime > g_simbiconStates[g_stateStep].dt)਍ऀ笀ഀഀ
		if (g_stateStep == 0)਍ऀऀ笀ഀഀ
			g_stateStep = 1;਍ഀഀ
			OutputDebugStringA("Desired angle achieved!\n");਍ऀऀ紀ഀഀ
		਍ऀऀ最开昀匀琀愀琀攀䔀氀愀瀀猀攀搀吀椀洀攀 㴀 　㬀ഀഀ
	}਍ऀ攀氀猀攀 椀昀 ⠀最开猀琀愀琀攀匀琀攀瀀 㴀㴀 ㄀ ☀☀ 最开昀漀漀琀䌀漀渀琀愀挀琀嬀℀最开猀琀愀琀攀匀琀攀瀀崀 㴀㴀 琀爀甀攀⤀ഀഀ
	{਍ऀऀ最开昀漀漀琀䌀漀渀琀愀挀琀嬀℀最开猀琀愀琀攀匀琀攀瀀崀 㴀 昀愀氀猀攀㬀ഀഀ
਍ऀऀ椀昀 ⠀最开猀眀椀渀最䰀攀最 㴀㴀 匀䰀䔀开刀䤀䜀䠀吀⤀ഀഀ
			g_swingLeg = SLE_LEFT;਍ऀऀ攀氀猀攀ഀഀ
			g_swingLeg = SLE_RIGHT;਍ऀऀ最开猀琀愀琀攀匀琀攀瀀 㴀 　㬀ഀഀ
਍ऀऀ伀甀琀瀀甀琀䐀攀戀甀最匀琀爀椀渀最䄀⠀∀䐀攀猀椀爀攀搀 愀渀最氀攀 愀挀栀椀攀瘀攀搀℀ ⠀匀眀椀渀最 氀攀最 挀栀愀渀最攀搀⤀尀渀∀⤀㬀ഀഀ
	}਍ऀഀഀ
	//dJointGetHingeAngleRate()਍ऀഀഀ
	/*dJointSetHingeParam(g_joints[JE_R2].joint, dParamVel, -k*z);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㈀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(g_joints[JE_R3].joint, dParamVel, -k*z/4);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㌀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
਍ഀഀ
	z = D3DX_PI/180*20*g_stepBiped[1];਍ഀഀ
	dJointSetHingeParam(g_joints[JE_L1].joint, dParamVel, -k*z);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㄀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(g_joints[JE_L2].joint, dParamVel, -k*z);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㈀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀ഀഀ
	dJointSetHingeParam(g_joints[JE_L3].joint, dParamVel, k*z/4);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㌀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ 昀䴀愀砀⤀㬀⨀⼀ഀഀ
਍ऀ⼀⨀搀刀攀愀氀 挀甀爀爀攀渀琀䄀渀最氀攀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㄀崀⸀樀漀椀渀琀⤀ ⴀ 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㄀崀⸀樀漀椀渀琀⤀㬀ഀഀ
਍ऀ稀 㴀 䐀㌀䐀堀开倀䤀⼀㄀㠀　㬀ഀഀ
	dJointSetHingeParam(g_joints[JE_R1].joint, dParamVel, -k*currentAngle);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㄀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀嘀攀氀Ⰰ ⴀ欀⨀挀甀爀爀攀渀琀䄀渀最氀攀⤀㬀⨀⼀ഀഀ
਍ഀഀ
	g_stepBiped[0] = 0;਍ऀ最开猀琀攀瀀䈀椀瀀攀搀嬀㄀崀 㴀 　㬀ऀഀഀ
}਍ഀഀ
void makeBipedBody(dWorldID world, dSpaceID space)਍笀ഀഀ
	unsigned i;਍ऀ搀䴀愀猀猀 洀愀猀猀㬀ഀഀ
਍ऀ⼀⼀ 䌀爀愀琀攀 愀 䠀䄀吀⠀琀漀爀猀漀ⴀ愀爀洀ⴀ琀爀甀渀欀⤀ഀഀ
	g_objects[BE_HAT].body = dBodyCreate(world);਍ऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
	dMassSetBoxTotal(&mass, torso_m, torso_w, torso_h, torso_d);਍ऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀Ⰰ ☀洀愀猀猀⤀㬀ഀഀ
	g_objects[BE_HAT].geom = dCreateBox(space, torso_w, torso_h, torso_d);਍ऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀最攀漀洀Ⰰ 最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀⤀㬀ഀഀ
	dBodySetPosition(g_objects[BE_HAT].body, bodyCenters[BE_HAT][0], bodyCenters[BE_HAT][1], bodyCenters[BE_HAT][2]);਍ഀഀ
	// Create groins, calves (HAT and feet are excluded)਍ऀ昀漀爀 ⠀椀 㴀 ㄀㬀 椀 㰀 䈀䔀开䌀伀唀一吀 ⴀ ㈀㬀 椀⬀⬀⤀ഀഀ
	{਍ऀऀ最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀 㴀 搀䈀漀搀礀䌀爀攀愀琀攀⠀眀漀爀氀搀⤀㬀ഀഀ
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);਍ऀऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
		dMassSetCapsuleTotal(&mass, bodyWeights[i], 2 /* Length in y-axis */, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);਍ऀऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀Ⰰ ☀洀愀猀猀⤀㬀ഀഀ
		g_objects[i].geom = dCreateCapsule(space, bodyRadiusAndHeight[i][0], bodyRadiusAndHeight[i][1]);਍ऀऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀最攀漀洀Ⰰ 最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀⤀㬀ഀഀ
	}਍ഀഀ
	// Create feet਍ऀ昀漀爀 ⠀椀 㴀 䈀䔀开䌀伀唀一吀 ⴀ ㈀㬀 椀 㰀 䈀䔀开䌀伀唀一吀㬀 椀⬀⬀⤀ഀഀ
	{਍ऀऀ最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀 㴀 搀䈀漀搀礀䌀爀攀愀琀攀⠀眀漀爀氀搀⤀㬀ഀഀ
		dBodySetPosition(g_objects[i].body, bodyCenters[i][0], bodyCenters[i][1], bodyCenters[i][2]);਍ऀऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
		dMassSetBoxTotal(&mass, bodyWeights[i], foot_w, foot_h, foot_d);਍ऀऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀Ⰰ ☀洀愀猀猀⤀㬀ഀഀ
		g_objects[i].geom = dCreateBox(space, foot_w, foot_h, foot_d);਍ऀऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀最开漀戀樀攀挀琀猀嬀椀崀⸀最攀漀洀Ⰰ 最开漀戀樀攀挀琀猀嬀椀崀⸀戀漀搀礀⤀㬀ഀഀ
	}਍ഀഀ
	// Create com(center of mass) proxy on the center of leg joints਍ऀ最开挀漀洀倀爀漀砀礀⸀戀漀搀礀 㴀 搀䈀漀搀礀䌀爀攀愀琀攀⠀眀漀爀氀搀⤀㬀ഀഀ
	dBodySetPosition(g_comProxy.body, 0, leg_h + foot_h, 0);਍ऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
	const dReal comProxyTotalMass = 0.0000001;਍ऀ挀漀渀猀琀 搀刀攀愀氀 挀漀洀倀爀漀砀礀刀愀搀椀甀猀 㴀 　⸀　　　　　　㄀㬀ഀഀ
	dMassSetSphereTotal(&mass, comProxyTotalMass, comProxyRadius);਍ऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀最开挀漀洀倀爀漀砀礀⸀戀漀搀礀Ⰰ ☀洀愀猀猀⤀㬀ഀഀ
	g_comProxy.geom = dCreateSphere(space, comProxyRadius);਍ऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀最开挀漀洀倀爀漀砀礀⸀最攀漀洀Ⰰ 最开挀漀洀倀爀漀砀礀⸀戀漀搀礀⤀㬀ഀഀ
	g_comProxy.joint = dJointCreateHinge(world, 0);਍ऀ搀䨀漀椀渀琀䄀琀琀愀挀栀⠀最开挀漀洀倀爀漀砀礀⸀樀漀椀渀琀Ⰰ 最开挀漀洀倀爀漀砀礀⸀戀漀搀礀Ⰰ 最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀⤀㬀ഀഀ
	dJointSetHingeAnchor(g_comProxy.joint, 0, leg_h + foot_h, 0);਍ऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀䄀砀椀猀⠀最开挀漀洀倀爀漀砀礀⸀樀漀椀渀琀Ⰰ ㄀Ⰰ 　Ⰰ 　⤀㬀 ⼀⼀ 䄀氀氀 栀椀渀最攀猀 爀漀琀愀琀攀 戀礀 砀ⴀ愀砀椀猀ഀഀ
	//dJointSetFixed(g_comProxy.joint);਍ऀഀഀ
	// Create joints and link਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 䨀䔀开䌀伀唀一吀㬀 椀⬀⬀⤀ഀഀ
	{਍ऀऀ最开樀漀椀渀琀猀嬀椀崀⸀樀漀椀渀琀 㴀 搀䨀漀椀渀琀䌀爀攀愀琀攀䠀椀渀最攀⠀眀漀爀氀搀Ⰰ 　⤀㬀ഀഀ
		dJointAttach(g_joints[i].joint, g_objects[ jointLinks[i][0] ].body, g_objects[ jointLinks[i][1] ].body);਍ऀऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀䄀渀挀栀漀爀⠀最开樀漀椀渀琀猀嬀椀崀⸀樀漀椀渀琀Ⰰ 樀漀椀渀琀䌀攀渀琀攀爀猀嬀椀崀嬀　崀Ⰰ 樀漀椀渀琀䌀攀渀琀攀爀猀嬀椀崀嬀㄀崀Ⰰ 樀漀椀渀琀䌀攀渀琀攀爀猀嬀椀崀嬀㈀崀⤀㬀ഀഀ
		dJointSetHingeAxis(g_joints[i].joint, 1, 0, 0); // All hinges rotate by x-axis਍ऀ紀ഀഀ
਍ऀ⼀⨀ 䘀椀砀 琀爀甀渀欀 ⨀⼀ഀഀ
	//g_trunkFixJoint = dJointCreateFixed(world, 0);  // 固定ジョイント਍ऀ⼀⼀搀䨀漀椀渀琀䄀琀琀愀挀栀⠀最开琀爀甀渀欀䘀椀砀䨀漀椀渀琀Ⰰ 最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀Ⰰ 　⤀㬀ഀഀ
	//dJointSetFixed(g_trunkFixJoint);਍ഀഀ
	/*g_wallTrunkJoint[0] = dJointCreatePlane2D(world, 0);਍ऀ搀䨀漀椀渀琀䄀琀琀愀挀栀⠀最开眀愀氀氀吀爀甀渀欀䨀漀椀渀琀嬀　崀Ⰰ 最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀Ⰰ 　⤀㬀ഀഀ
	dJointSetPlane2DXParam(g_wallTrunkJoint[0], dParamFMax, 10);਍ऀ最开眀愀氀氀吀爀甀渀欀䨀漀椀渀琀嬀㄀崀 㴀 搀䨀漀椀渀琀䌀爀攀愀琀攀倀氀愀渀攀㈀䐀⠀眀漀爀氀搀Ⰰ 　⤀㬀ഀഀ
	dJointAttach(g_wallTrunkJoint[1], g_objects[BE_HAT].body, 0);਍ऀ搀䨀漀椀渀琀匀攀琀倀氀愀渀攀㈀䐀堀倀愀爀愀洀⠀最开眀愀氀氀吀爀甀渀欀䨀漀椀渀琀嬀㄀崀Ⰰ 搀倀愀爀愀洀䘀䴀愀砀Ⰰ ㄀　⤀㬀⨀⼀ഀഀ
}਍ഀഀ
void GetBipedInfo(BipedInfo* info)਍笀ഀഀ
	info->pos = dBodyGetPosition(g_comProxy.body);਍ऀ椀渀昀漀ⴀ㸀瘀攀氀 㴀 搀䈀漀搀礀䜀攀琀䰀椀渀攀愀爀嘀攀氀⠀最开挀漀洀倀爀漀砀礀⸀戀漀搀礀⤀㬀ഀഀ
਍ऀ挀漀渀猀琀 搀刀攀愀氀⨀ 猀琀愀渀挀攀䄀渀欀氀攀倀漀猀 㴀 　㬀ഀഀ
	const dReal* stanceAnkleVel = 0;਍ऀ搀䈀漀搀礀䤀䐀 猀琀愀渀挀攀䘀漀漀琀䈀漀搀礀 㴀 　㬀ഀഀ
	if (g_swingLeg == SLE_RIGHT)਍ऀऀ猀琀愀渀挀攀䘀漀漀琀䈀漀搀礀 㴀 最开漀戀樀攀挀琀猀嬀䈀䔀开䘀伀伀吀开䰀崀⸀戀漀搀礀㬀ഀഀ
	else਍ऀऀ猀琀愀渀挀攀䘀漀漀琀䈀漀搀礀 㴀 最开漀戀樀攀挀琀猀嬀䈀䔀开䘀伀伀吀开刀崀⸀戀漀搀礀㬀ഀഀ
਍ऀ猀琀愀渀挀攀䄀渀欀氀攀倀漀猀 㴀 搀䈀漀搀礀䜀攀琀倀漀猀椀琀椀漀渀⠀猀琀愀渀挀攀䘀漀漀琀䈀漀搀礀⤀㬀ഀഀ
	stanceAnkleVel = dBodyGetLinearVel(stanceFootBody);਍ഀഀ
	dReal diff[3] = { info->pos[0] - stanceAnklePos[0], info->pos[1] - stanceAnklePos[1], info->pos[2] - stanceAnklePos[2] };਍ऀ椀渀昀漀ⴀ㸀搀 㴀 猀焀爀琀⠀ 搀椀昀昀嬀　崀⨀搀椀昀昀嬀　崀 ⬀ 搀椀昀昀嬀㄀崀⨀搀椀昀昀嬀㄀崀 ⬀ 搀椀昀昀嬀㈀崀⨀搀椀昀昀嬀㈀崀 ⤀㬀ഀഀ
	dReal diffVel[3] = { info->vel[0] - stanceAnkleVel[0], info->vel[1] - stanceAnkleVel[1], info->vel[2] - stanceAnkleVel[2] };਍ऀ椀渀昀漀ⴀ㸀瘀 㴀 猀焀爀琀⠀ 搀椀昀昀嘀攀氀嬀　崀⨀搀椀昀昀嘀攀氀嬀　崀 ⬀ 搀椀昀昀嘀攀氀嬀㄀崀⨀搀椀昀昀嘀攀氀嬀㄀崀 ⬀ 搀椀昀昀嘀攀氀嬀㈀崀⨀搀椀昀昀嘀攀氀嬀㈀崀 ⤀㬀ഀഀ
	਍ऀ椀渀昀漀ⴀ㸀愀渀最嬀　崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㄀崀⸀樀漀椀渀琀⤀㬀ഀഀ
	info->ang[1] = dJointGetHingeAngle(g_joints[JE_L1].joint);਍ऀ椀渀昀漀ⴀ㸀愀渀最嬀㈀崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㈀崀⸀樀漀椀渀琀⤀㬀ഀഀ
	info->ang[3] = dJointGetHingeAngle(g_joints[JE_L2].joint);਍ऀ椀渀昀漀ⴀ㸀愀渀最嬀㐀崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀最开樀漀椀渀琀猀嬀䨀䔀开刀㌀崀⸀樀漀椀渀琀⤀㬀ഀഀ
	info->ang[5] = dJointGetHingeAngle(g_joints[JE_L3].joint);਍ഀഀ
	info->angVel[0] = dJointGetHingeParam(g_joints[JE_R1].joint, dParamVel);਍ऀ椀渀昀漀ⴀ㸀愀渀最嘀攀氀嬀㄀崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㄀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀嘀攀氀⤀㬀ഀഀ
	info->angVel[2] = dJointGetHingeParam(g_joints[JE_R2].joint, dParamVel);਍ऀ椀渀昀漀ⴀ㸀愀渀最嘀攀氀嬀㌀崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㈀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀嘀攀氀⤀㬀ഀഀ
	info->angVel[4] = dJointGetHingeParam(g_joints[JE_R3].joint, dParamVel);਍ऀ椀渀昀漀ⴀ㸀愀渀最嘀攀氀嬀㔀崀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀倀愀爀愀洀⠀最开樀漀椀渀琀猀嬀䨀䔀开䰀㌀崀⸀樀漀椀渀琀Ⰰ 搀倀愀爀愀洀嘀攀氀⤀㬀ഀഀ
਍ऀ挀漀渀猀琀 搀刀攀愀氀⨀ 戀漀搀礀刀漀琀 㴀 搀䈀漀搀礀䜀攀琀刀漀琀愀琀椀漀渀⠀最开漀戀樀攀挀琀猀嬀䈀䔀开䠀䄀吀崀⸀戀漀搀礀⤀㬀ഀഀ
	D3DXMATRIX bodyRotMat;਍ऀ䐀㌀䐀堀儀唀䄀吀䔀刀一䤀伀一 焀刀漀琀㬀ഀഀ
	D3DXVECTOR3 scale, translation;਍ऀ伀搀攀䴀愀琀爀椀砀吀漀䐀砀⠀戀漀搀礀刀漀琀䴀愀琀Ⰰ 戀漀搀礀刀漀琀⤀㬀ഀഀ
	D3DXMatrixDecompose(&scale, &qRot, &translation, &bodyRotMat);਍ऀ䐀㌀䐀堀嘀䔀䌀吀伀刀㌀ 攀甀氀攀爀刀漀琀 㴀 䄀爀渀䴀愀琀栀㨀㨀儀甀愀琀吀漀䔀甀氀攀爀⠀☀焀刀漀琀⤀㬀ഀഀ
਍ऀ椀渀琀 愀 㴀 ㄀　㬀ഀഀ
}