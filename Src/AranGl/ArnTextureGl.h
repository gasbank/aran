#pragma once

#include "ArnTexture.h"

class ArnTextureGl : public ArnTexture
{
public:
											~ArnTextureGl(void);
	virtual bool							initialize();
	virtual void							Release();
	GLuint									getTextureId() const { return m_textureId; }
private:
											ArnTextureGl(void);
	GLuint									m_textureId;
};
