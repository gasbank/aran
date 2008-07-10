// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers



//
// TODO: Specify 3ds Max version which this plugin applied to.
// -----> Check Project Properties! Do not define here!
//#define MODEL_EXPORTER_FOR_MAX_9
//#define MODEL_EXPORTER_FOR_MAX_2008


// C++
#include <iostream>
#include <iomanip>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include <crtdbg.h>
// Windows APIs
//#include <afxwin.h>
//#include <windows.h>

// DirectX SDK APIs (November 2007)
//#include <d3dx9xof.h>
//#include <rmxfguid.h>
//#include <d3dx9mesh.h>

// Max SDK APIs (General)
//#include "coreexp.h"
//#include "Max.h"
//#include "istdplug.h"
//#include "iparamb2.h"
//#include "iparamm2.h"
//#include "decomp.h"
//#include "utilapi.h"
//#include "IDxMaterial.h"
//#include "quat.h"
//#include "hold.h"

// Max SDK APIs (IGame)
#include "IGame/IGame.h"
#include "IGame/IGameObject.h"
#include "IGame/IGameProperty.h"
#include "IGame/IGameControl.h"
#include "IGame/IGameModifier.h"
#include "IGame/IConversionManager.h"
#include "IGame/IGameError.h"
#include "IGame/IGameFX.h"


#include "types2.h"
#include "../Aran/Singleton.h"

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



#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(x) { delete x; x = 0; }
#endif

#include <d3d9.h>
#include <d3dx9.h>

#include "../Aran/Structs.h"