#pragma once

#ifndef WIN32
#error This library can be compiled in Win32 environment only.
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <d3d9.h>
#include <d3dx9.h>

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
#include <set>
#include <memory>

//
// Boost C++
//
#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/array.hpp>
#define foreach BOOST_FOREACH

//
// Aran Library
//
#include "AranApi.h"
#include "MyError.h"
#include "StructsDx9.h"
#include "ArnRenderableObject.h"
#include "Log.h"
