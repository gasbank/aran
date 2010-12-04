#include "AranDx9PCH.h"
#include "ArnTextureDx9.h"
#include "VideoManDx9.h"

ArnTextureDx9::ArnTextureDx9(void)
: m_target (0)
, m_d3d9Tex (0)
{
}

ArnTextureDx9::~ArnTextureDx9(void)
{
	SAFE_RELEASE (m_d3d9Tex);
}

bool ArnTextureDx9::initialize()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

bool ArnTextureDx9::Release()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

ArnTextureDx9* ArnTextureDx9::createFrom( const ArnTexture* tex )
{
	if (tex)
	{
		ArnTextureDx9 *ret = new ArnTextureDx9();
		ret->m_target = tex;
		if (ret->init() < 0)
		{
			delete ret;
			printf("ArnTextureDx9 initialization failed!\n");
			return 0;
		}
		else
		{
			ret->setInitialized(true);
			ret->setRendererType(RENDERER_DX9);
			return ret;
		}
	}
	else
		return 0;
}

int ArnTextureDx9::render( bool bIncludeShadeless ) const
{
	assert(m_d3d9Tex);
	assert(GetVideoManagerDx9 ().GetDev ());
	D3DPERF_BeginEvent(0xffff0000, L"Texture set");
	GetVideoManagerDx9 ().GetDev ()->SetTexture(0, m_d3d9Tex);
	D3DPERF_EndEvent();
	return 0;
}

void ArnTextureDx9::cleanup()
{
	SAFE_RELEASE (m_d3d9Tex);
}

int ArnTextureDx9::init()
{
	SAFE_RELEASE (m_d3d9Tex);

	HRESULT hr = S_OK;
	assert(m_target);
	assert(m_target->isInitialized());
	V_RETURN( D3DXCreateTextureFromFileA( GetVideoManagerDx9 ().GetDev (), m_target->getFileName(), &m_d3d9Tex ) );
	return hr;
}
