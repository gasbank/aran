#include "AranPCH.h"
#include "ArnTexture.h"
#include "VideoMan.h"

#include "IL/il.h"

ArnTexture::ArnTexture(const char* texFileName)
: m_fileName(texFileName)
, m_inited(false)
, m_d3d9Tex(0)
{
}

ArnTexture::~ArnTexture(void)
{
	Release();
#ifdef WIN32
	SAFE_RELEASE(m_d3d9Tex);
#endif
}

ArnTexture* ArnTexture::createFrom(const char* texFileName)
{
	ArnTexture* ret = new ArnTexture(texFileName);
	// TODO: Texture instantiation should not be done at this stage...
	//ArnCreateTextureFromFile(&GetVideoManager(), ret->m_fileName.c_str(), &ret);
	return ret;
}

bool ArnTexture::initGl()
{
	int width = 0, height = 0;
	unsigned char * data = 0;
	const char* texFileName = m_fileName.c_str();
	const char* texFileNameExt = texFileName + strlen(texFileName) - 4;

	if (strcmp(texFileNameExt, ".ppm") == 0)
	{
		ArnLoadFromPpmFile(&data, &width, &height, texFileName);
	}
	else if (strcmp(texFileNameExt, ".raw") == 0)
	{
		// open texture data
		FILE * file = fopen( m_fileName.c_str(), "rb" );
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

void ArnTexture::Release()
{
	glDeleteTextures(1, &m_textureId);
}

#ifdef WIN32
HRESULT ArnCreateTextureFromFile( VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture )
{
	LPDIRECT3DTEXTURE9 tex;
	D3DXCreateTextureFromFileA(pDevice->GetDev(), pSrcFile, &tex);
	(*ppTexture)->setDxTexture(tex);
	return S_OK;
}
#else
HRESULT ArnCreateTextureFromFile( VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
#endif

void ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName)
{
	assert(*buff == 0);
	FILE* f = 0;
	f = fopen(fileName, "rb");
	if (!f)
	{
		fprintf(stderr, "Cannot open texture image file. : %s\n", fileName);
		return;
	}
	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char aLine[256];
	int maxIntensity;
	fscanf(f, "%s\n", aLine);  /* magic number for PPM files */
	if (strcmp(aLine, "P6") != 0)
	{
		fprintf(stderr, "This file has no proper header for PPM files. : %s\n", fileName);
		return;
	}
	fscanf(f, "%[^\n]", aLine); // Throw away a dummy line (GIMP specific)
	fscanf(f, "%i %i\n", width, height);
	fscanf(f, "%i\n", &maxIntensity);  /* max intensity value */
	assert(maxIntensity == 255);
	*buff = (unsigned char*)malloc(3 * (*width) * (*height));
	fread(*buff, 3, (*width) * (*height), f);
	assert(ftell(f) == (int)fileSize);
	fclose(f);
	return;
}
