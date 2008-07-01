#include "StdAfx.h"
#include "DefaultRenderLayer.h"
#include "../VideoLib/VideoMan.h"
#include "../VideoLib/load_arn.h"
#include "../VideoLib/arn2.h"
#include "../VideoLib/ArnMesh.h"

DefaultRenderLayer::DefaultRenderLayer(Character* pChar)
{
	m_pChar = pChar;
}

DefaultRenderLayer::~DefaultRenderLayer(void)
{
}

HRESULT DefaultRenderLayer::render()
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
	
	//// Hero Character
	//D3DXMatrixTranslation(&matTranslation, -20.0f, 0.0f, 0.0f );
	//D3DXMatrixScaling( &matScaling, 0.1f, 0.1f, 0.1f );
	//m_pVideoMan->RenderModel(
	//	resMan.getModel( ResourceMan::MAN ),
	//	&( matScaling * matTranslation * *(m_pChar->GetFinalTransform()) * *m_pVideoMan->getModelArcBallRotation() )
	//	);
	//resMan.getModel( ResourceMan::MAN )->AdvanceTime( 0.1f );

	//// PoolC 3D Logo
	//D3DXMatrixTranslation(&matTranslation, 0.0f, 10.0f, -20.0f );
	//D3DXMatrixScaling( &matScaling, 0.1f, 0.1f, 0.1f );
	//D3DXMatrixRotationX( &matRotation, D3DXToRadian(60));
	//m_pVideoMan->RenderModel(
	//	resMan.getModel( ResourceMan::POOLC ),
	//	&( matScaling * matRotation * *(m_pChar->GetFinalTransform()) * *m_pVideoMan->getModelArcBallRotation() * matTranslation)
	//	);
	//resMan.getModel( ResourceMan::POOLC )->AdvanceTime( 0.1f );

	//D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f, 100.0f);
	//m_pVideoMan->RenderModel(resMan.getModel( ResourceMan::BIGHOUSE ), &matTranslation); // House background
	//resMan.getModel( ResourceMan::BIGHOUSE )->AdvanceTime( 0.1f );


	//////////////////////////////////////////////////////////////////////////
	// Print Text
	//////////////////////////////////////////////////////////////////////////
	static TCHAR debugBuffer[512];
	RECT rc;

	SetRect( &rc, 50, 50, 0, 0 );
	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Umhahahah....a~~~~") );
	m_pVideoMan->getDefaultFont()->DrawText( NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );

	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Current State: %d"), m_pChar->GetCharacterAnimationState() );
	SetRect( &rc, 50, 80, 0, 0 );
	m_pVideoMan->getDefaultFont()->DrawText( NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Next State: %d"), m_pChar->GetCharacterAnimationStateNext() );
	SetRect( &rc, 50, 110, 0, 0 );
	m_pVideoMan->getDefaultFont()->DrawText( NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

HRESULT BoxRenderLayer::render()
{
	ASSERTCHECK( m_lpDev );
	// World
	D3DXMatrixIdentity(&m_matWorld);
	// View
	D3DXMatrixLookAtLH(
		&m_matView,
		&m_pVideoMan->getMainCamera()->eye,		// the camera position
		&m_pVideoMan->getMainCamera()->at,		// the look-at position
		&m_pVideoMan->getMainCamera()->up		// the up direction
		);
	// Projection
	int screenWidth, screenHeight;
	m_pVideoMan->getScreenInfo( screenWidth, screenHeight );
	D3DXMatrixPerspectiveFovLH(
		&m_matProjection,
		D3DXToRadian(45),
		(float)screenWidth / (float)screenHeight,
		1.0f,
		1000.0f
		);
	m_pVideoMan->setWorldViewProjection(m_matWorld, m_matView, m_matProjection);

	D3DXMATRIX transform;
	D3DXMatrixIdentity(&transform);
	VideoMan::getSingleton().GetDev()->SetTransform(D3DTS_WORLD, &transform);
	
	VideoMan::getSingleton().GetDev()->SetFVF(ARNVERTEX_FVF);
	VideoMan::getSingleton().GetDev()->SetMaterial(VideoMan::getSingleton().getDefaultMaterial());
	D3DXMatrixRotationYawPitchRoll(&transform, D3DX_PI*0.8f, D3DX_PI/6, 0.0f);
	VideoMan::getSingleton().GetDev()->SetTransform(D3DTS_WORLD, &transform);
	size_t i;
	for (i = 0; i < m_objects.size(); ++i)
	{
		ArnMesh* mesh = dynamic_cast<ArnMesh*>(m_objects[i]);
		if (mesh)
		{
			mesh->getD3DMesh()->DrawSubset(0);
		}
	}

	D3DXMatrixIdentity(&transform);
	
	VideoMan::getSingleton().GetDev()->SetTransform(D3DTS_WORLD, &transform);
	//m_testMesh->DrawSubset(0);

	RECT rc;
	SetRect(&rc, 600, 20, 0, 0);
	_stprintf_s( m_debugBuffer, sizeof( m_debugBuffer ) / sizeof( TCHAR ), _T("BoxRenderLayer") );
	m_pVideoMan->getDefaultFont()->DrawText( NULL, m_debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 1.0f, 1.0f ) );
	return S_OK;
}

BoxRenderLayer::BoxRenderLayer()
: m_testMesh(NULL)
{
	if (FAILED(load_arn("models/gus2.arn", m_objects)))
	{
		DebugBreak();
	}
	size_t i;
	for (i = 0; i < m_objects.size(); ++i)
	{
		ArnMesh* mesh = dynamic_cast<ArnMesh*>(m_objects[i]);
		if (mesh)
		{
			arn_build_mesh(VideoMan::getSingleton().GetDev(), &mesh->getOb(), &mesh->getD3DMesh());
		}
	}
	m_testMesh = newTestPlaneMesh(2.0f, 4.0f, 20, 10);
	VideoMan::getSingleton().GetDev()->SetRenderState(D3DRS_LIGHTING, TRUE);
}

BoxRenderLayer::~BoxRenderLayer()
{
	size_t i;
	for (i = 0; i < m_objects.size(); ++i)
	{
		SAFE_DELETE(m_objects[i]);
	}

	SAFE_RELEASE(m_testMesh);
}

LPD3DXMESH newTestPlaneMesh(float width, float height, int segWidth, int segHeight)
{	
	ArnVertex* v = NULL;
	WORD* ind = 0;
	int faceCount = segWidth * segHeight * 2;
	int vertexCount = (segWidth+1) * (segHeight+1);
	HRESULT hr = S_OK;
	LPD3DXMESH mesh = NULL;
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
			curI[0] = v0Index;
			curI[1] = v0Index + (segWidth+1);
			curI[2] = v0Index + 1;

			curI[3] = v0Index + 1;
			curI[4] = v0Index + (segWidth+1);
			curI[5] = v0Index + (segWidth+1) + 1;
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