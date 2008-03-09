// ModelExporter2ClassDesc.cpp
// 2007, 2008 Geoyeob Kim
#include "stdafx.h"
#include "ModelExporter2ClassDesc.h"
#include "ModelExporter2.h"

extern HINSTANCE g_hInstance; // declared at Main.cpp

ModelExporter2ClassDesc::ModelExporter2ClassDesc(void)
{
}

ModelExporter2ClassDesc::~ModelExporter2ClassDesc(void)
{
}

BOOL ModelExporter2ClassDesc::IsPublic()
{
	return TRUE;
}
void* ModelExporter2ClassDesc::Create(BOOL loading)
{
	return new ModelExporter2();
}
const TCHAR* ModelExporter2ClassDesc::ClassName()
{
	return _T("ModelExporter2");
}
SClass_ID ModelExporter2ClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}
Class_ID ModelExporter2ClassDesc::ClassID()
{
	return MODELEXPORTER2_CLASS_ID;
}
const TCHAR* ModelExporter2ClassDesc::Category()
{
	return _T("Export");
}

const TCHAR* ModelExporter2ClassDesc::InternalName()
{
	return _T("ModelExporter2");
}
HINSTANCE ModelExporter2ClassDesc::HInstance()
{
	return g_hInstance;
}




