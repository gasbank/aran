// Macros.h
// 2007, 2008, 2009 Geoyeob Kim
//
// This file is shared between Aran and ModelExporter C++ Projects at VS2005 Solution.
// Should have dependency on d3dx9.h but NOT have on any 3ds Max related headers
// since Aran game engine itself must not rely on 3ds Max.
//
#pragma once

#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) { DXTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif

#define V_OKAY(x) { HRESULT __hr__; if(FAILED(__hr__ = (x))) throw std::runtime_error("V_OKAY() FAILED"); }
#define V_VERIFY(x) { if (FAILED(x)) throw MyError(MEE_GENERAL_VERIFICATION_FAILED); }
#define GLOBAL_TEXTURE_FILE_PATH			"Textures\\"
#define GLOBAL_ARN_FILE_PATH				"Models\\"
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if((p)!=0) { (p)->Release(); (p) = 0; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if((p)!=0) { delete [] (p); (p) = 0; }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if((p)!=0) { delete (p); (p) = 0; }
#endif

#define TCHARSIZE(x) (sizeof(x)/sizeof(TCHAR))

#define ARN_PI       (3.14159265358979323846)
#ifndef M_PI
#define M_PI ARN_PI
#endif

#ifndef WIN32
#define TCHAR char
#endif

#ifndef EP_SAFE_RELEASE
#define EP_SAFE_RELEASE(p)      { if (p) { (p)->release(); SAFE_DELETE(p); } }
#endif

template<typename T> void
EpSafeReleaseAll( T& obj )
{
	typename T::iterator it = obj.begin();
	for ( ; it != obj.end(); ++it )
	{
		EP_SAFE_RELEASE( *it );
	}
	obj.clear();
}


#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif


#if defined(DEBUG) | defined(_DEBUG)
#define ASSERTCHECK(x) \
	if (! (x)) \
{ \
	char lineNumber[8]; \
	_itoa_s(__LINE__, lineNumber, 10); \
	std::string ___file___Name___(__FILE__); \
	___file___Name___ += "("; \
	___file___Name___ += lineNumber; \
	___file___Name___ += ")\nFollowing statement is NOT TRUE or 0;\n"; \
	___file___Name___ += #x; \
	MessageBoxA(0, ___file___Name___.c_str(), "ASSERTION ERROR!", MB_OK | MB_ICONERROR); \
	return E_FAIL; \
}
#else
#define ASSERTCHECK(x)
#endif

// For annotation purpose
#define ARN_REMOVED
#define ARN_OWNERSHIP
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

// Throws
#define ARN_THROW_REMOVE_FUNCTION_ERROR \
	{ throw new std::runtime_error("This function should not be called because it was removed by the design issue!"); }
#define ARN_THROW_NOT_IMPLEMENTED_ERROR \
	{ throw new std::runtime_error("Not implemented!"); }
#define ARN_THROW_UNEXPECTED_CASE_ERROR \
	{ throw new std::runtime_error("Unexpected error!"); }
#define ARN_THROW_SHOULD_NOT_BE_USED_ERROR \
	{ throw new std::runtime_error("This code should not be used!"); }


//
// DLL export macro for Aran (ARAN Core Package)
//
#ifdef WIN32
	#define ARAN_API_EXPORT __declspec(dllexport)
	#define ARAN_API_IMPORT __declspec(dllimport)
#else
	#define ARAN_API_EXPORT
	#define ARAN_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARAN_EXPORTS)
	#define ARAN_API			ARAN_API_EXPORT
	#define ARAN_API_EXTERN
#else
	#define ARAN_API			ARAN_API_IMPORT
	#define ARAN_API_EXTERN		extern
#endif

//
// DLL Export macro for AranMath (ARAN Math Package)
//
#ifdef WIN32
	#define ARANMATH_API_EXPORT __declspec(dllexport)
	#define ARANMATH_API_IMPORT __declspec(dllimport)
#else
	#define ARANMATH_API_EXPORT
	#define ARANMATH_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARANMATH_EXPORTS)
	#define ARANMATH_API		ARANMATH_API_EXPORT
	#define ARANMATH_API_EXTERN
#else
	#define ARANMATH_API		ARANMATH_API_IMPORT
	#define ARANMATH_API_EXTERN extern
#endif

//
// DLL Export macro for AranPhy (ARAN Physics Package)
//
#ifdef WIN32
#define ARANPHY_API_EXPORT __declspec(dllexport)
#define ARANPHY_API_IMPORT __declspec(dllimport)
#else
#define ARANPHY_API_EXPORT
#define ARANPHY_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARANPHY_EXPORTS)
#define ARANPHY_API		ARANPHY_API_EXPORT
#define ARANPHY_API_EXTERN
#else
#define ARANPHY_API		ARANPHY_API_IMPORT
#define ARANPHY_API_EXTERN extern
#endif


//
// DLL Export macro for AranDx9 (ARAN Direct3D 9 Renderer Package)
//
#ifdef WIN32
#define ARANDX9_API_EXPORT __declspec(dllexport)
#define ARANDX9_API_IMPORT __declspec(dllimport)
#else
#define ARANDX9_API_EXPORT
#define ARANDX9_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARANDX9_EXPORTS)
#define ARANDX9_API		ARANDX9_API_EXPORT
#define ARANDX9_API_EXTERN
#else
#define ARANDX9_API		ARANDX9_API_IMPORT
#define ARANDX9_API_EXTERN extern
#endif

//
// DLL Export macro for AranGl (ARAN OpenGL Renderer Package)
//
#ifdef WIN32
#define ARANGL_API_EXPORT __declspec(dllexport)
#define ARANGL_API_IMPORT __declspec(dllimport)
#else
#define ARANGL_API_EXPORT
#define ARANGL_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARANGL_EXPORTS)
#define ARANGL_API		ARANGL_API_EXPORT
#define ARANGL_API_EXTERN
#else
#define ARANGL_API		ARANGL_API_IMPORT
#define ARANGL_API_EXTERN extern
#endif


//
// DLL Export macro for AranIk (ARAN Inverse Kinematics Package)
//
#ifdef WIN32
#define ARANIK_API_EXPORT __declspec(dllexport)
#define ARANIK_API_IMPORT __declspec(dllimport)
#else
#define ARANIK_API_EXPORT
#define ARANIK_API_IMPORT
#endif

#if defined(_USRDLL) && defined(ARANIK_EXPORTS)
#define ARANIK_API		ARANIK_API_EXPORT
#define ARANIK_API_EXTERN
#else
#define ARANIK_API		ARANIK_API_IMPORT
#define ARANIK_API_EXTERN extern
#endif

// limits a value to low and high
#define LIMIT_RANGE(low, value, high)	{	if (value < low)	value = low;	else if(value > high)	value = high;	}
#define ZERO_CLAMP(x)	( (EPSILON > fabs(x))?0.0f:(x) )						// set float to 0 if within tolerance
#define FLOAT_EQ(x,v)	( ((v) - EPSILON) < (x) && (x) < ((v) + EPSILON) )		// float equality test
#define	SQR(x)		( (x) * (x) )
#define FOUR_BYTES_INTO_DWORD(i1, i2, i3, i4)  ((DWORD)((i1)&0xff | (((i2)&0xff)<<8) | (((i3)&0xff)<<16) | (((i4)&0xff)<<24) ))

#ifndef DWORD
	#define DWORD unsigned int
#endif
#ifndef HRESULT
	#define HRESULT int
#endif
#ifndef BOOL
	#define BOOL int
#endif
#ifndef S_OK
	#define S_OK (0)
#endif
#ifndef E_FAIL
	#define E_FAIL (-1)
#endif

#define TYPEDEF_SHARED_PTR(type) \
	class type; \
	typedef std::tr1::shared_ptr < type > type##Ptr; \
	typedef std::tr1::shared_ptr < const type > type##ConstPtr;
