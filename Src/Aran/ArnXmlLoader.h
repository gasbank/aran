/*!
@file ArnXmlLoader.h
@author Geoyeob Kim
@date 2009
*/
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

/*!
@brief XML 파서 초기화
*/
ARAN_API int ArnInitializeXmlParser();
/*!
@brief XML 파서 해제
*/
ARAN_API void ArnCleanupXmlParser();

class TiXmlElement;

ArnNode* CreateArnNodeFromXmlElement(const TiXmlElement* elm, const char* binaryChunkBasePtr);

#endif // ARNXMLLOADER_H
