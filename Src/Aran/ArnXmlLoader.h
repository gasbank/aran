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

ARAN_API int ArnInitializeXmlParser();
ARAN_API void ArnCleanupXmlParser();

ArnNode* CreateArnNodeFromXmlElement(DOMElement* elm, char* binaryChunkBasePtr);

#endif // ARNXMLLOADER_H
