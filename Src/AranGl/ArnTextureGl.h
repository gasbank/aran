#pragma once

#include "ArnRenderableObject.h"
class ArnTexture;
class ArnTextureGl : public ArnRenderableObject
{
public:
											~ArnTextureGl(void);
	static ArnTextureGl*					createFrom(const ArnTexture* tex);
	//virtual int								initialize();
	virtual int								render();
	virtual void							cleanup();
	GLuint									getTextureId() const { return m_textureId; }
private:
											ArnTextureGl(void);
	int										init();
	GLuint									m_textureId;
	const ArnTexture*						m_target;
};
