/*
 * PymPch.h
 *
 *  Created on: 2010. 7. 22.
 *      Author: johnu
 */

#ifndef PYMCOREPCH_H_
#define PYMCOREPCH_H_

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

//
// Boost C++
//
#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/array.hpp>
#include <boost/static_assert.hpp>
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


#include <cholmod.h>
#include <umfpack.h>
#include <mosek.h>
#include <libconfig.h>


#endif /* PYMCOREPCH_H_ */
