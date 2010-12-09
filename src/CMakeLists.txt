CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(aran)
SET(CMAKE_OSX_ARCHITECTURES i386)
# Get CODEBASE and WORKING from envvar.
SET(CODEBASE $ENV{CODEBASE})
SET(WORKING $ENV{WORKING})
SET(MOSEK_ROOT $ENV{MOSEK_ROOT})
SET(BOOST_ROOT $ENV{BOOST_ROOT})
SET(CMAKE_OSX_ARCHITECTURES i386)
SET(PYM_3RDPARTIES_ROOT "${CODEBASE}src/")

if (MSVC90)
  SET(compiler_string /vc90)
endif (MSVC90)
if (MSVC10)
  SET(compiler_string /vc10)
endif (MSVC10)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${WORKING}/lib${compiler_string} )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WORKING}/bin${compiler_string} )
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${WORKING}/lib${compiler_string} )

MACRO(ADD_MSVC_PRECOMPILED_HEADER tgt hdr src)
if (MSVC)
   set_target_properties(${tgt} PROPERTIES COMPILE_FLAGS "/Yu\"${hdr}\"")
   set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS "/Yc\"${hdr}\"")
endif(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)
INCLUDE_DIRECTORIES(
${CODEBASE}tinyxml_2_5_3
${CODEBASE}opende/include
${CODEBASE}cml-1_0_1
${CODEBASE}zlib-1.2.3
${CODEBASE}DevIL-SDK-x86-1.7.8/include
)
LINK_DIRECTORIES(
${BOOST_ROOT}stage/lib
${CODEBASE}sdl-1.2.14/lib
${CODEBASE}SDL_gfx-2.0.22/Debug
${CODEBASE}SDL-1.2.13/VisualC/SDL/Debug
${CODEBASE}SDL-1.2.13/VisualC/SDLmain/Debug
${CODEBASE}DevIL-SDK-x86-1.7.8/lib
${CODEBASE}Working/lib
${CODEBASE}freetype-2.3.9/objs/win32/vc2008
${CODEBASE}DevIL-SDK-x86-1.7.8/lib
${CODEBASE}fltk-1.1.10/lib
${CODEBASE}src/glew-1.5.7/lib
${CODEBASE}opende/lib/DebugDoubleDLL
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
ADD_SUBDIRECTORY(AranMath_Test)
ADD_SUBDIRECTORY(AranIk_Test)
ADD_SUBDIRECTORY(AranPymuscle)
ADD_SUBDIRECTORY(pymuscle)
#ADD_SUBDIRECTORY(TexturedQuadViewer)
#ADD_SUBDIRECTORY(SceneViewer)

#
#   AranMath
#      |
#      |
#      |
#    Aran
#      |
#      +---------+-----------+----------+
#      |         |           |          |
#      |         |           |          |
#    AranIk    AranDx9    AranGl      AranPhy
#
ADD_DEPENDENCIES(Aran AranMath)
ADD_DEPENDENCIES(AranIk Aran)
ADD_DEPENDENCIES(AranPhy Aran)
ADD_DEPENDENCIES(AranDx9 AranMath Aran AranPhy AranIk)
ADD_DEPENDENCIES(AranGl AranMath Aran AranPhy AranIk)

ADD_DEPENDENCIES(AranIk_Test AranMath Aran AranPhy AranIk)
ADD_DEPENDENCIES(AranPymuscle AranMath Aran AranPhy AranIk pymcore pymoptimizer pymrs pymparser)
