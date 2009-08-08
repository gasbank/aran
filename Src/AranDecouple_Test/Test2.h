#pragma once

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
	#include <memory>
#else
	#include <tr1/memory>
#endif

#ifdef WIN32
	#include <SDL.h>
	#include <SDL_opengl.h>
#else
	#define GL_GLEXT_PROTOTYPES
	#include <SDL/SDL.h>
	#include <SDL/SDL_opengl.h>
#endif

#include "ft2build.h"
#include FT_FREETYPE_H

//
// ODE
//
#include <ode/ode.h>

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
// Aran
//
#include "AranApi.h"
#include "VideoManGl.h"
#include "AranGl.h"
#include "ArnTextureGl.h"
#include "ArnGlExt.h"
#include "AranPhy.h"
#include "ArnPhyBox.h"
#include "SimWorld.h"
