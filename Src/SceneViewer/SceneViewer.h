/*!
 * @file SceneViewer.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * ARAN scene graph XML 파일을 렌더링해 주는 예제 프로젝트입니다.
 * SDL을 사용해 윈도우를 관리하며 SceneList.txt에 명시된 XML 파일을 읽어들여
 * 사용자에게 보여줍니다. scene graph간의 전환은 N 키를 사용하고,
 * 현재 scene graph 파일을 다시 로딩하고 싶은 경우에는 R 키를 사용합니다.
 * Action이 포함된 scene graph의 1, 2, 3번 키를 이용해 Action을 활성화시키거나
 * 비활성화시킬 수 있습니다.
 * 또한 마우스로 화면을 클릭하면 어떤 노드를 클릭했는지를 콘솔창에 나타내 줍니다.
 * FreeType의 기능을 조금 사용해 현재 로딩된 XML 파일 이름을 보여주는 시험적인
 * 기능도 포함하고 있습니다.
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
