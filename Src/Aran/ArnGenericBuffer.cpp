#include "AranPCH.h"
#include "ArnGenericBuffer.h"

ArnGenericBuffer::ArnGenericBuffer(void)
: m_data(0)
{
}

ArnGenericBuffer::~ArnGenericBuffer(void)
{
	delete [] m_data;
}

