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
	virtual const char*						getName() const { return m_name.c_str(); }
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
	std::string								m_name;
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
 * @brief 이미지 라이브러리(DevIL) 초기화
 * @return 성공 시 0, 실패(재 초기화 포함) 시 음수
 */
ARAN_API int			ArnInitializeImageLibrary();
/*!
 * @brief 이미지 라이브러리(DevIL) 해제
 * @return 성공 시 0, 실패(미 초기화 후 호출 포함) 시 음수
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
