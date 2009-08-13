#ifndef ARANIKPCH_H_INCLUDED
#define ARANIKPCH_H_INCLUDED

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

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

//
// Boost C++
//
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#define foreach BOOST_FOREACH

//
// Aran Library
//
#include "AranApi.h"

#endif // ARANIKPCH_H_INCLUDED
