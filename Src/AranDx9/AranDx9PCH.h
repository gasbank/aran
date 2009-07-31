// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

//
// Xerces-c
//
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE


//
// Aran
//
#include "Macros.h"
#include "ArnCommonTypes.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnMatrix.h"
#include "ArnQuat.h"
