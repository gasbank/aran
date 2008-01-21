// Main.h
// 2007 Geoyeob Kim

#pragma once

//
// TODO: Specify 3ds Max version which this plugin applied to.
//
//#define MODEL_EXPORTER_FOR_MAX_9
#define MODEL_EXPORTER_FOR_MAX_2008




// C++
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <algorithm>


// Windows APIs
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>



// DirectX SDK APIs (November 2007)
#include <d3dx9xof.h>
//#include <rmxfguid.h>
#include <d3dx9mesh.h>

// Max SDK APIs (General)
#include "coreexp.h"
#include "Max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "decomp.h"
#include "utilapi.h"
#include "IDxMaterial.h"
#include "quat.h"
#include "hold.h"

// Max SDK APIs (IGame)
#include "IGame/IGame.h"
#include "IGame/IGameObject.h"
#include "IGame/IGameProperty.h"
#include "IGame/IGameControl.h"
#include "IGame/IGameModifier.h"
#include "IGame/IConversionManager.h"
#include "IGame/IGameError.h"
#include "IGame/IGameFX.h"



#pragma comment(lib, "igame.lib")
#pragma comment(lib, "core.lib")
#pragma comment(lib, "paramblk2.lib")
#pragma comment(lib, "maxutil.lib")
#pragma comment(lib, "geom.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "d3d9.lib")
#if !defined(NDEBUG)
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxerr9.lib")


#pragma comment(lib, "bmm.lib")


