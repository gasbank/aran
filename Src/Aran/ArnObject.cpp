#include "AranPCH.h"
#include "ArnObject.h"

unsigned int ArnObject::ms_objectId = 0;
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
unsigned int ArnObject::ms_ctorCount = 0;
unsigned int ArnObject::ms_dtorCount = 0;
std::set<ArnObject*> ArnObject::ms_instances;
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

ArnObject::ArnObject(NODE_DATA_TYPE type)
: m_type(type)
, m_objectId(++ms_objectId) // TODO: Thread safety?
{
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	++ms_ctorCount;
	//printf("Ctor %d : 0x%08x\n", ms_ctorCount, m_type);
	assert(ms_instances.find(this) == ms_instances.end());
	ms_instances.insert(this);
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
}

ArnObject::~ArnObject(void)
{
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	++ms_dtorCount;
	//printf("Dtor %d : 0x%08x\n", ms_dtorCount, m_type);
	assert(ms_instances.find(this) != ms_instances.end());
	ms_instances.erase(this);
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
}

#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
void
ArnObject::printInstances()
{
	foreach (ArnObject* obj, ms_instances)
	{
		const char* name = strlen(obj->getName()) ? obj->getName() : "<Unnamed>";
		printf("[ArnObject 0x%p] TYPE: 0x%08x, NAME: %s\n", (void*)obj, obj->getType(), name);
	}
}

ArnObject*
ArnObject::getObjectById(unsigned int objId)
{
	foreach (ArnObject* obj, ms_instances)
	{
		if (obj->getObjectId() == objId)
			return obj;
	}
	return 0;
}
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
