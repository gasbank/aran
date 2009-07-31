#pragma once

#include "ArnTexture.h"

class ArnTextureDx9 : public ArnTexture
{
public:
											~ArnTextureDx9(void);
	virtual bool							initialize();
	virtual void							Release();
	LPDIRECT3DTEXTURE9						getDxTexture() const { return m_d3d9Tex; }
	void									setDxTexture(LPDIRECT3DTEXTURE9 tex) { m_d3d9Tex = tex; }
	
private:
											ArnTextureDx9(void);
	LPDIRECT3DTEXTURE9						m_d3d9Tex;
};
