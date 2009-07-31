#pragma once

class ARAN_API ArnGenericBuffer
{
public:
										~ArnGenericBuffer(void);

	inline size_t						getDataSize() const;
	inline void*						getData() const;
	inline unsigned int					getCount() const;

protected:
										ArnGenericBuffer(void);

	inline void							setCount(unsigned int count);
	inline void							setUnitSize(unsigned int unitSize);
	inline void							setData(unsigned int size, const void* data); // Data will be deep copied.
	inline void							allocateData(unsigned int size);

private:
	unsigned int						m_count;
	unsigned int						m_unitSize;
	char*								m_data;
};

#include "ArnGenericBuffer.inl"
