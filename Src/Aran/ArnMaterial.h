#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMaterial1;
struct NodeMaterial2;
class ArnTexture;

class ArnMaterial : public ArnNode
{
public:
	virtual										~ArnMaterial(void);
	typedef std::vector<ArnTexture*>			 TextureList;

	static ArnMaterial*							createFrom(const NodeBase* nodeBase);
	static ArnMaterial*							createFrom(const DOMElement* elm);

	unsigned int								getMaterialCount() const { return m_materialCount; }
	const ArnMaterialData&						getD3DMaterialData() const { return m_data.m_d3dMaterial; }
	unsigned int								getTexImgCount() const { return m_data.m_texImgList.size(); }
	const STRING&								getTexImgName(unsigned int idx) const { return m_data.m_texImgList[idx]; }
	void										loadTexture();
	void										attachTexture(ArnTexture* ARN_OWNERSHIP tex) { m_d3dTextureList.push_back(tex); }
	const ArnTexture*							getFirstTexture() const { return *m_d3dTextureList.begin(); }
	size_t										getTextureCount() const { return m_d3dTextureList.size(); }
	void										initRendererObject();
	bool										isShadeless() const { return m_bShadeless; }

	ArnTexture*									getD3DTexture(unsigned int idx) const { if (idx < m_d3dTextureList.size()) return m_d3dTextureList[idx]; else return 0; }

	// *** INTERNAL USE ONLY START ***
	virtual void								interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
												ArnMaterial();
	void										buildFrom(const NodeMaterial1* nm);
	void										buildFrom(const NodeMaterial2* nm);

	unsigned int								m_materialCount;
	MaterialData								m_data;
	const NodeMaterial2*						m_nodeMaterial; // Needed for late loading of texture images
	bool										m_bTextureLoaded;
	TextureList ARN_OWNERSHIP					m_d3dTextureList; // This class takes the ownership of ArnTexture* instances.
	bool										m_bShadeless; // Does not effected by lights
};


void SetSimpleColoredMaterial(ArnMaterialData* material, ArnColorValue4f color);
void ArnSetupMaterialGl(const ArnMaterial* mtrl);
