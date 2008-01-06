// ModelExporterClassDesc.cpp
// 2007 Geoyeob Kim
//
#include "Main.h"
#include "ModelExporterClassDesc.h"

extern HINSTANCE g_hInstance; // declared at Main.cpp

ModelExporterClassDesc::ModelExporterClassDesc(void)
{
}

ModelExporterClassDesc::~ModelExporterClassDesc(void)
{
}

BOOL ModelExporterClassDesc::IsPublic()
{
	return TRUE;
}
void* ModelExporterClassDesc::Create(BOOL loading)
{
	return new ModelExporter();
}
const TCHAR* ModelExporterClassDesc::ClassName()
{
	return _T("ModelExporter");
}
SClass_ID ModelExporterClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}
Class_ID ModelExporterClassDesc::ClassID()
{
	return MODELEXPORTER_CLASS_ID;
}
const TCHAR* ModelExporterClassDesc::Category()
{
	return _T("Export");
}

const TCHAR* ModelExporterClassDesc::InternalName()
{
	return _T("ModelExporter");
}
HINSTANCE ModelExporterClassDesc::HInstance()
{
	return g_hInstance;
}




