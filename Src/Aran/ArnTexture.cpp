#include "AranPCH.h"
#include "ArnTexture.h"
#include "VideoMan.h"
//
// DevIL
//
#include "IL/il.h"

static bool gs_ilInitialized = false;

ArnTexture::ArnTexture()
: ArnNode(NDT_RT_TEXTURE)
, m_bInitialized(false)
, m_rawData(0)
, m_width(0)
, m_height(0)
, m_bWrap(false)
{
}

ArnTexture::~ArnTexture(void)
{
}

ArnTexture*
ArnTexture::createFrom(const char* texFileName)
{
	ArnTexture* ret = new ArnTexture();
	ret->m_fileName = texFileName;
	return ret;
}

ArnTexture*
ArnTexture::createFrom(const unsigned char* data, unsigned int width, unsigned int height, unsigned int bpp, bool wrap)
{
	assert(data && width && height && (bpp == 3 || bpp == 4));
	ArnTexture* ret = new ArnTexture();
	// m_rawData has a deep copy of the texture image data.
	ret->m_rawData.resize(width * height * bpp);
	memcpy(&ret->m_rawData[0], data, width * height * bpp);
	ret->m_width = width;
	ret->m_height = height;
	ret->m_bpp = bpp;
	ret->m_bWrap = wrap;
	return ret;
}

void
ArnTexture::interconnect( ArnNode* sceneRoot )
{
}

void
ArnTexture::init()
{
	assert(m_bInitialized == false);
	if (m_fileName.size() && m_rawData.size() == 0) // The path of a texture image is provided.
	{
		ArnTextureGetRawDataFromimageFile(m_rawData, &m_width, &m_height, &m_bpp, m_fileName.c_str());
	}
	else if (m_fileName.size() == 0 && m_rawData.size() && m_width && m_height && m_bpp) // In-memory pointer to raw image data is provided.
	{
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	m_bInitialized = true;
}
//////////////////////////////////////////////////////////////////////////

HRESULT
ArnCreateTextureFromFile( VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR

	//***LPDIRECT3DTEXTURE9 tex;
	//***D3DXCreateTextureFromFileA(pDevice->GetDev(), pSrcFile, &tex);
	//***(*ppTexture)->setDxTexture(tex);
	//***return S_OK;
}

void
ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName)
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

int
ArnInitializeImageLibrary()
{
	if (gs_ilInitialized == false)
	{
		ilInit();
		gs_ilInitialized = true;
		return 0;
	}
	else
	{
		return -1;
	}
}

int
ArnCleanupImageLibrary()
{
	if (gs_ilInitialized)
	{
		gs_ilInitialized = false;
		// TODO: No cleanup function for DevIL?
		return 0;
	}
	else
	{
		return -1;
	}
}

void
ArnTextureGetRawDataFromimageFile( std::vector<unsigned char>& data, unsigned int* width, unsigned int* height, unsigned int* bpp, const char* fileName )
{
	assert(gs_ilInitialized);
	assert(data.size() == 0);
	ILuint handle;
	ilGenImages(1, &handle);
	ilBindImage(handle);
	ILboolean result = ilLoadImage(fileName);
	if (result == IL_FALSE)
	{
		fprintf(stderr, " *** Texture file is not loaded correctly: %s\n", fileName);
		data.resize(0);
		*width = 0;
		*height = 0;
		return;
	}
	*width = ilGetInteger(IL_IMAGE_WIDTH);
	*height = ilGetInteger(IL_IMAGE_HEIGHT);
	ILint fmt = ilGetInteger(IL_IMAGE_FORMAT);
	ILint type = ilGetInteger(IL_IMAGE_TYPE);
	assert(type == IL_UNSIGNED_BYTE);
	switch (fmt)
	{
	case IL_RGB:
		*bpp = 3;
		break;
	case IL_RGBA:
		*bpp = 4;
		break;
	default:
		ARN_THROW_UNEXPECTED_CASE_ERROR
		break;
	}
	data.resize( (*width) * (*height) * (*bpp) );
	ilCopyPixels(0, 0, 0, *width, *height, 1, fmt, type, &data[0]);
	ilDeleteImages(1, &handle);
}

ArnTexture*
ArnCreateTextureFromArray( const unsigned char* data, unsigned int width, unsigned int height, unsigned int bpp, bool wrap )
{
	ArnTexture* ret = ArnTexture::createFrom(data, width, height, bpp, wrap);
	ret->init();
	return ret;
}
