#include "AranPCH.h"
#include "ArnXformable.h"
#include "ArnIpo.h"

ArnXformable::ArnXformable(NODE_DATA_TYPE ndt)
: ArnNode(ndt)
{
	D3DXMatrixIdentity(&m_localXform);
}

ArnXformable::~ArnXformable(void)
{
}

void ArnXformable::setIpo( const STRING& ipoName )
{
	if (ipoName.length())
	{
		ArnNode* ipo = getSceneRoot()->getNodeByName(getIpoName());
		if (ipo && ipo->getType() == NDT_RT_IPO)
			setIpo(reinterpret_cast<ArnIpo*>(ipo));
		else
			throw MyError(MEE_RTTI_INCONSISTENCY);
	}
	else
	{
		setIpo(0);
	}
}

D3DXMATRIX ArnXformable::getFinalXform() const
{
	if (	(getParent()->getType() == NDT_RT_MESH)
		||	(getParent()->getType() == NDT_RT_CAMERA)
		||	(getParent()->getType() == NDT_RT_LIGHT) )
	{
		return static_cast<ArnXformable*>(getParent())->getFinalXform() * getLocalXform();
	}
	else
	{
		return getLocalXform();
	}
}