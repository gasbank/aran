#pragma once

#include "iparamb2.h"

#define MODELEXPORTER2_CLASS_ID Class_ID(0x93848392, 0xC8223913)

class ModelExporter2ClassDesc : public ClassDesc2
{
public:
	ModelExporter2ClassDesc(void);
	~ModelExporter2ClassDesc(void);

	// See 'class ClassDesc' not 'class ClassDesc2'
	BOOL IsPublic();
	void* Create(BOOL loading = FALSE);
	const char* ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const char* Category();

	const char* InternalName(); // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance(); // returns owning module handle
};
