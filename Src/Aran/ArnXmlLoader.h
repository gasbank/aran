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

ArnNode* CreateArnNodeFromXmlElement(DOMElement* elm, char* binaryChunkBasePtr);
ARAN_API int InitializeXmlParser();
ARAN_API void DeallocateXmlParser();

#endif // ARNXMLLOADER_H
