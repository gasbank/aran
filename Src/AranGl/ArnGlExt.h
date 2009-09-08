/*!
 * @file ArnGlExt.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

//
// OpenGL Extensions (Win32 only)
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
//
// Note: These externals should be declared in DllMain.cpp and set in ArnInitGlExtFunctions()
//
#ifdef WIN32
	#define ARN_GL_EXT_ENTRY(type, var) extern ARANGL_API type var;
	#include "ArnGlExtEntry.h"
	#undef ARN_GL_EXT_ENTRY
#endif

/*!
 * @brief OpenGL 확장 기능 초기화
 * @remarks 본 함수를 호출하기 전에 OpenGL 컨텍스트가 생성되어 있어야 합니다.
 * @return 음수이면 실패
 *
 * Windows 환경에서 AranGl 라이브러리를 사용하게 될 경우 반드시 가장 먼저 호출해야합니다.
 * 본 함수의 반환값이 음수이면 확장 기능 초기화에 실패했다는 뜻이며, 이 경우에는 AranGl 라이브러리 관련
 * 함수 호출시 에러가 발생하거나 예고 없이 프로그램이 종료되는 경우가 있습니다. 이 경우 대부분
 * 프로그램은 실행을 지속할 수 없기 때문에 종료시켜야 합니다.
 * @code
 * if (ArnInitGlExtFunctions() < 0)
 * {
 *	std::cerr << " *** OpenGL extensions needed to run this program are not available." << std::endl;
 *	std::cerr << "     Check whether you are in the remote control display or have a legacy graphics adapter." << std::endl;
 *	std::cerr << "     Aborting..." << std::endl;
 *	abort();
 * }
 * @endcode
 */
ARANGL_API int ArnInitGlExtFunctions();
