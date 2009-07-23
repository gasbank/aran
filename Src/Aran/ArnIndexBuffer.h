#pragma once

#include "ArnGenericBuffer.h"

class ArnIndexBuffer : public ArnGenericBuffer
{
public:
	static ArnIndexBuffer*				createFromArray(unsigned int count, unsigned int unitSize, const void* array);
										~ArnIndexBuffer(void);

private:
};
