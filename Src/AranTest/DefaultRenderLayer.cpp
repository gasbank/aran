#include "AranPCH.h"
#include "DefaultRenderLayer.h"
#include "VideoMan.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "ResourceMan.h"
#include "Character.h"
#include "ArnFile.h"
#include "ArnSceneGraph.h"
#include "ArnCamera.h"

DefaultRenderLayer::DefaultRenderLayer(Aran::Character* pChar)
{
	m_pChar = pChar;
}

DefaultRenderLayer::~DefaultRenderLayer(void)
{
}

HRESULT DefaultRenderLayer::render(double fTime, float fElapsedTime)
{
	ASSERTCHECK( m_lpDev );

	static D3DXMATRIX matTranslation, matRotation, matScaling;
	static D3DXMATRIX matWorld, matView, matProjection;

	// World
	D3DXMatrixIdentity(&matWorld);

	// View
	D3DXMatrixLookAtLH(
		&matView,
		&m_pVideoMan->getMainCamera()->eye,		// the camera position
		&m_pVideoMan->getMainCamera()->at,		// the look-at position
		&m_pVideoMan->getMainCamera()->up		// the up direction
		);

	// Projection
	int screenWidth, screenHeight;
	m_pVideoMan->getScreenInfo( screenWidth, screenHeight );
	D3DXMatrixPerspectiveFovLH(
		&matProjection,
		D3DXToRadian(45),
		(float)screenWidth / (float)screenHeight,
		1.0f,
		1000.0f
		);

	m_pVideoMan->setWorldViewProjection(matWorld, matView, matProjection);



	const ResourceMan& resMan = ResourceMan::getSingleton();
	
	// Hero Character

	m_pVideoMan->GetDev()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	D3DXMatrixTranslation(&matTranslation, -20.0f, 0.0f, 0.0f );
	D3DXMatrixScaling( &matScaling, 0.1f, 0.1f, 0.1f );
	const D3DXMATRIX finalXform = matScaling * matTranslation * *(m_pChar->GetFinalTransform()) * *m_pVideoMan->getModelArcBallRotation();
	m_pVideoMan->RenderModel(resMan.getModel( ResourceMan::MAN ), &finalXform);
	const ModelReader* manModel = resMan.getModel( ResourceMan::MAN );
	if (manModel)
		manModel->AdvanceTime( 0.1f );

	//////////////////////////////////////////////////////////////////////////
	// Print Text
	//////////////////////////////////////////////////////////////////////////
	static TCHAR debugBuffer[512];
	RECT rc;

	int scrWidth, scrHeight;
	m_pVideoMan->getScreenInfo(scrWidth, scrHeight);
	SetRect(&rc, 0, 0, scrWidth, scrHeight);
	TCHAR msg[128];
	StringCchPrintf(msg, 128, L"%.2f FPS", m_pVideoMan->getFPS());
	m_pVideoMan->getDefaultFont()->DrawText( 0, msg, -1, &rc, DT_BOTTOM | DT_RIGHT | DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

HRESULT BoxRenderLayer::render(double fTime, float fElapsedTime)
{
	ASSERTCHECK( m_lpDev );
	D3DXMATRIX mWorld, mView, mProjection;

	VideoMan& videoMan = VideoMan::getSingleton();
	LPDIRECT3DDEVICE9 dev = videoMan.GetDev();
	// World
	D3DXMatrixIdentity(&mWorld);
	
	// View
	//D3DXMatrixLookAtLH(
	//	&m_matView,
	//	&videoMan.getMainCamera()->eye,		// the camera position
	//	&videoMan.getMainCamera()->at,		// the look-at position
	//	&videoMan.getMainCamera()->up		// the up direction
	//	);
	ArnCamera* cam = static_cast<ArnCamera*>( m_simpleSG->getSceneRoot()->getNodeByName( "Camera.001" ) );
	
	D3DXMATRIX camXform = cam->getFinalLocalXform();
	D3DXVECTOR3 pos = *(D3DXVECTOR3*)&camXform._41;
	
	
	D3DXVECTOR3 up = DX_CONSTS::D3DXVEC3_Y;
	D3DXVECTOR3 look = DX_CONSTS::D3DXVEC3_Z;
	
	D3DXVec3TransformCoord( &look, &look, &camXform );

	camXform._41 = camXform._42 = camXform._43 = 0; 
	D3DXVec3TransformCoord( &up, &up, &camXform );

	
	D3DXMatrixLookAtLH( &mView, &pos, &look, &up );
	dev->SetTransform( D3DTS_VIEW, &mView );
	
	// Projection
	int screenWidth, screenHeight;
	videoMan.getScreenInfo( screenWidth, screenHeight );
	D3DXMatrixPerspectiveFovLH(
		&mProjection,
		D3DXToRadian( 49.14f ),
		(float)screenWidth / (float)screenHeight,
		0.1f,
		1000.0f
		);
	dev->SetTransform( D3DTS_PROJECTION, &mProjection );

	//m_pVideoMan->setWorldViewProjection(m_matWorld, m_matView, m_matProjection);

	D3DXMATRIX transform;
	D3DXMatrixIdentity(&transform);
	dev->SetTransform(D3DTS_WORLD, &transform);
	
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dev->SetFVF(ArnVertex::FVF);
	videoMan.renderMeshesOnly(m_simpleSG->getSceneRoot());

	ArnMesh* door = dynamic_cast<ArnMesh*>( m_simpleSG->getSceneRoot()->getNodeByName( "Needle" ) );
	ArnMesh* tofuMan = dynamic_cast<ArnMesh*>( m_simpleSG->getSceneRoot()->getNodeByName( "Needle" ) );
	if ( GetAsyncKeyState( VK_RIGHT ) )
	{
		door->setDoAnim( true );
		tofuMan->setDoAnim( true );
	}
	if ( GetAsyncKeyState( VK_LEFT ) )
	{
		door->setDoAnim( false );
		tofuMan->setDoAnim( false );
	}
	if ( GetAsyncKeyState( VK_DOWN ) )
	{
		door->setAnimCtrlTime( 0 );
		tofuMan->setAnimCtrlTime( 0 );
	}
	m_simpleSG->getSceneRoot()->update(fTime, fElapsedTime);

	RECT rc;
	SetRect(&rc, 0, 0, 0, 0);
	StringCchPrintf(m_debugBuffer, 128, L"Aran Renderer");
	m_pVideoMan->getDefaultFont()->DrawText( 0, m_debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 1.0f, 1.0f ) );
	return S_OK;
}

BoxRenderLayer::BoxRenderLayer()
: m_testMesh(0), m_arnFileData(0), m_simpleSG(0)
{
	m_arnFileData = new ArnFileData;
	load_arnfile(_T("models/TestStage.arn"), *m_arnFileData);
	m_simpleSG = new ArnSceneGraph(*m_arnFileData);

	m_skelSceneData = new ArnFileData;
	load_arnfile(_T("c:/Scene.arn"), *m_skelSceneData);
	m_skelScene = new ArnSceneGraph(*m_skelSceneData);
	//m_skelSceneData = 0;
	//m_skelScene = 0;
}

BoxRenderLayer::~BoxRenderLayer()
{
	size_t i;
	for (i = 0; i < m_objects.size(); ++i)
	{
		SAFE_DELETE(m_objects[i]);
	}

	SAFE_RELEASE(m_testMesh);

	if (m_arnFileData)
		release_arnfile(*m_arnFileData);
	delete m_arnFileData;
	delete m_simpleSG;

	if (m_skelSceneData)
		release_arnfile(*m_skelSceneData);
	delete m_skelSceneData;
	delete m_skelScene;
}

LPD3DXMESH newTestPlaneMesh(float width, float height, int segWidth, int segHeight)
{	
	ArnVertex* v = 0;
	WORD* ind = 0;
	int faceCount = segWidth * segHeight * 2;
	int vertexCount = (segWidth+1) * (segHeight+1);
	HRESULT hr = S_OK;
	LPD3DXMESH mesh = 0;
	hr = D3DXCreateMeshFVF(faceCount, vertexCount, D3DXMESH_MANAGED, ARNVERTEX_FVF, VideoMan::getSingleton().GetDev(), &mesh);
	if (FAILED(hr))
		throw new std::runtime_error("D3DXCreateMeshFVF() error");
	
	int i, j;
	float segWidthSize = width / segWidth;
	float segHeightSize = height / segHeight;
	std::vector<ArnVertex> vertices(vertexCount);
	for (i = 0; i <= segHeight; ++i)
	{
		for (j = 0; j <= segWidth; ++j)
		{
			ArnVertex* curV = &vertices[i * (segWidth+1) + j];
			curV->x = segWidthSize * j;
			curV->y = segHeightSize * i;
			curV->z = 0.0f;
			curV->nx = 0.0f;
			curV->ny = 0.0f;
			curV->nz = -1.0f;
		}
	}
	/*
	 *    v0---v1---v2
	 *    |a / |c / |
	 *    | / b| / d|
	 *    v3---v4---v5
	 *    |e / |g / |
	 *    | / f| / h|
	 *    v6---v7---v8
	 *
	 * a, b, c, d, e, f, g, h
	 * (each triangle counterclockwise)
	 */
	std::vector<WORD> indices(faceCount * 3);
	for (i = 0; i < segHeight; ++i)
	{
		for (j = 0; j < segWidth; ++j)
		{
			int v0Index = i * (segWidth+1) + j;
			WORD* curI = &indices[6 * (i * segWidth + j)];
			curI[0] = (WORD)(v0Index);
			curI[1] = (WORD)(v0Index + (segWidth+1));
			curI[2] = (WORD)(v0Index + 1);

			curI[3] = (WORD)(v0Index + 1);
			curI[4] = (WORD)(v0Index + (segWidth+1));
			curI[5] = (WORD)(v0Index + (segWidth+1) + 1);
		}
	}

	mesh->LockVertexBuffer(0, (void**)&v);
	memcpy(v, &vertices[0], vertexCount * sizeof(ArnVertex));
	mesh->UnlockVertexBuffer();

	mesh->LockIndexBuffer(0, (void**)&ind);
	memcpy(ind, &indices[0], faceCount * 3 * sizeof(WORD));
	mesh->UnlockIndexBuffer();
	return mesh;
}