// Main.cpp
// 2007 Geoyeob Kim
#include "stdafx.h"
#include "Main.h"
#include "ModelExporterClassDesc.h"			// classic
#include "ModelExporter2ClassDesc.h"		// new version

HINSTANCE g_hInstance;
ModelExporterClassDesc modelExporterClassDesc;
ModelExporter2ClassDesc modelExporter2ClassDesc;


ClassDesc2* GetModelExporterClassDesc()
{
	return &modelExporter2ClassDesc;
}

__declspec(dllexport) const TCHAR* LibDescription()
{
#ifdef MODEL_EXPORTER_FOR_MAX_9
	return _T("Aran Model Exporter for 3ds Max 9");
#elif defined MODEL_EXPORTER_FOR_MAX_2008
	return _T("Aran Model Exporter for 3ds Max 2008");
#endif

}

__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	switch (i)
	{
	case 0:
		return GetModelExporterClassDesc();
	default:
		return NULL;
	}
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec(dllexport) ULONG CanAutoDefer()
{
	return 1;
}

LRESULT __declspec(dllexport) __stdcall PluginCommit()
{
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hDllHandle;
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

