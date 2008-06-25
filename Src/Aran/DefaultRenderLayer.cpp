#include "StdAfx.h"
#include "DefaultRenderLayer.h"

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
	//m_lpDev->SetTransform(D3DTS_WORLD, &matWorld);

	// View
	D3DXMatrixLookAtLH(
		&matView,
		&m_pVideoMan->getMainCamera()->eye,					// the camera position
		&m_pVideoMan->getMainCamera()->at,					// the look-at position
		&m_pVideoMan->getMainCamera()->up						// the up direction
		);
	//m_lpDev->SetTransform(D3DTS_VIEW, &matView);

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
	//m_lpDev->SetTransform(D3DTS_PROJECTION, &matProjection);

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