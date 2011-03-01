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
ArnTexture::createFrom(const unsigned char* data, unsigned int width, unsigned int height, ArnColorFormat format, bool wrap)
{
	assert(data && width && height && (0 <= format && format <= 3));
	ArnTexture* ret = new ArnTexture();
	// m_rawData has a deep copy of the texture image data.
	ret->m_rawData.resize(width * height * ArnGetBppFromFormat(format));
	memcpy(&ret->m_rawData[0], data, width * height * ArnGetBppFromFormat(format));
	ret->m_width = width;
	ret->m_height = height;
	ret->m_format = format;
	ret->m_bWrap = wrap;
	return ret;
}

void
ArnTexture::interconnect( ArnNode* sceneRoot )
{
}

void split_path(std::string &dir, std::string &filename, const std::string& path)
{
  const size_t found = path.find_last_of("/\\");
  dir = path.substr(0,found);
  filename = path.substr(found+1);
}

void
ArnTexture::init()
{
	assert(m_bInitialized == false);
	if (m_fileName.size() && m_rawData.size() == 0) // The path of a texture image is provided.
	{
		ArnTextureGetRawDataFromimageFile(m_rawData, &m_width, &m_height, &m_format, m_fileName.c_str());
    if (m_format == ACF_UNKNOWN) {
      // File not found? Try fallback image name
      std::string dir, filename_only;
      split_path(dir, filename_only, m_fileName);
      printf("     Try fallback name : %s\n", filename_only.c_str());
      ArnTextureGetRawDataFromimageFile(m_rawData, &m_width, &m_height, &m_format, filename_only.c_str());
      if (m_format != ACF_UNKNOWN) {
        printf("     Fallback texture image loading success.\n");
      }
    }
	}
	else if (m_fileName.size() == 0 && m_rawData.size() && m_width && m_height && m_format) // In-memory pointer to raw image data is provided.
	{
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	m_bInitialized = true;
}

unsigned int ArnTexture::getBpp() const
{
	return ArnGetBppFromFormat(m_format);
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
#ifdef USE_DEVIL_AS_IMAGELIB
		ilInit();
#endif
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

template <class T>
T nearestpower2(T v)
{
	int k;
	if (v == 0)
		return 1;
	for (k = sizeof(T) * 8 - 1; ((static_cast<T>(1U) << k) & v) == 0; k--);
	if (((static_cast<T>(1U) << (k - 1)) & v) == 0)
		return static_cast<T>(1U) << k;
	return static_cast<T>(1U) << (k + 1);
}

#ifdef USE_DEVIL_AS_IMAGELIB
void
ArnTextureGetRawDataFromimageFile( std::vector<unsigned char>& data, unsigned int* width, unsigned int* height, ArnColorFormat* format, const char* fileName )
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
		*format = ACF_UNKNOWN;
		return;
	}
	*width = ilGetInteger(IL_IMAGE_WIDTH);
	*height = ilGetInteger(IL_IMAGE_HEIGHT);

	if (*width != nearestpower2(*width) || *height != nearestpower2(*height))
	{
		fprintf(stderr, " *** WARNING: Image dimension should be a power of two. (%d x %d) given.\n", *width, *height);
		/*
		data.resize(0);
		*width = 0;
		*height = 0;
		*format = ACF_UNKNOWN;
		return;
		*/
	}

	ILint fmt = ilGetInteger(IL_IMAGE_FORMAT);
	ILint type = ilGetInteger(IL_IMAGE_TYPE);
	assert(type == IL_UNSIGNED_BYTE);
	switch (fmt)
	{
	case IL_RGB: *format = ACF_RGB; break;
	case IL_BGR: *format = ACF_BGR; break;
	case IL_RGBA: *format = ACF_RGBA; break;
	case IL_BGRA: *format = ACF_BGRA; break;
	default: ARN_THROW_UNEXPECTED_CASE_ERROR break;
	}
	data.resize( (*width) * (*height) * ArnGetBppFromFormat(*format) );
	ilCopyPixels(0, 0, 0, *width, *height, 1, fmt, type, &data[0]);
	ilDeleteImages(1, &handle);
}
#else // #ifdef USE_DEVIL_AS_IMAGELIB
void
ArnTextureGetRawDataFromimageFile( std::vector<unsigned char>& data, unsigned int* width, unsigned int* height, ArnColorFormat* format, const char* fileName )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
#endif // #ifdef USE_DEVIL_AS_IMAGELIB



ArnTexture*
ArnCreateTextureFromArray( const unsigned char* data, unsigned int width, unsigned int height, ArnColorFormat format, bool wrap )
{
	ArnTexture* ret = ArnTexture::createFrom(data, width, height, format, wrap);
	ret->init();
	return ret;
}

#ifdef USE_DEVIL_AS_IMAGELIB
// Bitmap data returned is (R,G,B) tuples in row-major order.
unsigned char* readBMP2(const char* fileName, int& width, int& height, bool& rgbOrBgr)
{
	ILuint handle;
	ilGenImages(1, &handle);
	ilBindImage(handle);
	ILboolean result = ilLoadImage(fileName);
	if (result == IL_FALSE)
	{
		fprintf(stderr, " *** Texture file is not loaded correctly: %s\n", fileName);
		width = 0;
		height = 0;
		rgbOrBgr = false;
		return 0;
	}
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	ILint fmt = ilGetInteger(IL_IMAGE_FORMAT);
	ILint type = ilGetInteger(IL_IMAGE_TYPE);
	assert(type == IL_UNSIGNED_BYTE);
	switch (fmt)
	{
	case IL_RGB: rgbOrBgr = false; break;
	case IL_BGR: rgbOrBgr = true; break;
	default: throw std::runtime_error("Unspecified BMP type."); break;
	}
	unsigned char* data = new unsigned char[width * height * 3];
	ilCopyPixels(0, 0, 0, width, height, 1, fmt, type, data);
	ilDeleteImages(1, &handle);
	return data;
}
#else // #ifdef USE_DEVIL_AS_IMAGELIB
unsigned char* readBMP2(const char* fileName, int& width, int& height, bool& rgbOrBgr)
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
#endif // #ifdef USE_DEVIL_AS_IMAGELIB