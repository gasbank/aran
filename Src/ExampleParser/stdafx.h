// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//
// ODE
//
#include <ode/ode.h>

#include "AranApi.h"

#include "AranPhy.h"
#include "SimWorld.h"
#include "GeneralBody.h"
#include "GeneralJoint.h"

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH