/*!
 * @file ArnSceneGraph.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * Scene Graph
 */
#pragma once
#include "ArnNode.h"

struct ArnFileData;
class ArnHierarchy;
class ArnBone;
class ArnBinaryChunk;
class ArnLight;
class ArnCamera;

TYPEDEF_SHARED_PTR(ArnSceneGraph)

/*!
 * @brief 장면 그래프(scene graph)
 */
class ARAN_API ArnSceneGraph : public ArnNode
{
public:
									~ArnSceneGraph(void);
	/*!
	 * @brief 빈 장면 그래프를 생성 후 반환
	 */
	static ArnSceneGraph*			createFromEmptySceneGraph();
	/*!
	 * @brief Aran XML 장면 그래프 파일로부터 장면 그래프 생성 후 반환
	 */
	static ArnSceneGraph*			createFrom(const char* fileName);
	void							attachToRoot(ArnNode* node);
	void							render();
	void							initRendererObjects();
	ArnNode*						findFirstNodeOfType(NODE_DATA_TYPE ndt);
	/*!
	 * @brief 장면 그래프에 등록된 \a cam 다음 카메라를 반환
	 *
	 * 장면 그래프에 카메라가 추가될 때 \a m_cameraList 에도 등록이 됩니다.
	 * 이 함수는 \a cam 을 \a m_cameraList 에서 찾아 그 다음 카메라 포인터를
	 * 반환합니다. \a m_cameraList 에 \a cam 이 없을 경우에는 \a m_cameraList 의
	 * 첫 번째 원소를 반환하게 되며, \a m_cameraList 가 비어있을 경우에는 \c NULL 을
	 * 반환합니다. 같은 원리로 \a cam 에 \c NULL 을 주고, \c m_cameraList 가 비어있지
	 * 않다면 첫 번째 카메라를 반환받을 수 있습니다.
	 */
	ArnCamera*						getNextCamera(const ArnCamera* cam) const;
	/*!
	 * @brief 장면 그래프에 등록된 첫 번째 카메라를 반환
	 */
	ArnCamera*						getFirstCamera() const;
	/*!
	 * @internalonly
	 */
	//@{
	virtual void					interconnect(ArnNode* sceneRoot);
	//@}
private:
									ArnSceneGraph();
	EXPORT_VERSION					m_exportVersion;
	ArnBinaryChunk*					m_binaryChunk;
	std::list<ArnCamera*>			m_cameraList;
};

#include "ArnSceneGraph.inl"
