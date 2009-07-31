#include "AranPCH.h"
#include "ArnTexture.h"
#include "VideoMan.h"
//
// DevIL
//
#include "IL/il.h"

static bool gs_ilInitialized = false;

ArnTexture::ArnTexture(const char* texFileName)
: ArnObject(NDT_RT_TEXTURE)
, m_fileName(texFileName)
{
}

ArnTexture::~ArnTexture(void)
{
}

ArnTexture* ArnTexture::createFrom(const char* texFileName)
{
	ArnTexture* ret = new ArnTexture(texFileName);
	// TODO: Texture instantiation should not be done at this stage...
	//ArnCreateTextureFromFile(&GetVideoManager(), ret->m_fileName.c_str(), &ret);
	return ret;
}

//////////////////////////////////////////////////////////////////////////

HRESULT ArnCreateTextureFromFile( VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR

	//***LPDIRECT3DTEXTURE9 tex;
	//***D3DXCreateTextureFromFileA(pDevice->GetDev(), pSrcFile, &tex);
	//***(*ppTexture)->setDxTexture(tex);
	//***return S_OK;
}

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

void ArnInitializeImageLibrary()
{
	ilInit();
	gs_ilInitialized = true;
}

void ArnCleanupImageLibrary()
{
	assert(gs_ilInitialized);
	//
	// Insert cleanup code here
	//
	gs_ilInitialized = false;
}

void ArnTextureGetRawDataFromimageFile( unsigned char** data, int* width, int* height, const char* fileName )
{
	assert(gs_ilInitialized);
	ILuint handle;
	ilGenImages(1, &handle);
	ilBindImage(handle);
	ILboolean result = ilLoadImage(fileName);
	if (result == IL_FALSE)
	{
		fprintf(stderr, " *** Texture file is not loaded correctly: %s\n", fileName);
		*data = 0;
		*width = -1;
		*height = -1;
		return;
	}
	*width = ilGetInteger(IL_IMAGE_WIDTH);
	*height = ilGetInteger(IL_IMAGE_HEIGHT);
	*data = (unsigned char*)malloc( (*width) * (*height) * 3 );
	ilCopyPixels(0, 0, 0, *width, *height, 1, IL_RGB, IL_UNSIGNED_BYTE, *data);
	ilDeleteImages(1, &handle);
}
