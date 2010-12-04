#ifndef __BWPCH_H__
#define __BWPCH_H__

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
#include <GL/glew.h>
//#include <GL/glxew.h>
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
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
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

#include "fltkcustomization.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Slider.H>
#include <FL/gl.h>
#include <FL/math.h>
#include <FL/fl_draw.H>

#endif // #ifndef __BWPCH_H__
