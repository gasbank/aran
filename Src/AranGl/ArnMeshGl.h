#pragma once

class ArnMeshGl
{
public:
	ArnMeshGl(void);
	~ArnMeshGl(void);

private:
	virtual void							renderVbIb();
	virtual void							renderXml();
	GLuint									m_vboId;
	std::vector<GLuint>						m_vboIds;
	GLuint									m_vboUv;
};
