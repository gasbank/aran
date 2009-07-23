#include "AranPCH.h"
#include "ArnIndexBuffer.h"

ArnIndexBuffer::~ArnIndexBuffer(void)
{
}

ArnIndexBuffer* ArnIndexBuffer::createFromArray( unsigned int count, unsigned int unitSize, const void* array )
{
	ArnIndexBuffer* ret = new ArnIndexBuffer();
	ret->setCount(count);
	ret->setUnitSize(unitSize);
	ret->allocateData(count * unitSize);
	ret->setData(count * unitSize, array);
	return ret;
}
