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

#ifdef WIN32
#include <d3dx9.h>
#endif
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE


#include "IL/il.h"

#include "AranApi.h"
#include "VideoManGl.h"
#include "ArnGlExt.h"
