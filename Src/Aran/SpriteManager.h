#pragma once

class VideoMan;
class Sprite;

typedef std::map<std::string, Sprite*> SpriteMap;

/**
@brief Manager for Sprite class instances

*/
class SpriteManager : public Singleton<SpriteManager>
{
public:
	SpriteManager();
	~SpriteManager(void);

	void frameRender();
	void frameRenderSpecificSprite( const char* spriteName );
	Sprite* registerSprite( const char* spriteName, const char* spriteFileName );
	Sprite* getSprite( const char* spriteName ) const;
	Sprite* getSpriteRenerer() const { return m_d3dxSprite; }

	HRESULT onCreateDevice( VideoMan* vm);
	HRESULT onResetDevice( VideoMan* vm );
	void onLostDevice();
	void onDestroyDevice();

private:
	void init();
	void release();

	VideoMan*				m_dev;
	Sprite*					m_d3dxSprite;			// Screen space sprite
	Sprite*					m_d3dxObjectSprite;		// Object space sprite
	SpriteMap				m_spriteMap;			// Automatically rendered set of Sprites. Rendering is done at the last part of frameRender.
	ArnMatrix				m_viewMat;
	ArnMatrix				m_projMat;
};

inline SpriteManager& GetSpriteManager() { return SpriteManager::getSingleton(); }

