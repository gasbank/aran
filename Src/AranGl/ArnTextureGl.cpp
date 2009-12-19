#include "AranGlPCH.h"
#include "ArnTextureGl.h"
#include "ArnTexture.h"

ArnTextureGl::ArnTextureGl(void)
: m_textureId(0)
, m_target(0)
{
}

ArnTextureGl::~ArnTextureGl(void)
{
}

int
ArnTextureGl::init()
{
	assert(m_target);
	assert(m_target->isInitialized());
	m_textureId = 0;

	// build our texture MIP maps
	int fmt = 0;
	if (m_target->getFormat() == ACF_RGB)
		fmt = GL_RGB;
	else if (m_target->getFormat() == ACF_RGBA)
		fmt = GL_RGBA;
	else if (m_target->getFormat() == ACF_BGR)
		fmt = GL_BGR;
	else if (m_target->getFormat() == ACF_BGRA)
		fmt = GL_BGRA;
	else
	{
		// Invalid texture
		return -5;
	}
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
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLfloat>(m_target->isWrap() ? GL_REPEAT : GL_CLAMP) );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLfloat>(m_target->isWrap() ? GL_REPEAT : GL_CLAMP) );

	const char *fileName = m_target->getFileName ();
	assert (fileName);
	const size_t fileNameLen = strlen (fileName);
	assert (fileNameLen > 0);
	if (strcmp (&fileName[fileNameLen - 4], ".tga") == 0)
	{
		// TODO: DevIL TGA loading bug? Y-axis flip is needed.
		unsigned char *flipped = new unsigned char[m_target->getBpp () * m_target->getWidth () * m_target->getHeight ()];
		for (unsigned int i = 0; i < m_target->getHeight (); ++i)
		{
			memcpy (&flipped[ m_target->getBpp () * m_target->getWidth () * (m_target->getHeight () - 1 - i) ],
				&m_target->getRawData()[ m_target->getBpp () * m_target->getWidth () * i ],
				m_target->getBpp () * m_target->getWidth ());
		}
		gluBuild2DMipmaps( GL_TEXTURE_2D, m_target->getBpp(), m_target->getWidth(), m_target->getHeight(), fmt, GL_UNSIGNED_BYTE, flipped );
	}
	else
	{
		gluBuild2DMipmaps( GL_TEXTURE_2D, m_target->getBpp(), m_target->getWidth(), m_target->getHeight(), fmt, GL_UNSIGNED_BYTE, &m_target->getRawData()[0] );
	}
	glBindTexture( GL_TEXTURE_2D, 0 );
	return 0;
}

int
ArnTextureGl::render(bool bIncludeShadeless) const
{
	// 'bIncludeShadeless' unused.
	glBindTexture(GL_TEXTURE_2D, getTextureId());
	GLenum err = glGetError( );
	assert(err == 0);
	return 0;
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
	if (ret->init() < 0)
	{
		delete ret;
		printf("ArnTextureGl initialization failed!\n");
		return 0;
	}
	else
	{
		ret->setInitialized(true);
		ret->setRendererType(RENDERER_GL);
		return ret;
	}
}

void ConfigureRenderableObjectOf( ArnTexture* tex )
{
	tex->attachChild( ArnTextureGl::createFrom(tex) );
}
