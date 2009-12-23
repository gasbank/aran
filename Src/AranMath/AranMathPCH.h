#ifndef __ARANMATHPCH_H__
#define __ARANMATHPCH_H__

#ifdef WIN32
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <d3d9.h>
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

//
// Boost
//
#include <boost/array.hpp>

//
// Configurable Math Library (cml)
//
#include "cml/cml.h"

//
// Aran Library Global Macros
//
#include "Macros.h"
#include "ArnCommonTypes.h"
#include "AranMathTypeDefs.h"

#endif
