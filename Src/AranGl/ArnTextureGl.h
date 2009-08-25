#pragma once

#include "ArnRenderableObject.h"
class ArnTexture;
class ArnTextureGl : public ArnRenderableObject
{
public:
											~ArnTextureGl(void);
	static ArnTextureGl*					createFrom(const ArnTexture* tex);
	virtual int								render(bool bIncludeShadeless) const;
	virtual void							cleanup();
private:
											ArnTextureGl(void);
	int										init();
	GLuint									getTextureId() const { return m_textureId; }
	GLuint									m_textureId;
	const ArnTexture*						m_target;
};

ARANGL_API void ConfigureRenderableObjectOf( ArnTexture* tex );
