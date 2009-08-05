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

class TiXmlElement;

ArnNode* CreateArnNodeFromXmlElement(const TiXmlElement* elm, const char* binaryChunkBasePtr);

#endif // ARNXMLLOADER_H
