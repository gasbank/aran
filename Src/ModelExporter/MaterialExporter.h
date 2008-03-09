#pragma once

class ArMaterial;

class MaterialExporter
{
public:
	MaterialExporter(IGameMaterial* material, ArMaterial*& arMaterial);
	~MaterialExporter(void);

	void make();

private:
	IGameMaterial* m_material;
	ArMaterial* m_arMaterial;
};
