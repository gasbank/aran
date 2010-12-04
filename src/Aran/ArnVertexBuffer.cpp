#include "AranPCH.h"
#include "ArnVertexBuffer.h"

ArnVertexBuffer::~ArnVertexBuffer(void)
{
}

ArnVertexBuffer* ArnVertexBuffer::createFromArray( unsigned int count, unsigned int unitSize, const void* array )
{
	ArnVertexBuffer* ret = new ArnVertexBuffer();
	ret->setCount(count);
	ret->setUnitSize(unitSize);
	ret->allocateData(count * unitSize);
	ret->setData(count * unitSize, array);
	return ret;
}
