#ifndef ARANIKPCH_H_INCLUDED
#define ARANIKPCH_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <vector>
#include <list>
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
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#define foreach BOOST_FOREACH

//
// Aran Library
//
#include "AranApi.h"

#endif // ARANIKPCH_H_INCLUDED
