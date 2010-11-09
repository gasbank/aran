#pragma once
#ifndef __PYMRSPCH_H_
#define __PYMRSPCH_H_

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

#endif /* #ifndef __PYMRSPCH_H_ */
