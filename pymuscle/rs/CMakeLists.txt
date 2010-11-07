CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pymrs)
SET(PYMRS_SOURCES
pymrscore.cpp
pymrsrender.cpp
pymrsphysics.cpp
PrsGraph.cpp
PrsGraphCapi.cpp
PrsGraphData.cpp
)
SET(PYMRS_HEADERS
../PymPch.h
PymRsPch.h
pymrscore.h
pymrsrender.h
pymrsphysics.h
PrsGraph.h
PrsGraphCapi.h
PrsGraphData.h
)
INCLUDE_DIRECTORIES(
..
../core
../parser
../optimizer
${BOOST_ROOT}
${PYM_3RDPARTIES_ROOT}include
${PYM_3RDPARTIES_ROOT}CHOLMOD/Include
${PYM_3RDPARTIES_ROOT}libconfig-1.3.2
${PYM_3RDPARTIES_ROOT}glew-1.5.7/include
${MOSEK_ROOT}6/tools/platform/win32x86/h
)
LINK_DIRECTORIES(
${PYM_3RDPARTIES_ROOT}lib
${PYM_3RDPARTIES_ROOT}libconfig-1.3.2/Debug
${PYM_3RDPARTIES_ROOT}glew-1.5.7/lib
${MOSEK_ROOT}6/tools/platform/win32x86/bin
)
ADD_LIBRARY(pymrs SHARED ${PYMRS_SOURCES} ${PYMRS_HEADERS})
TARGET_LINK_LIBRARIES(pymrs pymoptimizer opengl32 glu32 glew32)
ADD_DEPENDENCIES(pymrs pymoptimizer)
