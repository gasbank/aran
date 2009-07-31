#include "AranGlPCH.h"
#include "ArnTextureGl.h"
#include "ArnTexture.h"

ArnTextureGl::ArnTextureGl(void)
{
}

ArnTextureGl::~ArnTextureGl(void)
{
}

int
ArnTextureGl::init()
{
	assert(m_target);
	int width = 0, height = 0;
	unsigned char * data = 0;
	const char* texFileName = m_target->getFileName();
	const char* texFileNameExt = texFileName + strlen(texFileName) - 4;

	ArnTextureGetRawDataFromimageFile(&data, &width, &height, m_target->getFileName());

	// allocate a texture name
	glGenTextures( 1, &m_textureId );

	// select our current texture
	glBindTexture( GL_TEXTURE_2D, m_textureId );

	// select modulate to mix texture with color for shading
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)

	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );

	// build our texture MIP maps
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );
	glBindTexture( GL_TEXTURE_2D, 0 );
	// free buffer
	free( data );
	return 0;
}

int
ArnTextureGl::render()
{
	ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ArnTextureGl::cleanup()
{
	glDeleteTextures(1, &m_textureId);
}

ArnTextureGl* ArnTextureGl::createFrom( const ArnTexture* tex )
{
	ArnTextureGl* ret = new ArnTextureGl();
	ret->m_target = tex;
	ret->init();
	ret->setInitialized(true);
	ret->setRendererType(RENDERER_GL);
	return ret;
}
