#pragma once

#include "ArNode.h"

class ArMesh;
class ArCamera;
class ArLight;

class ModelExporter2 : public Singleton<ModelExporter2>, public SceneExport
{
public:
	ModelExporter2(void);
	~ModelExporter2(void);

	int				ExtCount();					// Number of extensions supported
	const char*		Ext(int n);					// Extension #n (i.e. "3DS")
	const char*		LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const char*		ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const char*		AuthorName();				// ASCII Author name
	const char*		CopyrightMessage();			// ASCII Copyright message
	const char*		OtherMessage1();			// Other message #1
	const char*		OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	BOOL SupportsOptions(int ext, DWORD options);
	int	DoExport(const char *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

	// end of overrides
	//////////////////////////////////////////////////////////////////////////

	ArNode* const getNodeByName(std::string name) const;

private:
	void exportWorld();
	void exportNode(IGameNode* node);
	void exportNodeSub(IGameNode* node, IGameMesh* mesh, ArMesh* arMesh);
	void exportNodeSub(IGameCamera* camera, ArCamera* arCamera);
	void exportNodeSub(IGameLight* light, ArLight* arLight);

	// .ARN represents a model and that is consists of one or more nodes;
	typedef std::map<std::string, ArNode*> NodeMap;
	NodeMap m_nodes;


	// basic variables
	BOOL m_bFirstTimeInit;
	BOOL m_bExportSuccessful;
	char m_exportTime[128]; // export starting date and time

	Interface* m_coreInterface;
	TimeValue m_coreFrame;

	std::string m_exportName; // export file name
	IGameScene* m_gameScene;
};
