#pragma once
#include "ArnNode.h"

struct ArnFileData;
class ArnHierarchy;
class ArnBone;
class ArnBinaryChunk;
class ArnLight;

class ARAN_API ArnSceneGraph : public ArnNode
{
public:
									~ArnSceneGraph(void);

	static ArnSceneGraph*			createFromEmptySceneGraph();
	
	static ArnSceneGraph*			createFrom(const char* fileName);

	void							attachToRoot(ArnNode* node);
	void							render();
	void							initRendererObjects();
	ArnNode* findFirstNodeOfType(NODE_DATA_TYPE ndt);

	// *** INTERNAL USE ONLY START ***
	virtual void					interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
									ArnSceneGraph();
	EXPORT_VERSION					m_exportVersion;
	ArnBinaryChunk*					m_binaryChunk;
};

#include "ArnSceneGraph.inl"
