#pragma once
#include "renderlayer.h"

namespace Aran
{
	class Character;
}
class ArnObject;
class ArnSceneGraph;
struct ArnFileData;

class DefaultRenderLayer : public RenderLayer
{
public:
	DefaultRenderLayer(Aran::Character* pChar);
	virtual ~DefaultRenderLayer(void);

	virtual HRESULT render();
	virtual HRESULT update(double fTime, float fElapsedTime);

private:
	Aran::Character* m_pChar;
};

//////////////////////////////////////////////////////////////////////////

class BoxRenderLayer : public RenderLayer
{
public:
	BoxRenderLayer();
	virtual ~BoxRenderLayer();

	virtual HRESULT render();
	virtual HRESULT update(double fTime, float fElapsedTime);

private:
	
	TCHAR m_debugBuffer[512];
	

	std::vector<ArnObject*> m_objects;

	LPD3DXMESH m_testMesh;

	ArnSceneGraph* m_simpleSG;
	ArnFileData* m_arnFileData;

	ArnSceneGraph* m_skelScene;
	ArnFileData* m_skelSceneData;
};

LPD3DXMESH newTestPlaneMesh(float width, float height, int segWidth, int segHeight);