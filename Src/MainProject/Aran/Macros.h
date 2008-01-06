// Macros.h
// 2007 Geoyeob Kim
//
// This file is shared between Aran and ModelExporter C++ Projects at VS2005 Solution.
// Should have dependency on d3dx9.h but NOT have on any 3ds Max related headers
// since Aran game engine itself must not rely on 3ds Max.
//
#pragma once


#define V_OKAY(x) { HRESULT __hr__; if(FAILED(__hr__ = x)) return DXTRACE_ERR_MSGBOX(_T("V_OKAY() FAILED"), __hr__); }
#define GLOBAL_TEXTURE_FILE_PATH			"e:\\tex\\"
#define GLOBAL_ARN_FILE_PATH				"e:\\max\\export\\"
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if((p)!=NULL) { (p)->Release(); (p) = NULL; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if((p)!=NULL) { delete [] (p); (p) = NULL; }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if((p)!=NULL) { delete (p); (p) = NULL; }
#endif

namespace std {
#if defined _UNICODE || defined UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}


#ifndef DEBUG
#define ASSERTCHECK(x)
#else
#define ASSERTCHECK(x) \
	if (! (x)) \
{ \
	char lineNumber[8]; \
	_itoa_s(__LINE__, lineNumber, 10); \
	std::string fileName(__FILE__); \
	fileName += "("; \
	fileName += lineNumber; \
	fileName += ")\nFollowing statement is NOT TRUE;\n"; \
	fileName += #x; \
	MessageBoxA(NULL, fileName.c_str(), "ASSERTION ERROR!", MB_OK | MB_ICONERROR); \
	return E_FAIL; \
}
#endif



