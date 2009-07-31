#include "AranPCH.h"
#include "ArnTexture.h"
#include "VideoMan.h"

#include "IL/il.h"

ArnTexture::ArnTexture(const char* texFileName)
: ArnObject(NDT_RT_TEXTURE)
, m_fileName(texFileName)
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
