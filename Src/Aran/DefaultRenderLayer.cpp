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
	
	// Rendering using Fixed Shader with animations [ARN format]
	D3DXMatrixTranslation(&matTranslation, 0.0f, 10.0f, 0.0f);
	m_pVideoMan->RenderModel( resMan.getModel( ResourceMan::BOX1 ), &(matTranslation * *m_pVideoMan->getModelArcBallRotation() ));
	D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f, 10.0f);
	m_pVideoMan->RenderModel( resMan.getModel( ResourceMan::BOX1 ), &(matTranslation * *m_pVideoMan->getModelArcBallRotation() ));
	resMan.getModel( ResourceMan::BOX1 )->AdvanceTime(0.1f);


	// Hero Character
	D3DXMatrixTranslation(&matTranslation, -20.0f, 0.0f, 0.0f );
	D3DXMatrixScaling( &matScaling, 0.1f, 0.1f, 0.1f );
	m_pVideoMan->RenderModel(
		resMan.getModel( ResourceMan::MAN ),
		&( matScaling * matTranslation * *(m_pChar->GetFinalTransform()) * *m_pVideoMan->getModelArcBallRotation() )
		);
	resMan.getModel( ResourceMan::MAN )->AdvanceTime( 0.1f );

	// Moma Lenguin
	/*D3DXMatrixTranslation(&matTranslation, 5.0f, 0.0f, 0.0f );
	D3DXMatrixScaling( &matScaling, 0.001f, 0.001f, 0.001f );
	m_pVideoMan->RenderModel(
		resMan.getModel( ResourceMan::MOMA ),
		&( matScaling * matTranslation * *m_pVideoMan->getModelArcBallRotation() )
		);
	resMan.getModel( ResourceMan::MOMA )->AdvanceTime( 0.1f );*/

	// Box Skin
	D3DXMatrixTranslation(&matTranslation, 5.0f, 0.0f, 0.0f );
	D3DXMatrixScaling( &matScaling, 0.3f, 0.3f, 0.3f );
	m_pVideoMan->RenderModel(
		resMan.getModel( ResourceMan::BOXSKIN ),
		&( matScaling * matTranslation * *m_pVideoMan->getModelArcBallRotation() )
		);
	resMan.getModel( ResourceMan::BOXSKIN )->AdvanceTime( 0.1f );


	//////////////////////////////////////////////////////////////////////////
	// Dungeon(Level)
	//////////////////////////////////////////////////////////////////////////
	//D3DXMatrixTranslation(
	//	&matTranslation,
	//	m_pVideoMan->dungeonTranslation.x,
	//	m_pVideoMan->dungeonTranslation.y,
	//	m_pVideoMan->dungeonTranslation.z
	//	);
	//D3DXMatrixScaling( &matScaling, 0.3f, 0.3f, 0.3f );
	////m_pVideoMan->RenderModel( &m_pVideoMan->mrMan, &(matScaling * m_pVideoMan->modelArcBallRotation * matTranslation) );
	//m_pVideoMan->RenderModel( &m_pVideoMan->mrDungeon, &(matTranslation * matScaling) );
	//m_pVideoMan->mrDungeon.AdvanceTime( 0.1f );



	D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f, 100.0f);
	m_pVideoMan->RenderModel(resMan.getModel( ResourceMan::BIGHOUSE ), &matTranslation); // House background
	resMan.getModel( ResourceMan::BIGHOUSE )->AdvanceTime( 0.1f );



	// Animation Controller
	m_pVideoMan->lpAnimationController->AdvanceTime(0.1f, NULL);


	//////////////////////////////////////////////////////////////////////////
	// Print Text
	//////////////////////////////////////////////////////////////////////////
	static TCHAR debugBuffer[512];
	RECT rc;

	SetRect( &rc, 50, 50, 0, 0 );
	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Umhahahah....a~~~~") );
	m_pVideoMan->getDefaultFont()->DrawText(	NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );

	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Current State: %d"), m_pChar->GetCharacterAnimationState() );
	SetRect( &rc, 50, 80, 0, 0 );
	m_pVideoMan->getDefaultFont()->DrawText(	NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
	_stprintf_s( debugBuffer, sizeof( debugBuffer ) / sizeof( TCHAR ), _T("Next State: %d"), m_pChar->GetCharacterAnimationStateNext() );
	SetRect( &rc, 50, 110, 0, 0 );
	m_pVideoMan->getDefaultFont()->DrawText(	NULL, debugBuffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return S_OK;


}