#ifndef __ARANPHYPCH_H__
#define __ARANPHYPCH_H__

#ifdef WIN32
#include <windows.h>
#endif

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

//
// Open Dynamics Engine (ode)
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
// AranMath
//
#include "Macros.h"
#include "ArnCommonTypes.h"
#include "ArnVec3.h"
#include "ArnVec4.h"
#include "ArnQuat.h"
#include "ArnMatrix.h"
#include "ArnMath.h"
#include "AranPhyStructs.h"

#endif
