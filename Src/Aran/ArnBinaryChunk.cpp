#include "AranPCH.h"
#include "ArnBinaryChunk.h"

/*
using namespace std;
using namespace boost;
using namespace boost::lambda;

void func(function<int(int, int)> f, int i, int j)
{
    cout << f(i, j) << endl;
}

void test()
{
    func(_1 + _2, 1, 2);
    func(_1 - _2, 1, 2);
}
*/

ArnBinaryChunk::ArnBinaryChunk()
: m_recordDef()
, m_data()
, m_deallocateData(0)
, m_recordCount(0)
, m_recordSize(0)
{
}

ArnBinaryChunk::~ArnBinaryChunk()
{
	if (m_deallocateData)
		delete m_data;
}

ArnBinaryChunk*
ArnBinaryChunk::createFrom(const char* fileName)
{
	FILE* f = fopen(fileName, "rb");
	ArnBinaryChunk* ret = 0;
	if (f)
	{
		fseek(f, 0, SEEK_END);
		size_t fileSize = ftell(f);
		if (fileSize)
		{
			ret = new ArnBinaryChunk();
			ret->m_data = new char[fileSize];
			ret->m_deallocateData = true;
			fseek(f, 0, SEEK_SET);
			fread(ret->m_data, fileSize, 1, f);
			fclose(f);
			ret->m_recordSize = fileSize;
			ret->m_recordCount = 1;
		}
	}
	else
	{
		throw MyError(MEE_FILE_ACCESS_ERROR);
	}
	return ret;
}

void
ArnBinaryChunk::addField(const char* type, const char* usage)
{
	ArnChunkFieldType acft = ACFT_UNKNOWN;
	if (strcmp(type, "float") == 0)
		acft = ACFT_FLOAT;
	else if (strcmp(type, "float2") == 0)
		acft = ACFT_FLOAT2;
	else if (strcmp(type, "float3") == 0)
		acft = ACFT_FLOAT3;
	else if (strcmp(type, "float8") == 0)
		acft = ACFT_FLOAT8;
	else if (strcmp(type, "int") == 0)
		acft = ACFT_INT;
	else if (strcmp(type, "int3") == 0)
		acft = ACFT_INT3;
	else if (strcmp(type, "int4") == 0)
		acft = ACFT_INT4;
	else
		acft = ACFT_UNKNOWN;
	assert(acft != ACFT_UNKNOWN);
	m_recordDef.push_back(Field(acft, usage, m_recordSize));
	m_recordSize += ArnChunkFieldTypeSize[acft];
}

void
ArnBinaryChunk::copyFieldArray(void* target, int targetSize, const char* usage) const
{
	const Field* field = 0;
	foreach(const Field& f, m_recordDef)
	{
		if (f.usage.compare(usage) == 0)
		{
			field = &f;
			break;
		}
	}
	assert(field);
	const int fieldSize = ArnChunkFieldTypeSize[field->type];
	assert(targetSize == fieldSize * m_recordCount); // Clients should provide the exact 'targetSize' for 'target'.
	for (int i = 0; i < m_recordCount; ++i)
	{
		memcpy((void*)((char*)target + fieldSize * i), m_data + m_recordSize * i + field->offset, fieldSize);
	}
}

void
ArnBinaryChunk::printFieldArray(const char* usage) const
{
	const Field* field = 0;
	foreach(const Field& f, m_recordDef)
	{
		if (f.usage.compare(usage) == 0)
		{
			field = &f;
			break;
		}
	}
	assert(field);
	for (int i = 0; i < m_recordCount; ++i)
	{
		if (field->type == ACFT_FLOAT3)
		{
			const ArnVec3* vec3 = (const ArnVec3*)(m_data + m_recordSize * i + field->offset);
			char buf[128];
			ArnVec3GetFormatString(buf, 128, *vec3);
			std::cout << buf << std::endl;
		}
	}
}

unsigned int
ArnBinaryChunk::getRecordCount() const
{
	return m_recordCount;
}

unsigned int
ArnBinaryChunk::getRecordSize() const
{
	return m_recordSize;
}

char*
ArnBinaryChunk::getRawDataPtr()
{
	return m_data;
}

const char*
ArnBinaryChunk::getRecordAt( int i ) const
{
	assert(i < m_recordCount);
	return m_data + m_recordSize * i;
}
