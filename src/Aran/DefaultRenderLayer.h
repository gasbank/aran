#pragma once
#include "renderlayer.h"

namespace Aran
{
	class Character;
}
class ArnObject;
class ArnSceneGraph;
struct ArnFileData;

class DefaultRenderLayer :
	public RenderLayer
{
public:
	DefaultRenderLayer(Aran::Character* pChar);
	~DefaultRenderLayer(void);

	virtual HRESULT render(double fTime, float fElapsedTime);

private:
	Aran::Character* m_pChar;
};

//////////////////////////////////////////////////////////////////////////

class BoxRenderLayer : public RenderLayer
{
public:
	BoxRenderLayer();
	~BoxRenderLayer();

	virtual HRESULT render(double fTime, float fElapsedTime);

private:
	
	TCHAR m_debugBuffer[512];
	

	std::vector<ArnObject*> m_objects;

	LPD3DXMESH m_testMesh;

	ArnSceneGraph* m_simpleSG;
	ArnFileData* m_arnFileData;
};

LPD3DXMESH newTestPlaneMesh(float width, float height, int segWidth, int segHeight);