#ifndef OILIFY_H_INCLUDED
#define OILIFY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <set>
#ifdef WIN32
	#include <windows.h>
	#include <memory>
#else
	#include <tr1/memory>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"

#ifndef WIN32
#include "ft2build.h"
#include FT_FREETYPE_H
#endif

//
// ODE
//
#include <ode/ode.h>

//
// Boost C++
//
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#include <boost/circular_buffer.hpp>
#define foreach BOOST_FOREACH

//
// Aran
//
#include "AranApi.h"
#include "BwWin32Timer.h"

//
// DevIL
//
#include "IL/il.h"


#endif // OILIFY_H_INCLUDED
