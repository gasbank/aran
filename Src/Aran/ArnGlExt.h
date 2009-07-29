#pragma once

//
// OpenGL Extensions (Win32 only)
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
//
// Note: These externals should be declared in DllMain.cpp and set in ArnInitGlExtFunctions()
//
#ifdef WIN32
	#define ARN_GL_EXT_ENTRY(type, var) extern ARAN_API type var;
	#include "ArnGlExtEntry.h"
	#undef ARN_GL_EXT_ENTRY
#endif

ARAN_API int ArnInitGlExtFunctions();
