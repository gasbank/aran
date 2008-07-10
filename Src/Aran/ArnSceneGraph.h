#pragma once

struct ArnFileData;
class ArnNode;

class ArnSceneGraph
{
public:
	ArnSceneGraph(const ArnFileData& afd);
	~ArnSceneGraph(void);

private:
	const ArnFileData& m_afd;
	ArnNode* m_sceneRoot;
};
