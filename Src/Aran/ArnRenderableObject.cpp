#include "AranPCH.h"
#include "ArnRenderableObject.h"

ArnRenderableObject::ArnRenderableObject(void)
: ArnObject(NDT_RT_RENDERABLEOBJECT)
, m_bInitialized(false)
{
}

ArnRenderableObject::~ArnRenderableObject(void)
{
}
