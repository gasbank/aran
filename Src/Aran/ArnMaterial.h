#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeMaterial1;
struct NodeMaterial2;
class ArnTexture;

class ARAN_API ArnMaterial : public ArnNode
{
public:
	virtual										~ArnMaterial(void);
	typedef std::vector<ArnTexture*>			TextureList;
	static ArnMaterial*							createFrom(const NodeBase* nodeBase);
	static ArnMaterial*							createFrom(const DOMElement* elm);
	void										attachTexture(ArnTexture* ARN_OWNERSHIP tex);
	const ArnTexture*							getFirstTexture() const;
	ArnTexture*									getD3DTexture(unsigned int idx) const;
	size_t										getTextureCount() const { return getChildren().size(); }
	void										initRendererObject();
	bool										isShadeless() const { return m_bShadeless; }
	
	/*! @name Deprecated methods
	These methods are not recommended after ARN30 and will be removed in the next version.
	*/
	//@{
	void										loadTexture();
	unsigned int								getMaterialCount() const { return m_materialCount; }
	const ArnMaterialData&						getD3DMaterialData() const { return m_data.m_d3dMaterial; }
	unsigned int								getTexImgCount() const { return m_data.m_texImgList.size(); }
	const std::string&							getTexImgName(unsigned int idx) const { return m_data.m_texImgList[idx]; }
	//@}

	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	virtual void								interconnect(ArnNode* sceneRoot);
	//@}
private:
												ArnMaterial();
	void										buildFrom(const NodeMaterial1* nm);
	void										buildFrom(const NodeMaterial2* nm);
	bool										m_bShadeless; // Does not effected by lights

	// TODO: The following variables should be removed.
	MaterialData								m_data;
	const NodeMaterial2*						m_nodeMaterial; // Needed for late loading of texture images
	bool										m_bTextureLoaded;
	unsigned int								m_materialCount;
	TextureList ARN_OWNERSHIP					m_d3dTextureList; // This class takes the ownership of ArnTexture* instances.
};

void SetSimpleColoredMaterial(ArnMaterialData* material, ArnColorValue4f color);
