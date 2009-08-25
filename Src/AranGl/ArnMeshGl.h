#pragma once

#include "ArnRenderableObject.h"

class ArnMeshGl : public ArnRenderableObject
{
public:
											~ArnMeshGl(void);
	static ArnMeshGl*						createFrom(const ArnMesh* mesh);
	virtual int								render(bool bIncludeShadeless) const;
	virtual void							cleanup();
private:
											ArnMeshGl(void);
	bool									initRendererObjectVbIb();
	bool									initRendererObjectXml();
	void									renderVbIb() const;
	void									renderXml(bool bIncludeShadeless) const;
	GLuint									m_vboId;
	std::vector<GLuint>						m_vboIds;
	GLuint									m_vboUv;
	const ArnMesh*							m_target; // Rendering object which has all the data
};
