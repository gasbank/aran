/*!
@file Test2.h
@author Geoyeob Kim
@date 2009

ARAN 라이브러리의 각종 기능을 시험해볼 수 있는 scene graph renderer 예제 코드입니다.
이 예제에서는 Blender 익스포터로 만들어진 XML 파일을 읽어서 화면에 보여줍니다.
사용자와의 상호작용은 없습니다.
*/
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
#include "GeneralJoint.h"
#include "AranIk.h"
#include "ArnIkSolver.h"

#include "Tree.h"
#include "Jacobian.h"
#include "Node.h"

struct HitRecord
{
	GLuint numNames;	// Number of names in the name stack for this hit record
	GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
	GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
	GLuint contents;	// Name stack contents
};

enum MessageHandleResult
{
	MHR_DO_NOTHING,
	MHR_EXIT_APP,
	MHR_NEXT_SCENE,
	MHR_RELOAD_SCENE
};
