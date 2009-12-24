#pragma once

class ArnSceneGraphDx9 : public ArnSceneGraph
{
public:
									~ArnSceneGraphDx9(void);
	static ArnSceneGraphDx9*		createFrom(const ArnFileData* afd);

private:
									ArnSceneGraphDx9(void);
									ArnSceneGraphDx9(const ArnFileData* afd);
	void							postprocessingARN20();
	void							buildBoneHierarchy( ArnHierarchy* hierNode, ArnNode* skelNode, ArnBone* parentBoneNode );
	const ArnFileData*				m_afd;
};
