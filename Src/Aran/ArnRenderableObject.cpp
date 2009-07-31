#include "AranPCH.h"
#include "ArnRenderableObject.h"

ArnRenderableObject::ArnRenderableObject(void)
: ArnNode(NDT_RT_RENDERABLEOBJECT)
, m_bInitialized(false)
{
}

ArnRenderableObject::~ArnRenderableObject(void)
{
}

void ArnRenderableObject::interconnect( ArnNode* sceneRoot )
{

}