#pragma once
#include "../videolib/renderlayer.h"
#include "../VideoLib/Structs.h"
#include <vector>

class Character;
class ArnObject;

class DefaultRenderLayer :
	public RenderLayer
{
public:
	DefaultRenderLayer(Character* pChar);
	~DefaultRenderLayer(void);

	virtual HRESULT render();

private:
	Character* m_pChar;
};

//////////////////////////////////////////////////////////////////////////

class BoxRenderLayer : public RenderLayer
{
public:
	BoxRenderLayer();
	~BoxRenderLayer();

	virtual HRESULT render();

private:
	
	TCHAR m_debugBuffer[512];
	D3DXMATRIX m_matWorld, m_matView, m_matProjection;

	std::vector<ArnObject*> m_objects;

	LPD3DXMESH m_testMesh;
};

LPD3DXMESH newTestPlaneMesh(float width, float height, int segWidth, int segHeight);
