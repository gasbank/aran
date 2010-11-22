#pragma once
#ifndef __PYMRSPCH_H_
#define __PYMRSPCH_H_

#ifdef WIN32
#include <windows.h>
#endif

#include "PymPch.h"


#include <stdio.h>
#include <stdlib.h>
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
#include <memory>
#else
#include <tr1/memory>
#endif


#ifndef WIN32
#include <GL/glew.h>
#include <GL/glxew.h>
#include <sys/time.h>
#include <pthread.h>
#else
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#endif
#include <map>
#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/circular_buffer.hpp>
#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

#include "PymPch.h"

#include <cholmod.h>
#include <umfpack.h>
#include <mosek.h>
#include <libconfig.h>


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
// AranMath
//
#include "Macros.h"
#include "ArnCommonTypes.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnMath.h"
#include "AranApi.h"
#include "VideoManGl.h"
#include "AranGl.h"

#endif /* #ifndef __PYMRSPCH_H_ */
