#pragma once

#include <windows.h>
#include <tchar.h>
#include "iparamb2.h"

#include "ModelExporter.h"

#define MODELEXPORTER_CLASS_ID Class_ID(0x938ff44a, 0xC8476283)

class ModelExporterClassDesc : public ClassDesc2
{
public:
	ModelExporterClassDesc(void);
	~ModelExporterClassDesc(void);

	// See 'class ClassDesc' not 'class ClassDesc2'
	BOOL IsPublic();
	void* Create(BOOL loading = FALSE);
	const TCHAR* ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();

	const TCHAR* InternalName(); // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance(); // returns owning module handle

};


