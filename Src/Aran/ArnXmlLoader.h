/*!
 * @file ArnXmlLoader.h
 * @author Geoyeob Kim
 * @date 2009
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
 * @brief XML 파서 초기화
 * @return 성공 시 0, 실패(재 초기화 포함) 시 음수
 */
ARAN_API int ArnInitializeXmlParser();
/*!
 * @brief XML 파서 해제
 * @return 성공 시 0, 실패(미 초기화 후 호출 포함) 시 음수
 */
ARAN_API int ArnCleanupXmlParser();

class TiXmlElement;

ArnNode* CreateArnNodeFromXmlElement(const TiXmlElement* elm, const char* binaryChunkBasePtr);

#endif // ARNXMLLOADER_H
