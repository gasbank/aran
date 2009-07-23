#include "AranPCH.h"
#include "ArnObject.h"

unsigned int ArnObject::ms_objectId = 0;

ArnObject::ArnObject(NODE_DATA_TYPE type)
: m_type(type)
, m_objectId(++ms_objectId) // TODO: Thread safety?
{
}

ArnObject::~ArnObject(void)
{
}
