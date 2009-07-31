#include "AranGlPCH.h"
#include "ArnTextureGl.h"

ArnTextureGl::ArnTextureGl(void)
{
}

ArnTextureGl::~ArnTextureGl(void)
{
}

bool ArnTextureGl::initialize()
{
	int width = 0, height = 0;
	unsigned char * data = 0;
	const char* texFileName = getFileName();
	const char* texFileNameExt = texFileName + strlen(texFileName) - 4;

	if (strcmp(texFileNameExt, ".ppm") == 0)
	{
		ArnLoadFromPpmFile(&data, &width, &height, texFileName);
	}
	else if (strcmp(texFileNameExt, ".raw") == 0)
	{
		// open texture data
		FILE * file = fopen( texFileName, "rb" );
		if ( file == NULL )
			return false;

		// allocate buffer
		width = 256;
		height = 256;
		data = (unsigned char*)malloc( width * height * 3 );

		// read texture data
		fread( data, width * height * 3, 1, file );
		fclose( file );
	}
	else if (strcmp(texFileNameExt, ".png") == 0)
	{
		ILuint handle;
		ilGenImages(1, &handle);
		ilBindImage(handle);
		ILboolean result = ilLoadImage(m_fileName.c_str());
		if (result == IL_FALSE)
		{
			fprintf(stderr, " *** Texture file is not loaded correctly: %s\n", m_fileName.c_str());
			return false;
		}
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		data = (unsigned char*)malloc( width * height * 3 );
		ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
		ilDeleteImages(1, &handle);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
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

	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );

	// build our texture MIP maps
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );
	glBindTexture( GL_TEXTURE_2D, 0 );
	// free buffer
	free( data );
	m_inited = true;
	return true;
}

void ArnTextureGl::Release()
{
	glDeleteTextures(1, &m_textureId);
}
