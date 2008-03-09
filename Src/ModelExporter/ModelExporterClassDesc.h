// ModelExporterClassDesc.h
// 2007, 2008 Geoyeob Kim
//
#pragma once

#include "iparamb2.h"

#define MODELEXPORTER_CLASS_ID Class_ID(0x938ff44a, 0xC8476283)

class ModelExporterClassDesc : public ClassDesc2
{
public:
	ModelExporterClassDesc(void);
	~ModelExporterClassDesc(void);

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


