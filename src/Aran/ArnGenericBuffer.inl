void ArnGenericBuffer::setCount(unsigned int count)
{
	m_count = count;
}

void ArnGenericBuffer::setUnitSize(unsigned int unitSize)
{
	m_unitSize = unitSize;
}

void ArnGenericBuffer::setData(unsigned int size, const void* data) // Data will be deep copied.
{
	if (!m_data || (m_count * m_unitSize) != size)
	{
		fprintf(stderr, "ArnGenericBuffer setData error!\n");
		abort();
	}
	else
	{
		memcpy(m_data, data, size);
	}
}

void ArnGenericBuffer::allocateData( unsigned int size )
{
	if (m_data)
	{
		fprintf(stderr, "ArnGenericBuffer already has data!\n");
		abort();
	}
	else
	{
		m_data = new char[size];
	}
}

size_t ArnGenericBuffer::getDataSize() const
{
	return m_unitSize * m_count;
}

void* ArnGenericBuffer::getData() const
{
	return m_data;
}

unsigned int ArnGenericBuffer::getCount() const
{
	return m_count;
}
