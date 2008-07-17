#include "AranPCH.h"
#include "ArnMovable.h"
#include "ArnIpo.h"

ArnMovable::ArnMovable(NODE_DATA_TYPE ndt)
: ArnNode(ndt)
{

}

ArnMovable::~ArnMovable(void)
{
}

void ArnMovable::setIpo( const STRING& ipoName )
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
