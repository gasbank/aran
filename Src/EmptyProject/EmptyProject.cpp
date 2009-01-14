//--------------------------------------------------------------------------------------
// File: EmptyProject.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Modified PoolG Team, a Division of PoolC
//
//--------------------------------------------------------------------------------------
#include "EmptyProjectPCH.h"
#include "DXUTcamera.h"
#include "resource.h"
#include "Picture.h"
#include "HatBiped.h"
#include "RobotArm.h"

LPDIRECT3DVERTEXSHADER9         g_pVertexShader = NULL;
LPD3DXCONSTANTTABLE             g_pConstantTable = NULL;
LPDIRECT3DVERTEXDECLARATION9    g_pVertexDeclaration = NULL;

CFirstPersonCamera				g_camera;
Picture							g_pic;


dWorldID      g_world;
dSpaceID      g_space;
dGeomID       g_ground;
dJointGroupID g_contactgroup;

MyObject g_ball;

//LPD3DXMESH g_blendedMesh = 0;
LPDIRECT3DVERTEXBUFFER9 g_blendedVB = 0;
LPDIRECT3DINDEXBUFFER9 g_blendedIB = 0;
LPDIRECT3DVERTEXDECLARATION9 g_decl = 0;

#define D3DFVF_BLENDEDVERT (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX0)

const unsigned vertCount = 12;
const unsigned triCount = 20;
int g_step = 0;

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


// コールバック関数

static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
	dBodyID b1 = dGeomGetBody(o1), b2 = dGeomGetBody(o2);
	if (b1 && b2 && dAreConnectedExcluding(b1,b2,dJointTypeContact)) return;
	// if ((o1 != ground) && (o2 != ground)) return;

	static const int N = 20;
	dContact contact[N];
	int n = dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));
	if (n > 0) {
		for (int i=0; i<n; i++) {
			contact[i].surface.mode = dContactSoftERP | dContactSoftCFM;
			contact[i].surface.mu   = dInfinity; //2.0;
			contact[i].surface.soft_erp = 0.9;
			contact[i].surface.soft_cfm = 1e-5;
			dJointID c = dJointCreateContact(g_world,g_contactgroup,&contact[i]);
			dJointAttach(c,b1,b2);
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
	
    return true;
}



void makeBallBody()
{
	dReal r = 0.2, m  = 1.0;
	dReal x0 = 2.0, y0 = 0.0, z0 = 2.0;
	dMass mass;

	g_ball.body = dBodyCreate(g_world);
	dMassSetZero(&mass);
	dMassSetSphereTotal(&mass,m,r);
	dBodySetMass(g_ball.body,&mass);
	dBodySetPosition(g_ball.body, x0, y0, z0);
	g_ball.r      = r;
	g_ball.geom   = dCreateSphere(g_space,g_ball.r); // 球ジオメトリの生成
	dGeomSetBody(g_ball.geom,g_ball.body);         // ボディとジオメトリの関連付け
	
}
HRESULT makeBallMesh(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	V(D3DXCreateSphere(pd3dDevice, (float)g_ball.r, 8, 8, &g_ball.mesh, 0));
	return hr;
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
	// Setup main camera
	D3DXVECTOR3 vecEye( 0.0f, -5.0f, -5.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
	g_camera.SetViewParams( &vecEye, &vecAt );
	

	// Load sample image (vertex and index buffer creation with texture)
	g_pic.init(L"bigtile.tga", pd3dDevice, 10);
	g_pic.setSize(32, 32);

	makeBallMesh(pd3dDevice);
	//makeBipedMesh(pd3dDevice);
	makeArmMesh(pd3dDevice);

	makeBlendedMesh(pd3dDevice);


	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 2000.0f );
	
    return S_OK;
}


void frameMoveOde(float fElapsedTime)
{
	PcontrolArm();

	dSpaceCollide(g_space,0,&nearCallback);  // 衝突検出関数
	dWorldStep(g_world, fElapsedTime);
	dJointGroupEmpty(g_contactgroup); // ジョイントグループを空にする
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_pic.frameMove(fElapsedTime);
	g_camera.FrameMove(fElapsedTime);

	frameMoveOde(fElapsedTime);
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
		g_pic.draw();
		
		D3DXMATRIX worldMat;
		const dReal* ballPos = dBodyGetPosition(g_ball.body);
		D3DXMatrixTranslation(&worldMat, (float)ballPos[0], (float)ballPos[1], -(float)ballPos[2]);
		pd3dDevice->SetTransform(D3DTS_WORLD, &worldMat);

		//pd3dDevice->SetTexture(0, 0);
		g_ball.mesh->DrawSubset(0);

		//renderBiped(pd3dDevice);
		renderArm(pd3dDevice);

		//renderIndexedPrimitives(pd3dDevice);

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
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
	g_pic.release();

	//releaseBipedMesh();
	releaseArmMesh();

	SAFE_RELEASE( g_ball.mesh );
	SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE( g_pConstantTable );
	SAFE_RELEASE( g_pVertexDeclaration );

	SAFE_RELEASE( g_blendedVB );
	SAFE_RELEASE( g_blendedIB );
	SAFE_RELEASE( g_decl );
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
}

void initializeOde()
{
	dInitODE();
	g_world        = dWorldCreate();
	g_space        = dHashSpaceCreate(0);
	
	g_contactgroup = dJointGroupCreate(0);
	g_ground       = dCreatePlane(g_space,0,0,1,0);
	dWorldSetGravity(g_world, 0, 0, -9.8);
	dWorldSetCFM(g_world, 1e-3);
	dWorldSetERP(g_world, 0.9);
	makeBallBody();

	//makeBipedBody(g_world, g_space);
	makeArmBody(g_world, g_space);
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

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"EmptyProject" );
    DXUTCreateDevice( true, 640, 480 );

    // Start the render loop
    DXUTMainLoop();

    // TODO: Perform any application-level cleanup here
	destroyOde();

    return DXUTGetExitCode();
}

