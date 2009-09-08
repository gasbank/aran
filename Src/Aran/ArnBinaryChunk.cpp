#include "AranPCH.h"
#include "ArnBinaryChunk.h"

#include "zlib.h"

// Copied and modified from zpipe.c example of zlib
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

int inf(FILE *source, char *dest)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	unsigned destOffset = 0;
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;

			memcpy(dest + destOffset, out, have);
			destOffset += have;
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("zpipe: ", stderr);
	switch (ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}

//////////////////////////////////////////////////////////////////////////

ArnBinaryChunk::ArnBinaryChunk()
: ArnObject(NDT_RT_BINARYCHUNK)
, m_recordDef()
, m_data()
, m_deallocateData(true)
, m_recordCount(0)
, m_recordSize(0)
{
}

ArnBinaryChunk::~ArnBinaryChunk()
{
	if (m_deallocateData)
		delete [] m_data;
}

ArnBinaryChunk*
ArnBinaryChunk::createFrom(const char* fileName, bool zlibCompressed, unsigned int uncompressedSize)
{
	FILE* f = fopen(fileName, "rb");
	ArnBinaryChunk* ret = 0;
	if (f)
	{
		if (zlibCompressed == false)
		{
			fseek(f, 0, SEEK_END);
			size_t fileSize = ftell(f);
			assert(fileSize == uncompressedSize);
			if (fileSize)
			{
				ret = new ArnBinaryChunk();
				char* data = new char[fileSize]; // Since ret->m_data type is 'const char*', we use temporary variable named 'data'.
				ret->m_deallocateData = true;
				fseek(f, 0, SEEK_SET);
				fread(data, 1, fileSize, f);
				fclose(f);
				ret->m_data = data;
				ret->m_recordSize = fileSize;
				ret->m_recordCount = 1;
			}
		}
		else // zlib compressed binary file
		{
			ret = new ArnBinaryChunk();
			char* data = new char[uncompressedSize]; // Since ret->m_data type is 'const char*', we use temporary variable named 'data'.
			int zret;
			if ((zret = inf(f, data)) != Z_OK)
			{
				zerr(zret);
				delete [] data;
				throw MyError(MEE_FILE_ACCESS_ERROR);
			}
			ret->m_data = data;
			ret->m_deallocateData = true;
			ret->m_recordSize = uncompressedSize;
			ret->m_recordCount = 1;
		}
		fclose(f);
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

const char*
ArnBinaryChunk::getConstRawDataPtr() const
{
	return m_data;
}

const char*
ArnBinaryChunk::getRecordAt( int i ) const
{
	assert(i < m_recordCount);
	return m_data + m_recordSize * i;
}
