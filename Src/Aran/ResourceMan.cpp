#include "StdAfx.h"
#include "ResourceMan.h"
#include "VideoMan.h"

IMPLEMENT_SINGLETON(ResourceMan);

#ifndef _LogWrite
#define _LogWrite(a,b)
#endif

ResourceMan::ResourceMan(void)
{
}

ResourceMan::~ResourceMan(void)
{
	unregisterAllModels();
}

HRESULT ResourceMan::registerModel( MODELID id, const TCHAR* modelFileName )
{
	TCHAR logMessage[128];

	ModelMap::iterator it = m_models.find( id );
	ModelMap::iterator itEnd = m_models.end();

	if ( it == itEnd ) // registration valid (not exist already)
	{
		ModelReader* pModelReader = new ModelReader();
		pModelReader->SetFileName( modelFileName );
		m_models.insert( ModelMap::value_type( id, pModelReader ) );

		_stprintf_s( logMessage, TCHARSIZE(logMessage), _T("%s%s"), _T( "Model Loading: " ), modelFileName );
		_LogWrite( logMessage, LOG_OKAY );
		
		return S_OK;
	}
	else
	{
		// already exist...
		_stprintf_s( logMessage, TCHARSIZE(logMessage), _T("%s%s"), _T( "Model Loading: " ), modelFileName );
		_LogWrite( logMessage, LOG_FAIL );
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
				NULL
				);
			if ( FAILED( hr ) )
			{
				_LogWrite( _T( "Model Loading Error!" ), LOG_FAIL );
				return -1;
			}
			else
			{
				pMR->AdvanceTime( 0.001f );
				++initedCount;
			}
			
		}
	}
	_LogWrite( _T( "Model Initialization" ), LOG_OKAY );
	return S_OK;
}

HRESULT ResourceMan::unregisterAllModels()
{
	while ( m_models.size() )
	{
		ModelMap::iterator it = m_models.begin();
		ModelReader* pMR = it->second;
		SAFE_DELETE( pMR );
		m_models.erase(it);
	}
	return S_OK;
}

const ModelReader* ResourceMan::getModel( MODELID id ) const
{
	ModelMap::const_iterator it = m_models.find( id );
	ModelMap::const_iterator itEnd = m_models.end();

	if ( it != itEnd )
	{
		return it->second;
	}
	else
	{
		return NULL; // not exist!
	}
}