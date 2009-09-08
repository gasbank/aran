#include "EmptyProjectPCH.h"਍⌀椀渀挀氀甀搀攀 ∀䈀愀氀氀⸀栀∀ഀഀ
਍ഀഀ
MyObject g_ball;਍ഀഀ
਍䠀刀䔀匀唀䰀吀 洀愀欀攀䈀愀氀氀䴀攀猀栀⠀䤀䐀椀爀攀挀琀㌀䐀䐀攀瘀椀挀攀㤀⨀ 瀀搀㌀搀䐀攀瘀椀挀攀⤀ഀഀ
{਍ऀ䠀刀䔀匀唀䰀吀 栀爀㬀ഀഀ
	V(D3DXCreateSphere(pd3dDevice, (float)g_ball.r, 8, 8, &g_ball.mesh, 0));਍ऀ爀攀琀甀爀渀 栀爀㬀ഀഀ
}਍ഀഀ
਍ഀഀ
void makeBallBody(dWorldID world, dSpaceID space)਍笀ഀഀ
	dReal r = 0.2, m  = 1.0;਍ऀ搀刀攀愀氀 砀　 㴀 㔀⸀　Ⰰ 礀　 㴀 㔀⸀　Ⰰ 稀　 㴀 　⸀　㬀ഀഀ
	dMass mass;਍ഀഀ
	g_ball.body = dBodyCreate(world);਍ऀ搀䴀愀猀猀匀攀琀娀攀爀漀⠀☀洀愀猀猀⤀㬀ഀഀ
	dMassSetSphereTotal(&mass,m,r);਍ऀ搀䈀漀搀礀匀攀琀䴀愀猀猀⠀最开戀愀氀氀⸀戀漀搀礀Ⰰ☀洀愀猀猀⤀㬀ഀഀ
	dBodySetPosition(g_ball.body, x0, y0, z0);਍ऀ最开戀愀氀氀⸀爀      㴀 爀㬀ഀഀ
	g_ball.geom   = dCreateSphere(space,g_ball.r); // 球ジオメトリの生成਍ऀ搀䜀攀漀洀匀攀琀䈀漀搀礀⠀最开戀愀氀氀⸀最攀漀洀Ⰰ最开戀愀氀氀⸀戀漀搀礀⤀㬀         ⼀⼀ �윰ꌰ栰렰ꨰ젰渰ꈰ⎕�兎രഀ
਍紀ഀഀ
਍瘀漀椀搀 爀攀渀搀攀爀䈀愀氀氀⠀ 䤀䐀椀爀攀挀琀㌀䐀䐀攀瘀椀挀攀㤀⨀ 瀀搀㌀搀䐀攀瘀椀挀攀 ⤀ഀഀ
{਍ऀ䐀㌀䐀堀䴀䄀吀刀䤀堀 眀漀爀氀搀䴀愀琀㬀ഀഀ
	const dReal* ballPos = dBodyGetPosition(g_ball.body);਍ऀ䐀㌀䐀堀䴀愀琀爀椀砀吀爀愀渀猀氀愀琀椀漀渀⠀☀眀漀爀氀搀䴀愀琀Ⰰ ⠀昀氀漀愀琀⤀戀愀氀氀倀漀猀嬀　崀Ⰰ ⠀昀氀漀愀琀⤀戀愀氀氀倀漀猀嬀㄀崀Ⰰ ⴀ⠀昀氀漀愀琀⤀戀愀氀氀倀漀猀嬀㈀崀⤀㬀ഀഀ
	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);਍ऀ最开戀愀氀氀⸀洀攀猀栀ⴀ㸀䐀爀愀眀匀甀戀猀攀琀⠀　⤀㬀ഀഀ
}਍ഀഀ
void releaseBallMesh()਍笀ഀഀ
	SAFE_RELEASE(g_ball.mesh);਍紀�