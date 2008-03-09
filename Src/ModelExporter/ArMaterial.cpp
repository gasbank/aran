#include "stdafx.h"
#include "ArMaterial.h"

ArMaterial::ArMaterial(void)
{
	m_textureFileName[0] = '\0';
	m_ambient = ArColorValue::RED;
	m_diffuse = ArColorValue::RED;
	m_specular = ArColorValue::RED;
	m_emissive = ArColorValue::RED;
}

ArMaterial::~ArMaterial(void)
{
}
