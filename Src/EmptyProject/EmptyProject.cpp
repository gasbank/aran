#include "EmptyProjectPCH.h"
#include "DXUTcamera.h"
#include "resource.h"
#include "Picture.h"
#include "SpriteManager.h"
#include "Sprite.h"
#include "HatBiped.h"
#include "RobotArm.h"
#include "Ball.h"

LPDIRECT3DVERTEXSHADER9         g_pVertexShader = NULL;
LPD3DXCONSTANTTABLE             g_pConstantTable = NULL;
LPDIRECT3DVERTEXDECLARATION9    g_pVertexDeclaration = NULL;

CFirstPersonCamera				g_camera;
Picture							g_pic;

dWorldID     					g_world;
dSpaceID     					g_space;
dGeomID      					g_ground;
dGeomID      					g_wall, g_wall2;
dJointGroupID					g_contactgroup;

//LPD3DXMESH g_blendedMesh = 0;
LPDIRECT3DVERTEXBUFFER9			g_blendedVB = 0;
LPDIRECT3DINDEXBUFFER9			g_blendedIB = 0;
LPDIRECT3DVERTEXDECLARATION9	g_decl = 0;

LPD3DXFONT							g_pFont							= 0;

#define D3DFVF_BLENDEDVERT (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX0)

const unsigned vertCount = 12;
const unsigned triCount = 20;
int g_step = 0;

int g_stepBiped[2] = { 0, 0 };
bool g_bStartSim = false;

SpriteManager*						g_spriteManager					= 0;
Sprite*								g_stateSprite = 0;
enum FsmState { STATE0, STATE1, STATE2, STATE3, NUM_STATE };
DrawRequest*						g_fsmDr[NUM_STATE];

FsmState g_currentState = STATE0;
extern MyObject g_objects[BE_COUNT];

bool g_footContact[2] = { true, true };

struct VERTEX
{
	VERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v, float w0, float w1, float w2, DWORD matIndices)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		weight0 = w0;
		weight1 = w1;
		weight2 = w2;
		matrixIndices = matIndices;
		normal[0] = nx;
		normal[1] = ny;
		normal[2] = nz;
		this->u = u;
		this->v = v;
	}
	float x,y,z;
	float weight0;
	float weight1;
	float weight2;
	DWORD matrixIndices; // 0x44332211 --> 11, 22, 33, 44 are individual bone matrix indices
	float normal[3];
	float u, v;
};

D3DVERTEXELEMENT9 g_blendedVertDecl[] = 
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
	{ 0, 24, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
	{ 0, 28, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};


static void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
	dBodyID b1 = dGeomGetBody(o1);
	dBodyID b2 = dGeomGetBody(o2);

	if (b1 == g_objects[BE_FOOT_R].body || b2 == g_objects[BE_FOOT_R].body)
		g_footContact[0] = true;
	else if (b1 == g_objects[BE_FOOT_L].body || b2 == g_objects[BE_FOOT_L].body)
		g_footContact[1] = true;

	if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
		return;
	// if ((o1 != ground) && (o2 != ground)) return;

	
	static const int N = 20;
	dContact contact[N];
	int i;
	int n = dCollide(o1, o2, N, &contact[0].geom, sizeof(dContact));
	if (n > 0)
	{
		for (i = 0; i < n; i++)
		{
			if (o1 == g_wall || o1 == g_wall2 || o2 == g_wall || o2 == g_wall2)
			{
				contact[i].surface.mode = 0;
				contact[i].surface.mu   = 0;
				dJointID c = dJointCreateContact(g_world, g_contactgroup, &contact[i]);
				dJointAttach(c, b1, b2);
			}
			else
			{
				//contact[i].surface.mode = dContactSoftERP | dContactSoftCFM;
				contact[i].surface.mode = 0;
				contact[i].surface.mu   = dInfinity; //2.0;
				//contact[i].surface.mu   = 5000;
				//contact[i].surface.soft_erp = 0.9;
				contact[i].surface.soft_erp = 0;
				contact[i].surface.soft_cfm = 0;
				
				dJointID c = dJointCreateContact(g_world, g_contactgroup, &contact[i]);
				dJointAttach(c, b1, b2);
			}
		}
	}
}

//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
    // Typically want to skip back buffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

	//const DWORD maxBoneCount = 30;
	//if (pCaps->MaxVertexBlendMatrixIndex < maxBoneCount)
	//	return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	
    return true;
}



#define FOUR_BYTES_INTO_DWORD(i1, i2, i3, i4)  ((DWORD)((i1)&0xff | (((i2)&0xff)<<8) | (((i3)&0xff)<<16) | (((i4)&0xff)<<24) ))

HRESULT makeBlendedMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	V(pd3dDevice->CreateVertexBuffer(sizeof(VERTEX) * vertCount, D3DUSAGE_WRITEONLY, D3DFVF_BLENDEDVERT, D3DPOOL_MANAGED, &g_blendedVB, 0));

	VERTEX* vert;

	g_blendedVB->Lock(0, 0, (void**)&vert, 0);
	
	vert[  0] = VERTEX(   1.000000f,   1.000000f,   1.000000f,   0.000000f,   0.447188f,   0.894406f,   0.000000f,   1.000000f,   0.333333f,   0.333333f,   0.333333f, FOUR_BYTES_INTO_DWORD(1, 2, 0, 255) );
	vert[  1] = VERTEX(   1.000000f,  -1.000000f,   1.000000f,   0.000000f,  -0.832026f,   0.554674f,   0.000000f,   1.000000f,   0.333333f,   0.333333f,   0.333333f, FOUR_BYTES_INTO_DWORD(1, 2, 0, 255) );
	vert[  2] = VERTEX(  -1.000000f,  -1.000000f,   1.000000f,  -0.577349f,  -0.577349f,   0.577349f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(0, 255, 255, 255) );
	vert[  3] = VERTEX(  -1.000000f,   1.000000f,   1.000000f,  -0.408246f,   0.816492f,   0.408246f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(0, 255, 255, 255) );
	vert[  4] = VERTEX(   1.000000f,   0.999999f,  -1.000000f,   0.000000f,   0.799982f,  -0.599994f,   0.000000f,   1.000000f,   0.333333f,   0.333333f,   0.333333f, FOUR_BYTES_INTO_DWORD(1, 2, 0, 255) );
	vert[  5] = VERTEX(   0.999999f,  -1.000001f,  -1.000000f,   0.000000f,  -0.707083f,  -0.707083f,   0.000000f,   1.000000f,   0.333333f,   0.333333f,   0.333333f, FOUR_BYTES_INTO_DWORD(1, 2, 0, 255) );
	vert[  6] = VERTEX(  -1.000000f,  -1.000000f,  -1.000000f,  -0.577349f,  -0.577349f,  -0.577349f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(0, 255, 255, 255) );
	vert[  7] = VERTEX(  -1.000000f,   1.000000f,  -1.000000f,  -0.666646f,   0.333323f,  -0.666646f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(0, 255, 255, 255) );
	vert[  8] = VERTEX(   2.999999f,  -1.000001f,  -1.000000f,   0.333323f,  -0.666646f,  -0.666646f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(2, 255, 255, 255) );
	vert[  9] = VERTEX(   3.000000f,   0.999999f,  -1.000000f,   0.816492f,   0.408246f,  -0.408246f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(2, 255, 255, 255) );
	vert[ 10] = VERTEX(   3.000000f,  -1.000000f,   1.000000f,   0.666646f,  -0.333323f,   0.666646f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(2, 255, 255, 255) );
	vert[ 11] = VERTEX(   3.000000f,   1.000000f,   1.000000f,   0.408246f,   0.816492f,   0.408246f,   0.000000f,   1.000000f,   1.000000f,   0.000000f,   0.000000f, FOUR_BYTES_INTO_DWORD(2, 255, 255, 255) );

	g_blendedVB->Unlock();

	V(pd3dDevice->CreateIndexBuffer(sizeof(WORD) * triCount * 3, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_blendedIB, 0));

	WORD* f;
	g_blendedIB->Lock(0, 0, (void**)&f, 0);


	f[  0] = 4;
	f[  1] = 0;
	f[  2] = 3;
	f[  3] = 4;
	f[  4] = 3;
	f[  5] = 7;
	f[  6] = 6;
	f[  7] = 7;
	f[  8] = 2;
	f[  9] = 7;
	f[ 10] = 3;
	f[ 11] = 2;
	f[ 12] = 1;
	f[ 13] = 5;
	f[ 14] = 2;
	f[ 15] = 5;
	f[ 16] = 6;
	f[ 17] = 2;
	f[ 18] = 4;
	f[ 19] = 7;
	f[ 20] = 5;
	f[ 21] = 7;
	f[ 22] = 6;
	f[ 23] = 5;
	f[ 24] = 3;
	f[ 25] = 0;
	f[ 26] = 2;
	f[ 27] = 0;
	f[ 28] = 1;
	f[ 29] = 2;
	f[ 30] = 5;
	f[ 31] = 8;
	f[ 32] = 4;
	f[ 33] = 8;
	f[ 34] = 9;
	f[ 35] = 4;
	f[ 36] = 5;
	f[ 37] = 1;
	f[ 38] = 8;
	f[ 39] = 1;
	f[ 40] = 10;
	f[ 41] = 8;
	f[ 42] = 0;
	f[ 43] = 4;
	f[ 44] = 11;
	f[ 45] = 4;
	f[ 46] = 9;
	f[ 47] = 11;
	f[ 48] = 0;
	f[ 49] = 11;
	f[ 50] = 10;
	f[ 51] = 0;
	f[ 52] = 10;
	f[ 53] = 1;
	f[ 54] = 11;
	f[ 55] = 9;
	f[ 56] = 10;
	f[ 57] = 9;
	f[ 58] = 8;
	f[ 59] = 10;



	g_blendedIB->Unlock();


	V( pd3dDevice->CreateVertexDeclaration(g_blendedVertDecl, &g_decl ) );
	
	return hr;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
	HRESULT hr;
	// Setup main camera
	D3DXVECTOR3 vecEye( 7, 3, 7 );
	D3DXVECTOR3 vecAt ( 0, 0.5, 0 );
	g_camera.SetViewParams( &vecEye, &vecAt );

	// Load sample image (vertex and index buffer creation with texture)
	g_pic.init(L"bigtile_notext.tga", pd3dDevice, 1);
	g_pic.setSize(32, 32);


	makeBallMesh(pd3dDevice);
	makeBipedMesh(pd3dDevice);
	makeArmMesh(pd3dDevice);

	makeBlendedMesh(pd3dDevice);


	GetSpriteManager().onCreateDevice( pd3dDevice );

	g_stateSprite = GetSpriteManager().registerSprite( "StateSprite", "FSM.png" );
	g_stateSprite->registerRect( "State0", 0, 0, 64, 64);
	g_stateSprite->registerRect( "State1", 64, 0, 128, 64);
	g_stateSprite->registerRect( "State2", 64, 64, 128, 128);
	g_stateSprite->registerRect( "State3", 0, 64, 64, 128);

	g_fsmDr[STATE0] = g_stateSprite->drawRequest( "State0", 0, 0, 0, 0, 0xffffffff );
	g_fsmDr[STATE0]->bRender = false;
	g_fsmDr[STATE1] = g_stateSprite->drawRequest( "State1", 0, 0, 0, 0, 0xffffffff );
	g_fsmDr[STATE1]->bRender = false;
	g_fsmDr[STATE2] = g_stateSprite->drawRequest( "State2", 0, 0, 0, 0, 0xffffffff );
	g_fsmDr[STATE2]->bRender = false;
	g_fsmDr[STATE3] = g_stateSprite->drawRequest( "State3", 0, 0, 0, 0, 0xffffffff );
	g_fsmDr[STATE3]->bRender = false;

	V_RETURN( D3DXCreateFont( pd3dDevice, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_RASTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Gulimche"), &g_pFont) );
    return S_OK;
}
D3DLIGHT9 g_light;
D3DMATERIAL9 g_defMaterial;
D3DXMATRIX g_orthoProjMat, g_fixedViewMat;

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	ZeroMemory(&g_light, sizeof(D3DLIGHT9));
	g_light.Ambient = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
	g_light.Diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	g_light.Range = 50.0f;
	g_light.Position = D3DXVECTOR3(0, 5.0f, 10.0f);
	g_light.Type = D3DLIGHT_POINT;
	g_light.Direction = D3DXVECTOR3(0, -1.0f, 0);
	g_light.Attenuation0 = 0.001f;
	g_light.Attenuation1 = 0;
	g_light.Attenuation2 = 0;
	ZeroMemory(&g_defMaterial, sizeof(D3DMATERIAL9));
	g_defMaterial.Ambient = D3DXCOLOR(0, 0, 1, 1.0f);
	g_defMaterial.Diffuse = D3DXCOLOR(0.85f, 0.1f, 0.1f, 1.0f);

	pd3dDevice->SetMaterial(&g_defMaterial);
	pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	pd3dDevice->SetLight(0, &g_light);
	pd3dDevice->LightEnable(0, TRUE);

	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 2000.0f );
	
	D3DXVECTOR3 eye(0, 0, -50.0f), at(0, 0, 0), up(0, 1.0f, 0);
	D3DXMatrixOrthoLH(&g_orthoProjMat, (FLOAT)pBackBufferSurfaceDesc->Width, (FLOAT)pBackBufferSurfaceDesc->Height, 0.1f, 100.0f);
	D3DXMatrixLookAtLH(&g_fixedViewMat, &eye, &at, &up);


	GetSpriteManager().onResetDevice( pd3dDevice, pBackBufferSurfaceDesc, pUserContext );
	g_pFont->OnResetDevice();
    return S_OK;
}


void frameMoveOde(float fElapsedTime)
{
	//PcontrolArm();
	

	dSpaceCollide(g_space, 0, &nearCallback);  // 衝突検出関数

	PcontrolHatBiped(fElapsedTime);
	//dWorldStep(g_world, fElapsedTime);
	dWorldStep(g_world, 0.001);
	dJointGroupEmpty(g_contactgroup); // ジョイントグループを空にする

	g_footContact[0] = g_footContact[1] = false;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_pic.frameMove(fElapsedTime);
	g_camera.FrameMove(fElapsedTime);

	if (g_bStartSim)
		frameMoveOde(fElapsedTime);

	g_fsmDr[STATE0]->bRender = false;
	g_fsmDr[STATE1]->bRender = false;
	g_fsmDr[STATE2]->bRender = false;
	g_fsmDr[STATE3]->bRender = false;
	g_fsmDr[g_currentState]->bRender = true;
}

HRESULT renderIndexedPrimitives(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS );
	pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	D3DXMATRIX matIden;
	D3DXMatrixIdentity( &matIden );
	unsigned i;
	// D3DTS_WORLDMATRIX(0) is the same as D3DTS_WORLD!
	for (i = 0; i < 4; ++i)
	{
		pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(i), &matIden );
	}
	D3DXMATRIX matScale, matRotX;
	D3DXVECTOR3 Xaxis(1, 0, 0);
	D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
	D3DXMatrixRotationAxis(&matRotX, &Xaxis, D3DX_PI / 180 * g_step);
	D3DXMATRIX matFinal = matRotX;

	//pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(1), &matFinal );
	
	//pd3dDevice->SetTransform( D3DTS_WORLD, &matIden );

	pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(2), &matFinal );


	V( pd3dDevice->SetVertexDeclaration(g_decl) );
	V( pd3dDevice->SetStreamSource(0, g_blendedVB, 0, sizeof(VERTEX)) );
	V( pd3dDevice->SetIndices(g_blendedIB) );
	V( pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertCount, 0, triCount) );

	pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
	pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
	return hr;
}
void frameRenderState(IDirect3DDevice9* pd3dDevice)
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pd3dDevice->SetTransform(D3DTS_VIEW, &g_fixedViewMat);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_orthoProjMat);
}

void renderDebugMessage()
{
	RECT rc;
	rc.top = 0;
	rc.left = 0;
	rc.right = 800;
	rc.bottom = 600;
	const int lineHeight = 14;

	D3DXCOLOR textColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	const dReal* comPos = 0;
	const dReal* comVel = 0;
	dReal jointAngVel[6];
	dReal jointAng[6];
	dReal d, v;
	BipedInfo bi;
	GetBipedInfo(&bi);
	char debugmsg[128];

	sprintf(debugmsg, "COM_Pos     : %8.4f,%8.4f,%8.4f", bi.pos[0], bi.pos[1], bi.pos[2]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "COM_Vel     : %8.4f,%8.4f,%8.4f", bi.vel[0], bi.vel[1], bi.vel[2]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "JointAng     (Right): %8.4f,%8.4f,%8.4f", bi.ang[0], bi.ang[2], bi.ang[3]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "JointAng_Vel (Right): %8.4f,%8.4f,%8.4f", bi.angVel[0], bi.angVel[2], bi.angVel[3]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "JointAng     (Left) : %8.4f,%8.4f,%8.4f", bi.ang[0], bi.ang[2], bi.ang[3]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "JointAng_Vel (Left) : %8.4f,%8.4f,%8.4f", bi.angVel[1], bi.angVel[3], bi.angVel[5]);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "d and v             :          %8.4f,%8.4f", bi.d, bi.v);
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
	rc.top += lineHeight; rc.bottom += lineHeight;
	sprintf(debugmsg, "Foot Contact        :  Right-%s    Left-%s", g_footContact[0]?"yes":" no", g_footContact[1]?"yes":"no");
	g_pFont->DrawTextA( 0, debugmsg, -1, &rc, DT_NOCLIP | DT_RIGHT, textColor );
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );


    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		// Setup view and projection xforms
		pd3dDevice->SetTransform(D3DTS_VIEW, g_camera.GetViewMatrix());
		pd3dDevice->SetTransform(D3DTS_PROJECTION, g_camera.GetProjMatrix());

		// Draw our image
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		g_pic.setPos(-16, 0, -16);
		g_pic.draw();
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

		pd3dDevice->SetTexture(0, 0);
		
		//renderBall(pd3dDevice);
		//renderArm(pd3dDevice);
		renderBiped(pd3dDevice);
		
		//renderIndexedPrimitives(pd3dDevice);


		frameRenderState(pd3dDevice);

		//GetSpriteManager().frameRenderSpecificSprite( "StateSprite" );
		GetSpriteManager().frameRender();

		renderDebugMessage();

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{

	g_pic.handleMessages(hWnd, uMsg, wParam, lParam);
	g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
    return 0;
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
	if ( SpriteManager::getSingletonPtr() )
		GetSpriteManager().onLostDevice();
#define SAFE_ONLOSTDEVICE(x) if (x) (x)->OnLostDevice();
	SAFE_ONLOSTDEVICE( g_pFont );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
	g_pic.release();

	releaseBallMesh();
	releaseArmMesh();
	releaseBipedMesh();
	
	SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE( g_pConstantTable );
	SAFE_RELEASE( g_pVertexDeclaration );

	SAFE_RELEASE( g_blendedVB );
	SAFE_RELEASE( g_blendedIB );
	SAFE_RELEASE( g_decl );

	GetSpriteManager().onDestroyDevice();
	SAFE_RELEASE( g_pFont );
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	KeyboardProcArm(nChar, bKeyDown, bAltDown, pUserContext);

	if (nChar == 'O')
		g_step++;
	else if (nChar == 'P')
		g_step--;
	else if (nChar == 'T')
		g_stepBiped[0]++;
	else if (nChar == 'Y')
		g_stepBiped[0]--;
	else if (nChar == 'U')
		g_stepBiped[1]++;
	else if (nChar == 'I')
		g_stepBiped[1]--;
	else if (nChar == '1')
		g_bStartSim = true;

	if (nChar == 107 && bKeyDown) // NUMPAD's + key
	{
		g_currentState = (FsmState)((int)g_currentState + 1);
		if (g_currentState > STATE3)
			g_currentState = STATE0;
	}
}

void initializeOde()
{
	dInitODE();
	g_world        = dWorldCreate();
	g_space        = dHashSpaceCreate(0);
	
	g_contactgroup = dJointGroupCreate(0);
	g_ground       = dCreatePlane(g_space, 0, 1, 0, 0);
	g_wall         = dCreatePlane(g_space, 1, 0, 0, -0.25);
	g_wall2         = dCreatePlane(g_space, -1, 0, 0, -0.25);
	dWorldSetGravity(g_world, 0, -9.81, 0);
	//dWorldSetGravity(g_world, 0, 0, 0);
	dWorldSetCFM(g_world, 1e-3);
	dWorldSetERP(g_world, 0.9);

	//makeBallBody(g_world, g_space);
	//makeArmBody(g_world, g_space);
	makeBipedBody(g_world, g_space);
}
void destroyOde()
{
	dJointGroupDestroy(g_contactgroup);
	dSpaceDestroy(g_space);
	dWorldDestroy(g_world);
	dCloseODE();
}
//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( KeyboardProc );

    // TODO: Perform any application-level initialization here
	initializeOde();

	g_spriteManager = new SpriteManager();

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"EmptyProject", 0, 0, 0, 0, 0, true );
    DXUTCreateDevice( true, 800, 600 );

	

    // Start the render loop
    DXUTMainLoop();

	DXUTShutdown();

    // TODO: Perform any application-level cleanup here
	destroyOde();

	delete g_spriteManager;

    return 0;
}

