#include "StdAfx.h"
#include "ArnBone.h"
#include "ArnFile.h"

ArnBone::ArnBone(void)
: ArnNode(NDT_RT_BONE)
{
}

ArnBone::~ArnBone(void)
{
}

ArnNode* ArnBone::createFrom( const NodeBase* nodeBase )
{
	return 0;
}

void ArnBone::buildFrom( const NodeBone1* nb )
{

}