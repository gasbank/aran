#ifndef ARNXMLLOADER_H
#define ARNXMLLOADER_H

class ArnNode;

class ArnXmlLoader
{
public:
	ArnXmlLoader();
	~ArnXmlLoader();
protected:
private:
};

ARAN_API int InitializeXmlParser();
ARAN_API void DeallocateXmlParser();

ArnNode* CreateArnNodeFromXmlElement(DOMElement* elm, char* binaryChunkBasePtr);

#endif // ARNXMLLOADER_H
