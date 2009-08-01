// AranPCH.h
// 2008, 2009 Geoyeob Kim (gasbank@gmail.com)
#pragma once
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <CommCtrl.h>
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
// Aran Library
//
#include "Macros.h"
#include "ArnCommonTypes.h"
#include "Structs.h"
#include "Singleton.h"
#include "Log.h"
#include "MyError.h"

template <typename T>
class ArrayDeleter
{
public:
	void operator () (T* d) const
	{
		delete [] d;
	}
};
