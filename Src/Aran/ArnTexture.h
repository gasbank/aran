#pragma once

#include "ArnObject.h"

class ArnRenderableObject;

// Aran library compartment for LPD3DTEXTURE9
class ARAN_API ArnTexture : public ArnObject
{
public:
											~ArnTexture(void);
	static ArnTexture*						createFrom(const char* texFileName);
	virtual const char*						getName() const { return m_name.c_str(); }
	const char*								getFileName() const { return m_fileName.c_str(); }
	void									setRenderableObject(boost::shared_ptr<ArnRenderableObject> ptr) { m_renderableObject = ptr; }
	boost::shared_ptr<ArnRenderableObject>	getRenderableObject() const { return m_renderableObject; }
private:
											ArnTexture(const char* texFileName);
	std::string								m_name;
	std::string								m_fileName;
	boost::shared_ptr<ArnRenderableObject>	m_renderableObject;
};

class VideoMan;

ARAN_API void ArnInitializeImageLibrary();
ARAN_API void ArnCleanupImageLibrary();
ARAN_API void ArnTextureGetRawDataFromimageFile( unsigned char** data, int* width, int* height, const char* fileName );

HRESULT ArnCreateTextureFromFile(VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture);
void ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName);
