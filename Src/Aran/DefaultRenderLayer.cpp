#include "StdAfx.h"
#include "DefaultRenderLayer.h"
#include "VideoMan.h"
#include "load_arn.h"
#include "arn2.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "ResourceMan.h"
#include "Character.h"

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
	
	// Hero Character
	m_pVideoMan->GetDev()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	D3DXMatrixTranslation(&matTranslation, -20.0f, 0.0f, 0.0f );
	D3DXMatrixScaling( &matScaling, 0.1f, 0.1f, 0.1f );
	m_pVideoMan->RenderModel(
		resMan.getModel( ResourceMan::MAN ),
		&( matScaling * matTranslation * *(m_pChar->GetFinalTransform()) * *m_pVideoMan->getModelArcBallRotation() )
		);
	resMan.getModel( ResourceMan::MAN )->AdvanceTime( 0.1f );

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
	VideoMan& videoMan = VideoMan::getSingleton();
	LPDIRECT3DDEVICE9 dev = videoMan.GetDev();
	// World
	D3DXMatrixIdentity(&m_matWorld);
	// View
	D3DXMatrixLookAtLH(
		&m_matView,
		&videoMan.getMainCamera()->eye,		// the camera position
		&videoMan.getMainCamera()->at,		// the look-at position
		&videoMan.getMainCamera()->up		// the up direction
		);
	// Projection
	int screenWidth, screenHeight;
	videoMan.getScreenInfo( screenWidth, screenHeight );
	D3DXMatrixPerspectiveFovLH(
		&m_matProjection,
		videoMan.getMainCamera()->angle,
		(float)screenWidth / (float)screenHeight,
		videoMan.getMainCamera()->nearClip,
		videoMan.getMainCamera()->farClip
		);
	m_pVideoMan->setWorldViewProjection(m_matWorld, m_matView, m_matProjection);

	D3DXMATRIX transform;
	D3DXMatrixIdentity(&transform);
	dev->SetTransform(D3DTS_WORLD, &transform);
	
	//dev->SetFVF(ARNVERTEX_FVF);
	//dev->SetMaterial(VideoMan::getSingleton().getDefaultMaterial());
	//D3DXMatrixRotationYawPitchRoll(&transform, D3DX_PI*0.8f, D3DX_PI/6, 0.0f);
	
	D3DXMATRIX flip;
	flip.m[0][0] = 1; flip.m[0][1] = 0; flip.m[0][2] = 0; flip.m[0][3] = 0; 
	flip.m[1][0] = 0; flip.m[1][1] = 1; flip.m[1][2] = 0; flip.m[1][3] = 0; 
	flip.m[2][0] = 0; flip.m[2][1] = 0; flip.m[2][2] = -1; flip.m[2][3] = 0; 
	flip.m[3][0] = 0; flip.m[3][1] = 0; flip.m[3][2] = 0; flip.m[3][3] = 1; 
	
	D3DXMATRIX yRot;
	D3DXMatrixRotationY(&yRot, D3DX_PI);

	D3DXMATRIX change = flip * yRot;
	//dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	unsigned int i, j;
	for (i = 0; i < m_objects.size(); ++i)
	{
		if (m_objects[i]->getType() == ANT_MESH)
		{
			ArnMesh* mesh = (ArnMesh*)m_objects[i];
			D3DXMatrixIdentity(&transform);
			ArnNode* parNode = mesh->getParent();
			while (parNode != NULL)
			{
				transform = *parNode->getLocalTransform() * transform;
				parNode = parNode->getParent();
			}
			transform = *mesh->getLocalTransform() * transform;

			
			/*D3DXVECTOR3* scalingVec = (D3DXVECTOR3*)mesh->getOb().hdr->scl;
			D3DXVECTOR3* translationVec = (D3DXVECTOR3*)mesh->getOb().hdr->loc;
			D3DXQUATERNION* rotQuat = (D3DXQUATERNION*)mesh->getOb().hdr->rotQuat;
			D3DXMatrixTransformation(&transform, NULL, NULL, scalingVec, NULL, rotQuat, translationVec);
			*/
			transform = transform * *VideoMan::getSingleton().getArcballResult();
			dev->SetTransform(D3DTS_WORLD, &transform);

			for (j = 0; j < mesh->getOb().hdr->materialCount; ++j)
			{
				unsigned int maIndex = mesh->getOb().attrToMaterialMap[j];
				ArnMaterial* ma = dynamic_cast<ArnMaterial*>(m_objects[maIndex]);
				dev->SetMaterial(&ma->getOb().hdr->d3dMaterial);
				mesh->getD3DMesh()->DrawSubset(j);
			}
		}
	}
	dev->SetTransform(D3DTS_WORLD, &transform);
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
	VideoMan& videoMan = VideoMan::getSingleton();
	if (FAILED(load_arn("models/gus2.arn", m_objects)))
	{
		DebugBreak();
	}
	size_t i;
	ArnCamera* arnCam = NULL;
	for (i = 0; i < m_objects.size(); ++i)
	{
		ArnMesh* mesh = dynamic_cast<ArnMesh*>(m_objects[i]);
		if (mesh)
		{
			arn_build_mesh(VideoMan::getSingleton().GetDev(), &mesh->getOb(), &mesh->getD3DMesh());
		}
		if (!arnCam && m_objects[i]->getType() == ANT_CAMERA)
		{
			arnCam = reinterpret_cast<ArnCamera*>(m_objects[i]);
		}
	}
	m_testMesh = newTestPlaneMesh(2.0f, 4.0f, 20, 10);
	videoMan.GetDev()->SetRenderState(D3DRS_LIGHTING, TRUE);

	if (arnCam)
	{
		videoMan.SetCamera(*arnCam);
	}
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