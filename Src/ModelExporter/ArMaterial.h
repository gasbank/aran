#pragma once
#include "arobject.h"


class ArMaterial :
	public ArObject
{
public:
	ArMaterial(void);
	virtual ~ArMaterial(void);

	const std::string& getTextureFileName() const { return m_textureFileName; }
	void setTextureFileName(const std::string& val) { m_textureFileName = val; }
	ArColorValue getDiffuse() const { return m_diffuse; }
	void setDiffuse(const ArColorValue& val) { m_diffuse = val; }
	ArColorValue getAmbient() const { return m_ambient; }
	void setAmbient(const ArColorValue& val) { m_ambient = val; }
	ArColorValue getEmissive() const { return m_emissive; }
	void setEmissive(const ArColorValue& val) { m_emissive = val; }
	ArColorValue getSpecular() const { return m_specular; }
	void setSpecular(const ArColorValue& val) { m_specular = val; }

private:
	std::string m_textureFileName;
	
	ArColorValue m_diffuse, m_ambient, m_specular, m_emissive;
	

};
