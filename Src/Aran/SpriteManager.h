﻿#pragma once

class Sprite;

typedef std::map<std::string, Sprite*> SpriteMap;

/**
@brief Sprite 인스턴스를 관리하는 클래스

2D 이미지를 로드 및 출력하는 Sprite 클래스 인스턴스를 등록하거나 가져올 수 있습니다.
*/
class SpriteManager : public Singleton<SpriteManager>
{
public:
	SpriteManager();
	~SpriteManager(void);
	
	/**
	@brief 등록된 모든 Sprite 의 DrawRequest 를 렌더링한다.
	@remarks DrawRequest 중 bRender 가 true인 것만 렌더링 된다.
	*/
	void frameRender();

	/**
	@brief 특정한 Sprite 의 DrawRequest 를 렌더링한다.
	@remarks 여기서 정한 spriteName 의 custom rendering 플레그가 true 여야만 한다.
	*/
	void frameRenderSpecificSprite( const char* spriteName );
	Sprite* registerSprite( const char* spriteName, const char* spriteFileName );

	Sprite* getSprite( const char* spriteName ) const;

	LPD3DXSPRITE getSpriteRenerer() const { return m_d3dxSprite; }

	HRESULT onCreateDevice( IDirect3DDevice9* pd3dDevice );
	HRESULT onResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
							void* pUserContext );
	void onLostDevice();
	void onDestroyDevice();

private:
	void init();
	void release();

	LPDIRECT3DDEVICE9		m_dev;
	LPD3DXSPRITE			m_d3dxSprite;			// Screen space sprite
	LPD3DXSPRITE			m_d3dxObjectSprite;		// Object space sprite
	SpriteMap				m_spriteMap;			// Automatically rendered set of Sprites. Rendering is done at the last part of frameRender.
	D3DXMATRIX				m_viewMat;
	D3DXMATRIX				m_projMat;
};

inline SpriteManager& GetSpriteManager() { return SpriteManager::getSingleton(); }