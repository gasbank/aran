#pragma once
class VideoMan;

// Aran library compartment for LPD3DTEXTURE9
class ArnTexture
{
public:
											~ArnTexture(void);

	static ArnTexture*						createFrom(const char* texFileName);

	bool									initGl();
	void									Release();
	GLuint									getTextureId() const { return m_textureId; }
	LPDIRECT3DTEXTURE9						getDxTexture() const { return m_d3d9Tex; }
	void									setDxTexture(LPDIRECT3DTEXTURE9 tex) { m_d3d9Tex = tex; }

private:
											ArnTexture(const char* texFileName);
	std::string								m_fileName;
	bool									m_inited;
	GLuint									m_textureId;
	LPDIRECT3DTEXTURE9						m_d3d9Tex;
};

HRESULT ArnCreateTextureFromFile(VideoMan* pDevice, const char* pSrcFile, ArnTexture** ppTexture);
void ArnLoadFromPpmFile(unsigned char** buff, int* width, int* height, const char* fileName);

