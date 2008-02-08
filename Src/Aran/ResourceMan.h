#pragma once


class ResourceMan
{
public:
	ResourceMan(void);
	~ResourceMan(void);


	enum MODELID
	{
		MAN,
		DUNGEON,
		BOX1,
		BOX2,
		BOX3,

		MODEL_COUNT,
	};
	HRESULT registerModel( MODELID id, const TCHAR* modelFileName );
	HRESULT unregisterModel( MODELID id );
	int initializeAll();

private:
	typedef std::map<MODELID, ModelReader*> ModelMap;
	ModelMap m_models;
};
