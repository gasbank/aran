#pragma once
#include "../videolib/renderlayer.h"

class Character;

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
