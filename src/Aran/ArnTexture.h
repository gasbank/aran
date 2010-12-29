/*! @file ArnTexture.h
 *  @author Geoyeob Kim
 *  @date 2009
 */
#pragma once

#include "ArnNode.h"

class ArnRenderableObject;

enum ArnColorFormat
{
	ACF_UNKNOWN,
	ACF_RGB,
	ACF_RGBA,
	ACF_BGR,
	ACF_BGRA
};

TYPEDEF_SHARED_PTR(ArnTexture)

// Aran library compartment for LPD3DTEXTURE9
class ARAN_API ArnTexture : public ArnNode
{
public:
											~ArnTexture(void);
	static ArnTexture*						createFrom(const char* texFileName);
	static ArnTexture*						createFrom(const unsigned char* data, unsigned int width, unsigned int height, ArnColorFormat format, bool wrap); // Create from in-memory raw image data
	void									init();
	bool									isInitialized() const { return m_bInitialized; }
	const char*								getFileName() const { return m_fileName.c_str(); }
	const std::vector<unsigned char>&		getRawData() const { return m_rawData; }
	unsigned int							getWidth() const { return m_width; }
	unsigned int							getHeight() const { return m_height; }
	ArnColorFormat							getFormat() const { return m_format; }
	unsigned int							getBpp() const;
	bool									isWrap() const { return m_bWrap; }
	/*!
	 * @internalonly
	 */
	//@{
	virtual void							interconnect(ArnNode* sceneRoot);
	//@}
private:
											ArnTexture();
	bool									m_bInitialized;
	std::string								m_fileName;
	std::vector<unsigned char>				m_rawData;
	unsigned int							m_width;
	unsigned int							m_height;
	ArnColorFormat							m_format;
	bool									m_bWrap; // Wrap the texture image along x and y axes.
};

class VideoMan;

/*!
 * @brief ?¥Î?ÏßÄ ?ºÏù¥Î∏åÎü¨Î¶?DevIL) Ï¥àÍ∏∞??
 * @return ?±Í≥µ ??0, ?§Ìå®(??Ï¥àÍ∏∞???¨Ìï®) ???åÏàò
 */
ARAN_API int			ArnInitializeImageLibrary();
/*!
 * @brief ?¥Î?ÏßÄ ?ºÏù¥Î∏åÎü¨Î¶?DevIL) ?¥Ï†ú
 * @return ?±Í≥µ ??0, ?§Ìå®(ÎØ?Ï¥àÍ∏∞?????∏Ï∂ú ?¨Ìï®) ???åÏàò
 */
ARAN_API int			ArnCleanupImageLibrary();

ARAN_API ArnTexture*	ArnCreateTextureFromArray( const unsigned char* data, unsigned int width, unsigned int height, ArnColorFormat format, bool wrap );
void					ArnTextureGetRawDataFromimageFile( std::vector<unsigned char>& data, unsigned int* width, unsigned int* height, ArnColorFormat* bpp, const char* fileName );
HRESULT					ArnCreateTextureFromFile(VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture);
void					ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName);
inline unsigned int		ArnGetBppFromFormat(ArnColorFormat format)
{
	if (format == ACF_RGB || format == ACF_BGR)
		return 3;
	else if (format == ACF_RGBA || format == ACF_BGRA)
		return 4;
	else return 0;
}

ARAN_API unsigned char* readBMP2(const char* fileName, int& width, int& height, bool& rgbOrBgr);