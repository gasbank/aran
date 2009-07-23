#pragma once

#include "ArnGenericBuffer.h"

class ArnVertexBuffer : public ArnGenericBuffer
{
public:
	static ArnVertexBuffer*				createFromArray(unsigned int count, unsigned int unitSize, const void* array);
										~ArnVertexBuffer(void);

private:
};
