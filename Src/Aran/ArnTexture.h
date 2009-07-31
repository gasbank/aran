#pragma once

#include "ArnNode.h"

class ArnRenderableObject;

// Aran library compartment for LPD3DTEXTURE9
class ARAN_API ArnTexture : public ArnNode
{
public:
											~ArnTexture(void);
	static ArnTexture*						createFrom(const char* texFileName);
	virtual const char*						getName() const { return m_name.c_str(); }
	const char*								getFileName() const { return m_fileName.c_str(); }
	
	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	virtual void							interconnect(ArnNode* sceneRoot);
	//@}
private:
											ArnTexture(const char* texFileName);
	std::string								m_name;
	std::string								m_fileName;
};

class VideoMan;

ARAN_API void ArnInitializeImageLibrary();
ARAN_API void ArnCleanupImageLibrary();
ARAN_API void ArnTextureGetRawDataFromimageFile( unsigned char** data, int* width, int* height, const char* fileName );

HRESULT ArnCreateTextureFromFile(VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture);
void ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName);
