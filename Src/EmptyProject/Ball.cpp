#include "EmptyProjectPCH.h"
#include "Ball.h"


MyObject g_ball;


HRESULT makeBallMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	V(D3DXCreateSphere(pd3dDevice, (float)g_ball.r, 8, 8, &g_ball.mesh, 0));
	return hr;
}



void makeBallBody(dWorldID world, dSpaceID space)
{
	dReal r = 0.2, m  = 1.0;
	dReal x0 = 5.0, y0 = 5.0, z0 = 0.0;
	dMass mass;

	g_ball.body = dBodyCreate(world);
	dMassSetZero(&mass);
	dMassSetSphereTotal(&mass,m,r);
	dBodySetMass(g_ball.body,&mass);
	dBodySetPosition(g_ball.body, x0, y0, z0);
	g_ball.r      = r;
	g_ball.geom   = dCreateSphere(space,g_ball.r); // 球ジオメトリの生成
	dGeomSetBody(g_ball.geom,g_ball.body);         // ボディとジオメトリの関連付け

}

void renderBall( IDirect3DDevice9* pd3dDevice )
{
	D3DXMATRIX worldMat;
	const dReal* ballPos = dBodyGetPosition(g_ball.body);
	D3DXMatrixTranslation(&worldMat, (float)ballPos[0], (float)ballPos[1], -(float)ballPos[2]);
	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);
	g_ball.mesh->DrawSubset(0);
}

void releaseBallMesh()
{
	SAFE_RELEASE(g_ball.mesh);
}