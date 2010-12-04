#include "AranPCH.h"
#include "ArnLight.h"

ArnLight::ArnLight()
: ArnXformable(NDT_RT_LIGHT)
{
}

ArnLight::~ArnLight(void)
{
}

ArnLight* ArnLight::createFrom( const NodeBase* nodeBase )
{
	ArnLight* node = new ArnLight();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_LIGHT1:
			node->buildFrom(static_cast<const NodeLight1*>(nodeBase));
			break;
		case NDT_LIGHT2:
			node->buildFrom(static_cast<const NodeLight2*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
}

void ArnLight::buildFrom( const NodeLight1* nl )
{	
	m_d3dLight = *nl->m_light;
}

void ArnLight::buildFrom( const NodeLight2* nl )
{
	setParentName(nl->m_parentName);
	setLocalXform(*nl->m_localXform);
	m_d3dLight = *nl->m_light;
	setIpoName(nl->m_ipoName);
}

void ArnLight::interconnect( ArnNode* sceneRoot )
{
	setIpo(getIpoName());
	configureAnimCtrl();

	ArnNode::interconnect(sceneRoot);
}

ArnLight* ArnLight::createPointLight( const ArnVec3& pos, const ArnColorValue4f& color, const ArnVec3& att )
{
	ArnLight* ret = new ArnLight();
	memset(&ret->m_d3dLight, 0, sizeof(ArnLightData));
	ret->m_d3dLight.Ambient = color;
	ret->m_d3dLight.Diffuse = color;
	ret->m_d3dLight.Specular = color;
	ret->m_d3dLight.Position = pos;
	ret->m_d3dLight.Type = ARNLIGHT_POINT;
	ret->m_d3dLight.Attenuation0 = att.x; // const attenuation
	ret->m_d3dLight.Attenuation1 = att.y; // linear attenuation
	ret->m_d3dLight.Attenuation2 = att.z; // quadratic attenuation
	return ret;
}
