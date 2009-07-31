#include "AranGlPCH.h"

#ifdef WIN32

#define ARN_GL_EXT_ENTRY(type, var) type var;
#include "ArnGlExtEntry.h"
#undef ARN_GL_EXT_ENTRY

int ArnInitGlExtFunctions()
{
	#define ARN_GL_EXT_ENTRY(type, var) if ((var = (type)wglGetProcAddress(#var)) == 0) return -5;
	#include "ArnGlExtEntry.h"
	#undef ARN_GL_EXT_ENTRY
	return 0;
}

#endif

#ifndef WIN32
ARAN_API int ArnInitGlExtFunctions()
{
	// Do nothing on linux
	return 0;
}
#endif
