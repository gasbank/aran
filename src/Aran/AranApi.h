#pragma once

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#endif

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

#include <boost/array.hpp>

#include "Macros.h"
#include "ArnCommonTypes.h"
#include "Structs.h"
#include "Singleton.h"
#include "ArnXmlLoader.h"
#include "ArnSceneGraph.h"
#include "ArnCamera.h"
#include "ArnSkeleton.h"
#include "ArnAnimationController.h"
#include "ArnMesh.h"
#include "ArnViewportData.h"
#include "ArnMath.h"
#include "ArnTexture.h"

ARAN_API long ArnGetBuildCount();
