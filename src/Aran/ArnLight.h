#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeLight1;
struct NodeLight2;

class ARAN_API ArnLight : public ArnXformable
{
public:
									~ArnLight(void);

	static ArnLight*				createPointLight(const ArnVec3& pos, const ArnColorValue4f& color, const ArnVec3& att);
	static ArnLight*				createFrom(const NodeBase* nodeBase);
	static ArnLight*				createFrom(const TiXmlElement* elm);

	const ArnLightData&				getD3DLightData() const { return m_d3dLight; }
	void							setD3DLightDataPosition(const ArnVec3& v) { m_d3dLight.Position = v; }


	// *** INTERNAL USE ONLY START ***
	virtual void					interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
									ArnLight();

	void							buildFrom(const NodeLight1* nl);
	void							buildFrom(const NodeLight2* nl);

	ArnLightData					m_d3dLight;
};
