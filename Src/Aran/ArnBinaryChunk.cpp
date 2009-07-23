#include "AranPCH.h"
#include "ArnBinaryChunk.h"
#include "ArnXmlString.h"

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
: m_data(0)
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

typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

template <typename T> void parseFromToken(char* target, int& dataOffset, Tokenizer::const_iterator& it, const int doCount, T func(const char*))
{
	assert(doCount >= 1);
	for (int i = 0; i < doCount; ++i)
	{
		*(T*)(target + i*sizeof(T)) = func((*it).c_str());
		++it;
		dataOffset += sizeof(T);
	}
}

inline float atof2(const char* c) { return (float)atof(c); }

ArnBinaryChunk* ArnBinaryChunk::createFrom(DOMElement* elm, char* binaryChunkBasePtr)
{
	ArnBinaryChunk* ret = new ArnBinaryChunk();
	DOMElement* templ = dynamic_cast<DOMElement*>(elm->getElementsByTagName(GetArnXmlString().TAG_template)->item(0));
	assert(templ);
	DOMNodeList* templChildren = templ->getElementsByTagName(GetArnXmlString().TAG_field);
	const XMLSize_t childCount = templChildren->getLength();
	assert(childCount);
	for (XMLSize_t xx = 0; xx < childCount; ++xx)
	{
		DOMElement* childElm = dynamic_cast<DOMElement*>(templChildren->item(xx));
		ret->addField(	ScopedString(childElm->getAttribute(GetArnXmlString().ATTR_type)).c_str()
						, ScopedString(childElm->getAttribute(GetArnXmlString().ATTR_usage)).c_str());
	}

	if (XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_place), GetArnXmlString().VAL_xml))
	{
		DOMElement* arraydata = dynamic_cast<DOMElement*>(elm->getElementsByTagName(GetArnXmlString().TAG_arraydata)->item(0));
		DOMNodeList* arraydataChildren = arraydata->getElementsByTagName(GetArnXmlString().TAG_data);
		ret->m_recordCount = arraydataChildren->getLength();
		assert(ret->m_recordCount >= 0); // There can be no record in the chunk.
		ret->m_data = new char[ ret->m_recordCount * ret->m_recordSize ];
		ret->m_deallocateData = true;
		int dataOffset = 0;
		for (XMLSize_t xx = 0; xx < (XMLSize_t)ret->m_recordCount; ++xx)
		{
			DOMElement* data = dynamic_cast<DOMElement*>(arraydataChildren->item(xx));
			ScopedString attrStr = ScopedString(data->getAttribute(GetArnXmlString().ATTR_value));
			//std::cout << "Original string: " << attrStr.c_str() << std::endl;
		    boost::char_separator<char> sep(";");
			std::string attrString(attrStr.c_str());
			Tokenizer tok(attrString, sep);
			Tokenizer::const_iterator it = tok.begin();
			/*
			for (; it != tok.end(); ++it)
			{
				std::cout << "<" << *it << ">" << std::endl;
			}
			*/

			foreach(const Field& field, ret->m_recordDef)
			{
				switch (field.type)
				{
				case ACFT_FLOAT:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 1, atof2);
					break;
				case ACFT_FLOAT2:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 2, atof2);
					break;
				case ACFT_FLOAT3:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 3, atof2);
					break;
				case ACFT_FLOAT8:
					parseFromToken<float>(ret->m_data + dataOffset, dataOffset, it, 8, atof2);
					break;
				case ACFT_INT:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 1, atoi);
					break;
				case ACFT_INT3:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 3, atoi);
					break;
				case ACFT_INT4:
					parseFromToken<int>(ret->m_data + dataOffset, dataOffset, it, 4, atoi);
					break;
				default:
					assert(!"Should not reach here!");
					break;
				}
			}
			assert(dataOffset % ret->m_recordSize == 0);
		}
		assert(dataOffset == ret->m_recordCount * ret->m_recordSize);
	}
	else if (XMLString::equals(elm->getAttribute(GetArnXmlString().ATTR_place), GetArnXmlString().VAL_bin))
	{
		const int startOffset = atoi(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_startoffset)).c_str());
		const int endOffset = atoi(ScopedString(elm->getAttribute(GetArnXmlString().ATTR_endoffset)).c_str());
		const int dataSize = endOffset - startOffset;
		const int recordCount = dataSize / ret->m_recordSize;
		assert(dataSize % ret->m_recordSize == 0);
		if (recordCount)
			ret->m_data = binaryChunkBasePtr + startOffset;
		else
			ret->m_data = 0;
		ret->m_deallocateData = false;
		ret->m_recordCount = recordCount;
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}

	return ret;
}

ArnBinaryChunk* ArnBinaryChunk::createFrom(const char* fileName)
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

void ArnBinaryChunk::addField(const char* type, const char* usage)
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

void ArnBinaryChunk::copyFieldArray(void* target, int targetSize, const char* usage) const
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

void ArnBinaryChunk::printFieldArray(const char* usage) const
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
			vec3->getFormatString(buf, 128);
			std::cout << buf << std::endl;
		}
	}
}

unsigned int ArnBinaryChunk::getRecordCount() const
{
	return m_recordCount;
}

unsigned int ArnBinaryChunk::getRecordSize() const
{
	return m_recordSize;
}

char* ArnBinaryChunk::getRawDataPtr()
{
	return m_data;
}

const char* ArnBinaryChunk::getRecordAt( int i ) const
{
	assert(i < m_recordCount);
	return m_data + m_recordSize * i;
}