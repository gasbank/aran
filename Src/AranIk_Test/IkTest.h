/*!
 * @file IkTest.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * ARAN 라이브러리 중 AranIk 패키지의 기능을 시험해보는 프로그램입니다.
 * Inverse kinematics를 이용해 ArnSkeleton을 조작하는 것이 주된 목표입니다.
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
#include <boost/lambda/casts.hpp>
#include <boost/function/function_base.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/array.hpp>
#include <boost/circular_buffer.hpp>
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
#include "ArnPlane.h"
#include "ArnBone.h"

#include "Tree.h"
#include "Jacobian.h"
#include "Node.h"

/*
#include <CGAL/Cartesian.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/functional.h>
#include <CGAL/double.h>
#include <CGAL/convex_hull_2.h>
typedef double RT;
typedef   CGAL::Point_2<CGAL::Cartesian<double> >        Point_2;
*/

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
