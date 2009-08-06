#pragma once
#include "ArnNode.h"

struct ArnFileData;
class ArnHierarchy;
class ArnBone;
class ArnBinaryChunk;
class ArnLight;

TYPEDEF_SHARED_PTR(ArnSceneGraph);

class ARAN_API ArnSceneGraph : public ArnNode
{
public:
									~ArnSceneGraph(void);
	static ArnSceneGraph*			createFromEmptySceneGraph();
	static ArnSceneGraph*			createFrom(const char* fileName);
	void							attachToRoot(ArnNode* node);
	void							render();
	void							initRendererObjects();
	ArnNode*						findFirstNodeOfType(NODE_DATA_TYPE ndt);
	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	virtual void					interconnect(ArnNode* sceneRoot);
	//@}
private:
									ArnSceneGraph();
	EXPORT_VERSION					m_exportVersion;
	ArnBinaryChunk*					m_binaryChunk;
};

#include "ArnSceneGraph.inl"
