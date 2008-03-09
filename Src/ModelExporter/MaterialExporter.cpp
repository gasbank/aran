#include "stdafx.h"
#include "MaterialExporter.h"
#include "ArMaterial.h"

MaterialExporter::MaterialExporter(IGameMaterial* material, ArMaterial*& arMaterial)
{
	m_material = material;
	m_arMaterial = arMaterial;
}

MaterialExporter::~MaterialExporter(void)
{
}

void MaterialExporter::make()
{
	m_arMaterial->setName(m_material->GetMaterialName());

	int textureMapCount = m_material->GetNumberOfTextureMaps();
	if (textureMapCount)
	{
		IGameTextureMap* igtm = m_material->GetIGameTextureMap(0);
		m_arMaterial->setTextureFileName(igtm->GetBitmapFileName());
		if (textureMapCount > 0)
		{
			DebugPrint(_T("  ! WARNING: Only the first texture map will be exported; %d exist in the Max format."), textureMapCount);
		}
	}

	Point3 p3;
	if (m_material->GetDiffuseData())
	{
		m_material->GetDiffuseData()->GetPropertyValue(p3);
		m_arMaterial->setDiffuse(ArColorValue(p3.x, p3.y, p3.z, 1.0f));
	}
	if ( m_material->GetAmbientData())
	{
		m_material->GetAmbientData()->GetPropertyValue(p3);
		m_arMaterial->setAmbient(ArColorValue(p3.x, p3.y, p3.z, 1.0f));
	}
	if (m_material->GetSpecularData())
	{
		m_material->GetSpecularData()->GetPropertyValue(p3);
		m_arMaterial->setSpecular(ArColorValue(p3.x, p3.y, p3.z, 1.0f));
	}
	if ( m_material->GetEmissiveData())
	{
		m_material->GetEmissiveData()->GetPropertyValue(p3);
		m_arMaterial->setEmissive(ArColorValue(p3.x, p3.y, p3.z, 1.0f));
	}
}