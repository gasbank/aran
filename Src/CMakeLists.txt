CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(aran)
SET(CMAKE_OSX_ARCHITECTURES i386)
SET(CODEBASE "f:/devel3/")
SET(WORKING "f:/devel3/Working/")
SET(CMAKE_OSX_ARCHITECTURES i386)
SET(PYM_3RDPARTIES_ROOT f:/devel3/src/)
SET(MOSEK_ROOT "C:/Program Files (x86)/Mosek/")
SET(BOOST_ROOT d:/boost_1_44_0)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${WORKING} )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WORKING} )
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${WORKING} )

MACRO(ADD_MSVC_PRECOMPILED_HEADER tgt hdr src)
if (MSVC)
   set_target_properties(${tgt} PROPERTIES COMPILE_FLAGS "/Yu\"${hdr}\"")
   set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS "/Yc\"${hdr}\"")
endif(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)
LINK_DIRECTORIES(
D:/boost_1_44_0/stage/lib
${CODEBASE}sdl-1.2.14/lib
${CODEBASE}SDL_gfx-2.0.22/Debug
${CODEBASE}SDL-1.2.13/VisualC/SDL/Debug
${CODEBASE}SDL-1.2.13/VisualC/SDLmain/Debug
${CODEBASE}DevIL-SDK-x86-1.7.8/lib
${CODEBASE}zlib-1.2.3/projects/visualc6/Win32_DLL_Debug
${CODEBASE}tinyxml_2_5_3/tinyxml/Debug_STL
${CODEBASE}Working/lib
${CODEBASE}freetype-2.3.9/objs/win32/vc2008
${CODEBASE}DevIL-SDK-x86-1.7.8/lib
${CODEBASE}fltk-1.1.10/lib
${CODEBASE}arangit/pymuscle/build/bin/Debug
${CODEBASE}src/glew-1.5.7/lib
${PYM_3RDPARTIES_ROOT}lib
${PYM_3RDPARTIES_ROOT}libconfig-1.3.2/Debug
${PYM_3RDPARTIES_ROOT}glew-1.5.7/lib
${MOSEK_ROOT}6/tools/platform/win32x86/bin
)
# --- aran library modules ---
ADD_SUBDIRECTORY(AranMath)
ADD_SUBDIRECTORY(Aran)
ADD_SUBDIRECTORY(AranIk)
ADD_SUBDIRECTORY(AranPhy)
ADD_SUBDIRECTORY(AranDx9)
ADD_SUBDIRECTORY(AranGl)
# --- example projects ---
ADD_SUBDIRECTORY(AranIk_Test)
ADD_SUBDIRECTORY(pymuscle)
#ADD_SUBDIRECTORY(TexturedQuadViewer)
#ADD_SUBDIRECTORY(SceneViewer)

ADD_DEPENDENCIES(Aran AranMath)
ADD_DEPENDENCIES(AranIk AranMath Aran)
ADD_DEPENDENCIES(AranDx9 AranMath Aran AranPhy AranIk)
ADD_DEPENDENCIES(AranGl AranMath Aran AranPhy AranIk)
ADD_DEPENDENCIES(AranIk_Test AranMath Aran AranPhy AranIk pymcore pymoptimizer pymrs pymparser Jarvis)
