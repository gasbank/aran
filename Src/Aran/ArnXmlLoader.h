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
int InitializeXmlParser();
void DeallocateXmlParser();

#endif // ARNXMLLOADER_H
