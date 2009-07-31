#pragma once
class VideoMan;

#include "ArnObject.h"

// Aran library compartment for LPD3DTEXTURE9
class ArnTexture : public ArnObject
{
public:
											~ArnTexture(void);
	static ArnTexture*						createFrom(const char* texFileName);
	virtual bool							initialize() = 0;
	virtual void							Release() = 0;
	const char*								getFileName() const { return m_fileName.c_str(); }
private:
											ArnTexture(const char* texFileName);
	std::string								m_fileName;
	bool									m_inited;
};

HRESULT ArnCreateTextureFromFile(VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture);
void ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName);

