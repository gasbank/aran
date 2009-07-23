// AranPCH.h
// 2008, 2009 Geoyeob Kim (gasbank@gmail.com)
#pragma once
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <d3dx9.h>
	#include <dxerr9.h>
	#include <CommCtrl.h>

	//#pragma warning(disable:4505)
	//#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds
#endif

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include <fstream>
#include <list>
#include <vector>

#ifndef WIN32
	#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"

#ifndef WIN32
    #ifdef UNICODE
		#define _T(x) L##x
	#else
		#define _T(x) x
	#endif

    #define WIDEN(x) L ## x
	#define WIDEN2(x) WIDEN(x)
	#define __WFILE__ WIDEN2(__FILE__)
	#define __WFUNCTION__ WIDEN2(__FUNCTION__)
#endif

//
// xerces C++ XML Parser
//
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
XERCES_CPP_NAMESPACE_USE

//
// Boost C++
//
#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#define foreach BOOST_FOREACH

//
// Configurable Math Library (cml)
//
#include "cml/cml.h"

//
// Aran Library
//
#include "Macros.h"
#include "Structs.h"
#include "Singleton.h"
#include "Log.h"
#include "MyError.h"

//
// OpenGL Extensions (Win32 only)
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
//
#ifdef WIN32
extern PFNGLGENBUFFERSARBPROC					glGenBuffersARB;                     // VBO Name Generation Procedure
extern PFNGLBINDBUFFERARBPROC					glBindBufferARB;                     // VBO Bind Procedure
extern PFNGLBUFFERDATAARBPROC					glBufferDataARB;                     // VBO Data Loading Procedure
extern PFNGLBUFFERSUBDATAARBPROC				glBufferSubDataARB;               // VBO Sub Data Loading Procedure
extern PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARB;               // VBO Deletion Procedure
extern PFNGLGETBUFFERPARAMETERIVARBPROC			glGetBufferParameterivARB; // return various parameters of VBO
extern PFNGLMAPBUFFERARBPROC					glMapBufferARB;                       // map VBO procedure
extern PFNGLUNMAPBUFFERARBPROC					glUnmapBufferARB;                   // unmap VBO procedure

extern PFNGLACTIVETEXTUREARBPROC				glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD1DARBPROC				glMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD1DVARBPROC				glMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD1FARBPROC				glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FVARBPROC				glMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1IARBPROC				glMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1IVARBPROC				glMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1SARBPROC				glMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1SVARBPROC				glMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD2DARBPROC				glMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD2DVARBPROC				glMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC				glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2IARBPROC				glMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2IVARBPROC				glMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2SARBPROC				glMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2SVARBPROC				glMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD3DARBPROC				glMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD3DVARBPROC				glMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD3FARBPROC				glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC				glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3IARBPROC				glMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3IVARBPROC				glMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3SARBPROC				glMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3SVARBPROC				glMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD4DARBPROC				glMultiTexCoord4dARB;
extern PFNGLMULTITEXCOORD4DVARBPROC				glMultiTexCoord4dvARB;
extern PFNGLMULTITEXCOORD4FARBPROC				glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC				glMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4IARBPROC				glMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4IVARBPROC				glMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4SARBPROC				glMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4SVARBPROC				glMultiTexCoord4svARB;
#endif
