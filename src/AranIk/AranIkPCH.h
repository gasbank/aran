#ifndef __ARANIKPCH_H__
#define __ARANIKPCH_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <vector>
#include <list>
#include <float.h>
#include <iomanip>
#ifdef WIN32
	#include <memory>
#else
	#include <tr1/memory>
#endif

using namespace std;

#ifdef WIN32
#include <windows.h>
#endif

//
// Boost C++
//
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#define foreach BOOST_FOREACH

//
// Aran Library
//
#include "AranApi.h"

#endif
