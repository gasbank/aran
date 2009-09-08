#include "EmptyProjectPCH.h"਍⌀椀渀挀氀甀搀攀 ∀唀琀椀氀䘀甀渀挀⸀栀∀ഀഀ
਍⌀搀攀昀椀渀攀 一唀䴀 㐀                          ⼀⼀ 꼰瀰൥ഀ
਍挀漀渀猀琀 搀刀攀愀氀 砀嬀一唀䴀崀      㴀 笀　⸀　　Ⰰ 　⸀　　Ⰰ 　⸀　　Ⰰ 　⸀　　紀㬀  ⼀⼀ 촀쎑 砀ഀഀ
const dReal y[NUM]      = {0.00, 0.00, 0.00, 0.00};  // 重心 y਍挀漀渀猀琀 搀刀攀愀氀 稀嬀一唀䴀崀      㴀 笀　⸀　㔀Ⰰ 　⸀㔀　Ⰰ ㄀⸀㔀　Ⰰ ㌀紀㬀  ⼀⼀ 촀쎑 稀ഀഀ
const dReal length[NUM] = {0.10, 0.90, 1.00, 2.00};  // 長さ਍挀漀渀猀琀 搀刀攀愀氀 眀攀椀最栀琀嬀一唀䴀崀 㴀 笀㤀⸀　　Ⰰ ㈀⸀　　Ⰰ ㈀⸀　　Ⰰ ㈀⸀　　紀㬀  ⼀⼀ 쾌එഀ
const dReal r[NUM]      = {0.20, 0.1, 0.1, 0.1};  // 半径਍挀漀渀猀琀 搀刀攀愀氀 挀开砀嬀一唀䴀崀    㴀 笀　⸀　　Ⰰ 　⸀　　Ⰰ 　⸀　　Ⰰ 　⸀　　紀㬀  ⼀⼀ ꈀ삕⵻썎륟⁰砀ഀഀ
const dReal c_y[NUM]    = {0.00, 0.00, 0.00, 0.00};  // 関節中心点 y਍挀漀渀猀琀 搀刀攀愀氀 挀开稀嬀一唀䴀崀    㴀 笀　⸀　　Ⰰ 　⸀㄀　Ⰰ ㄀⸀　　Ⰰ ㈀⸀　　紀㬀  ⼀⼀ ꈀ삕⵻썎륟⁰稀ഀഀ
const dReal axis_x[NUM] = {0, 0, 0, 0};              // 関節回転軸 x਍挀漀渀猀琀 搀刀攀愀氀 愀砀椀猀开礀嬀一唀䴀崀 㴀 笀㄀Ⰰ ㄀Ⰰ ㄀Ⰰ ㄀紀㬀              ⼀⼀ ꈀ삕�₎礀ഀഀ
const dReal axis_z[NUM] = {0, 0, 0, 0};              // 関節回転軸 z਍ഀഀ
MyObject rlink[NUM];                   // リンク਍搀刀攀愀氀 吀䠀䔀吀䄀嬀一唀䴀崀 㴀 笀　⸀　紀㬀             ⼀⼀ ꈀ삕湻᥶퉪ꚉ孞爀愀搀崀ഀഀ
਍瘀漀椀搀 爀攀渀搀攀爀䄀爀洀⠀䤀䐀椀爀攀挀琀㌀䐀䐀攀瘀椀挀攀㤀⨀ 瀀搀㌀搀䐀攀瘀椀挀攀⤀ഀഀ
{਍ऀ䐀㌀䐀堀䴀䄀吀刀䤀堀 眀漀爀氀搀䴀愀琀㬀ഀഀ
	D3DXMATRIX transMat, rotMat, scaleMat;਍ऀ䐀㌀䐀堀䴀䄀吀刀䤀堀 琀爀愀渀猀娀㬀ഀഀ
	unsigned i;਍ऀ䐀㌀䐀堀䴀愀琀爀椀砀吀爀愀渀猀氀愀琀椀漀渀⠀☀琀爀愀渀猀娀Ⰰ 　Ⰰ 　Ⰰ ⴀ　⸀㔀昀⤀㬀ഀഀ
	for (i = 0; i < NUM; i++)਍ऀ笀ഀഀ
		const dReal* pos = dBodyGetPosition(rlink[i].body);਍ऀऀ挀漀渀猀琀 搀刀攀愀氀⨀ 爀漀琀 㴀 搀䈀漀搀礀䜀攀琀刀漀琀愀琀椀漀渀⠀爀氀椀渀欀嬀椀崀⸀戀漀搀礀⤀㬀ഀഀ
		D3DXMatrixTranslation(&transMat, (float)pos[0], (float)pos[1], (float)pos[2]);਍ऀऀ䐀㌀䐀堀䴀愀琀爀椀砀匀挀愀氀椀渀最⠀☀猀挀愀氀攀䴀愀琀Ⰰ ㄀⸀　昀Ⰰ ㄀⸀　昀Ⰰ ⴀ㄀⸀　昀⤀㬀ഀഀ
		OdeMatrixToDx(rotMat, rot);਍ऀऀ眀漀爀氀搀䴀愀琀 㴀 爀漀琀䴀愀琀 ⨀ 琀爀愀渀猀䴀愀琀 ⨀ 猀挀愀氀攀䴀愀琀㬀ഀഀ
		pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);਍ऀऀ爀氀椀渀欀嬀椀崀⸀洀攀猀栀ⴀ㸀䐀爀愀眀匀甀戀猀攀琀⠀　⤀㬀ഀഀ
	}਍紀ഀഀ
਍䠀刀䔀匀唀䰀吀 洀愀欀攀䄀爀洀䴀攀猀栀⠀䤀䐀椀爀攀挀琀㌀䐀䐀攀瘀椀挀攀㤀⨀ 瀀搀㌀搀䐀攀瘀椀挀攀⤀ഀഀ
{਍ऀ䠀刀䔀匀唀䰀吀 栀爀 㴀 匀开伀䬀㬀ഀഀ
	unsigned i;਍ऀ昀漀爀 ⠀椀 㴀 　㬀 椀 㰀 一唀䴀㬀 ⬀⬀椀⤀ഀഀ
		V(D3DXCreateBox(pd3dDevice, (float)r[i], (float)r[i], (float)length[i], &rlink[i].mesh, 0));਍ऀ爀攀琀甀爀渀 栀爀㬀ഀഀ
}਍瘀漀椀搀 爀攀氀攀愀猀攀䄀爀洀䴀攀猀栀⠀⤀ഀഀ
{਍ऀ甀渀猀椀最渀攀搀 椀㬀ഀഀ
	for (i = 0; i < NUM; ++i)਍ऀऀ匀䄀䘀䔀开刀䔀䰀䔀䄀匀䔀⠀爀氀椀渀欀嬀椀崀⸀洀攀猀栀⤀㬀ഀഀ
}਍ഀഀ
਍⼀⨀⨀⨀ �쌰젰ꈰﰰ渰ἰၵ⁢⨀⨀⨀⼀ഀഀ
void  makeArmBody(dWorldID world, dSpaceID space)਍笀ഀഀ
	dMass mass;                                    // 質量パラメータ਍ऀഀഀ
਍ऀ⼀⼀ 꼰渰ἰၵൢഀ
	for (int i = 0; i < NUM; i++) {਍ऀऀ爀氀椀渀欀嬀椀崀⸀戀漀搀礀 㴀 搀䈀漀搀礀䌀爀攀愀琀攀⠀眀漀爀氀搀⤀㬀ഀഀ
		dBodySetPosition(rlink[i].body, x[i], y[i], z[i]);਍ऀऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
		dMassSetCapsuleTotal(&mass,weight[i],3,r[i],length[i]);਍ऀऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀爀氀椀渀欀嬀椀崀⸀戀漀搀礀Ⰰ ☀洀愀猀猀⤀㬀ഀഀ
		rlink[i].geom = dCreateCapsule(space,r[i],length[i]);਍ऀऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀爀氀椀渀欀嬀椀崀⸀最攀漀洀Ⰰ爀氀椀渀欀嬀椀崀⸀戀漀搀礀⤀㬀ഀഀ
	}਍ഀഀ
	// ジョイントの生成とリンクへの取り付け਍ऀ爀氀椀渀欀嬀　崀⸀樀漀椀渀琀 㴀 搀䨀漀椀渀琀䌀爀攀愀琀攀䘀椀砀攀搀⠀眀漀爀氀搀Ⰰ 　⤀㬀  ⼀⼀ 切驖롛ꐰ젰രഀ
	dJointAttach(rlink[0].joint, rlink[0].body, 0);਍ऀ搀䨀漀椀渀琀匀攀琀䘀椀砀攀搀⠀爀氀椀渀欀嬀　崀⸀樀漀椀渀琀⤀㬀ഀഀ
	for (int j = 1; j < NUM; j++) {਍ऀऀ爀氀椀渀欀嬀樀崀⸀樀漀椀渀琀 㴀 搀䨀漀椀渀琀䌀爀攀愀琀攀䠀椀渀最攀⠀眀漀爀氀搀Ⰰ 　⤀㬀 ⼀⼀ 툀렰렰ꐰ젰രഀ
		dJointAttach(rlink[j].joint, rlink[j].body, rlink[j-1].body);਍ऀऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀䄀渀挀栀漀爀⠀爀氀椀渀欀嬀樀崀⸀樀漀椀渀琀Ⰰ 挀开砀嬀樀崀Ⰰ 挀开礀嬀樀崀Ⰰ 挀开稀嬀樀崀⤀㬀ഀഀ
		dJointSetHingeAxis(rlink[j].joint, axis_x[j], axis_y[j],axis_z[j]);਍ऀ紀ഀഀ
}਍⼀⨀⨀⨀ 倀㘀ꅒ ⨀⨀⨀⼀ഀഀ
void PcontrolArm()਍笀ഀഀ
	dReal k =  10.0, fMax = 100.0;                   // 比例ゲイン，最大トルク਍ऀ甀渀猀椀最渀攀搀 樀㬀ഀഀ
	for (j = 1; j < NUM; j++) {਍ऀऀ搀刀攀愀氀 琀洀瀀 㴀 搀䨀漀椀渀琀䜀攀琀䠀椀渀最攀䄀渀最氀攀⠀爀氀椀渀欀嬀樀崀⸀樀漀椀渀琀⤀㬀     ⼀⼀ ꈀ삕퉻溉혰靓ൟഀ
਍ऀऀ搀刀攀愀氀 稀 㴀 吀䠀䔀吀䄀嬀樀崀 ⴀ 琀洀瀀㬀                      ⼀⼀ 謀൝ഀ
਍ऀऀ搀䨀漀椀渀琀匀攀琀䠀椀渀最攀倀愀爀愀洀⠀爀氀椀渀欀嬀樀崀⸀樀漀椀渀琀Ⰰ搀倀愀爀愀洀嘀攀氀Ⰰ 欀⨀稀⤀㬀  ⼀⼀ 툀ᾉꚐ湞ⴰ骊൛ഀ
		dJointSetHingeParam(rlink[j].joint,dParamFMax,fMax); // トルクの設定਍ऀ紀ഀഀ
}਍ഀഀ
void KeyboardProcArm( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )਍笀ഀഀ
	switch (nChar) {਍ऀ挀愀猀攀 ✀䈀✀㨀 吀䠀䔀吀䄀嬀㄀崀 ⬀㴀 䴀开倀䤀⼀㄀㠀　㬀 戀爀攀愀欀㬀     ⼀⼀ 樀관ﰰരഀ
	case 'V': THETA[1] -= M_PI/180; break;     // fキー਍ऀ挀愀猀攀 ✀一✀㨀 吀䠀䔀吀䄀嬀㈀崀 ⬀㴀 䴀开倀䤀⼀㄀㠀　㬀 戀爀攀愀欀㬀     ⼀⼀ 欀관ﰰരഀ
	case 'C': THETA[2] -= M_PI/180; break;     // dキー਍ऀ挀愀猀攀 ✀䴀✀㨀 吀䠀䔀吀䄀嬀㌀崀 ⬀㴀 䴀开倀䤀⼀㄀㠀　㬀 戀爀攀愀欀㬀     ⼀⼀ 氀관ﰰരഀ
	case 'X': THETA[3] -= M_PI/180; break;     // sキー਍ऀ紀ഀഀ
}