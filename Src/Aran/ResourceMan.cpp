#include "StdAfx.h"
#include "ResourceMan.h"
#include "../VideoLib/VideoMan.h"

ResourceMan::ResourceMan(void)
{
}

ResourceMan::~ResourceMan(void)
{
}

HRESULT ResourceMan::registerModel( MODELID id, const TCHAR* modelFileName )
{
	ModelMap::iterator it = m_models.find( id );
	ModelMap::iterator itEnd = m_models.end();

	if ( it == itEnd ) // registration valid (not exist already)
	{
		ModelReader* pModelReader = new ModelReader();
		pModelReader->SetFileName( modelFileName );
		m_models.insert( ModelMap::value_type( id, pModelReader ) );
		return S_OK;
	}
	else
	{
		// already exist...
		return E_FAIL;
	}
}

HRESULT ResourceMan::unregisterModel( MODELID id )
{
	ModelMap::iterator it = m_models.find( id );
	ModelMap::iterator itEnd = m_models.end();

	if ( it != itEnd )
	{
		delete it->second;
		m_models.erase( it );
		return S_OK;
	}
	else
	{
		return E_FAIL; // not exist!
	}
}

int ResourceMan::initializeAll()
{	
	ModelMap::iterator it = m_models.begin();
	ModelMap::iterator itEnd = m_models.end();

	
	int initedCount = 0;
	for ( ; it != itEnd; ++it )
	{
		ModelReader* pMR = it->second;
		if ( !pMR->IsInitialized() )
		{
			HRESULT hr = pMR->Initialize(
				VideoMan::getSingleton().GetDev(),
				ARN_VDD::ARN_VDD_FVF,
				NULL,
				NULL,
				NULL );
			if ( FAILED( hr ) )
			{
				_LogWrite( "Model Loading Error!", LOG_FAIL );
				return -1;
			}
			else
			{
				++initedCount;
			}
			
		}
	}
	return S_OK;
}