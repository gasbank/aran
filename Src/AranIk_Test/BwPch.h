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

#include "ft2build.h"
#include FT_FREETYPE_H

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
 * BORDER_WIDTH:
 *
 * Thickness of FL_UP_BOX and FL_DOWN_BOX.  Current 1,2, and 3 are
 * supported.  3 is the historic FLTK look.  2 looks more like Microsoft
 * Windows, KDE, and Qt, and is the default when building for Windows.
 * 1 is a plausible future evolution...  Note that this may be simulated
 * at runtime by redefining the boxtypes using Fl::set_boxtype().
 */

#define BORDER_WIDTH 2

/*
 * HAVE_GL:
 *
 * Do you have OpenGL? Set this to 0 if you don't have or plan to use
 * OpenGL, and FLTK will be smaller.
 */

#define HAVE_GL 1

/*
 * HAVE_GL_GLU_H:
 *
 * Do you have the OpenGL Utility Library header file?
 * (many broken Mesa RPMs do not...)
 */

#define HAVE_GL_GLU_H 1

/*
 * USE_COLORMAP:
 *
 * Setting this to zero will save a good deal of code (especially for
 * fl_draw_image), but FLTK will only work on TrueColor visuals.
 */

#define USE_COLORMAP 1

/*
 * HAVE_XDBE:
 *
 * Do we have the X double-buffer extension?
 */

#define HAVE_XDBE 0

/*
 * USE_XDBE:
 *
 * Actually try to use the double-buffer extension?  Set this to zero
 * disable use of XDBE without breaking the list_visuals program.
 */

#define USE_XDBE HAVE_XDBE

/*
 * HAVE_OVERLAY:
 *
 * Use the X overlay extension?  FLTK will try to use an overlay
 * visual for Fl_Overlay_Window, the Gl_Window overlay, and for the
 * menus.  Setting this to zero will remove a substantial amount of
 * code from FLTK.  Overlays have only been tested on SGI servers!
 */

#define HAVE_OVERLAY 0

/*
 * HAVE_GL_OVERLAY:
 *
 * It is possible your GL has an overlay even if X does not.  If so,
 * set this to 1.
 */

#define HAVE_GL_OVERLAY 1

/*
 * WORDS_BIGENDIAN:
 *
 * Byte order of your machine: 1 = big-endian, 0 = little-endian.
 */

#define WORDS_BIGENDIAN 0

/*
 * U16, U32, U64:
 *
 * Types used by fl_draw_image.  One of U32 or U64 must be defined.
 * U16 is optional but FLTK will work better with it!
 */

#define U16 unsigned short
#define U32 unsigned
#undef U64

/*
 * HAVE_DIRENT_H, HAVE_SYS_NDIR_H, HAVE_SYS_DIR_H, HAVE_NDIR_H, HAVE_SCANDIR:
 *
 * Where is <dirent.h> (used only by fl_file_chooser and scandir).
 */

/*#undef HAVE_DIRENT_H */
/*#undef HAVE_SYS_NDIR_H */
/*#undef HAVE_SYS_DIR_H */
/*#undef HAVE_NDIR_H */
/*#undef HAVE_SCANDIR */

/*
 * Possibly missing sprintf-style functions:
 */

#undef HAVE_VSNPRINTF
#undef HAVE_SNPRINTF

/*
 * String functions...
 */

#define HAVE_STRCASECMP	1
/*#undef HAVE_STRLCAT*/
/*#undef HAVE_STRLCPY*/

/*
 * Do we have POSIX locale support?
 */

#define HAVE_LOCALE_H 1
#define HAVE_LOCALECONV 1

/*
 * HAVE_POLL:
 *
 * Use poll() if we don't have select().
 */

#define HAVE_POLL 0

/*
 * Do we have various image libraries?
 */

#define HAVE_LIBPNG
#define HAVE_LIBZ
#define HAVE_LIBJPEG

/*
 * Which header file do we include for libpng?
 */

#define HAVE_PNG_H
#undef HAVE_LIBPNG_PNG_H

/*
 * Do we have the png_xyz() functions?
 */

#define HAVE_PNG_GET_VALID
#define HAVE_PNG_SET_TRNS_TO_ALPHA

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/math.h>
#include <FL/fl_draw.H>

// Pym Realtime Simulator (pymrs) module

#include <cholmod.h>
#include <umfpack.h>
#include <mosek.h>
#include <libconfig.h>

#include "include/PrsGraphCapi.h"

#include "PymStruct.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PhysicsThreadMain.h"
#include "PymCmdLineParser.h"
#include "MathUtil.h"

#include "model.h"
#include "common.h"
#include "image.h"
#include "quaternion.h"

#include "pymrscore.h"
#include "pymrsrender.h"
#include "pymrsphysics.h"

#endif // #ifndef __BWPCH_H__
