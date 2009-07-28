#ifndef ARNBINARYCHUNK_H
#define ARNBINARYCHUNK_H

enum ArnChunkFieldType
{
	ACFT_UNKNOWN = 0,	// Unknown type
	ACFT_FLOAT,			// e.g. bone weight
	ACFT_FLOAT2,		// e.g. UV coordinates
	ACFT_FLOAT3,		// e.g. vertex coordiates, normals
	ACFT_FLOAT8,		// e.g. 2D UV coordinates quadruple of quad face
	ACFT_INT,			// e.g. vertex/face indices
	ACFT_INT3,			// e.g. vertex indices of tri face
	ACFT_INT4			// e.g. vertex indices of quad face
};
static const int ArnChunkFieldTypeSize[] =
{
	-1,
	sizeof(float)	* 1,
	sizeof(float)	* 2,
	sizeof(float)	* 3,
	sizeof(float)	* 8,
	sizeof(int)		* 1,
	sizeof(int)		* 3,
	sizeof(int)		* 4
};

class ArnBinaryChunk
{
public:
										~ArnBinaryChunk();
	static ArnBinaryChunk*				createFrom(DOMElement* elm, char* binaryChunkBasePtr);
	static ArnBinaryChunk*				createFrom(const char* fileName, bool zlibCompressed, unsigned int uncompressedSize);
	void								copyFieldArray(void* target, int targetSize, const char* usage) const;
	unsigned int						getRecordCount() const;
	unsigned int						getRecordSize() const;
	char*								getRawDataPtr();
	const char*							getRecordAt(int i) const;
	void								printFieldArray(const char* usage) const; // Debug purpose...

private:
										ArnBinaryChunk();
	void								addField(const char* type, const char* usage);

	struct Field
	{
		Field(ArnChunkFieldType _type, const char* _usage, int _offset)
		: type(_type)
		, usage(_usage)
		, offset(_offset) {}
		~Field() {}
		ArnChunkFieldType	type;
		std::string			usage;
		int					offset; // offset in the record
	};
	std::vector<Field>					m_recordDef; // record definition
	char*								m_data;
	bool								m_deallocateData; // Deallocation of m_data should be done in dtor if this flag is true.
	int									m_recordCount;
	int									m_recordSize;
};

#endif // ARNBINARYCHUNK_H
