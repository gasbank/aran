#pragma once

#include "ArnRenderableObject.h"

class ARANDX9_API ArnTextureDx9 : public ArnRenderableObject
{
public:
											~ArnTextureDx9(void);
	static ArnTextureDx9*					createFrom(const ArnTexture* tex);
	virtual int								render(bool bIncludeShadeless) const;
	virtual void							cleanup();

	LPDIRECT3DTEXTURE9						getDxTexture() const { return m_d3d9Tex; }
	void									setDxTexture(LPDIRECT3DTEXTURE9 tex) { m_d3d9Tex = tex; }

	bool									initialize();
	bool									Release();
private:
											ArnTextureDx9(void);
	int										init();
	LPDIRECT3DTEXTURE9						m_d3d9Tex;
	const ArnTexture*						m_target;
};
