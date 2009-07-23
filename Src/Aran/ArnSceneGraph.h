#pragma once
#include "ArnNode.h"

struct ArnFileData;
class ArnHierarchy;
class ArnBone;
class ArnBinaryChunk;
class ArnLight;

class ArnSceneGraph : public ArnNode
{
public:
									~ArnSceneGraph(void);

	static ArnSceneGraph*			createFromEmptySceneGraph();
	static ArnSceneGraph*			createFrom(const ArnFileData* afd);
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
									ArnSceneGraph(const ArnFileData* afd);

	void							createEmptyRootNode();
	void							postprocessingARN20();
	void							buildBoneHierarchy( ArnHierarchy* hierNode, ArnNode* skelNode, ArnBone* parentBoneNode );
	bool							m_bRendererObjectInited;
	const ArnFileData*				m_afd;
	EXPORT_VERSION					m_exportVersion;
	//ArnNode*						m_sceneRoot;  // DEPRECATED
	ArnBinaryChunk*					m_binaryChunk;
};

#include "ArnSceneGraph.inl"
