CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pymoptimizer)
SET(PYMOPTIMIZER_SOURCES
Optimize.cpp 
ToSparse.cpp 
TripletMatrix.cpp 
fibDiffW.cpp
)
SET(PYMOPTIMIZER_HEADERS
../PymPch.h
PymOptimizerPch.h
Optimize.h 
ToSparse.h 
TripletMatrix.h 
fibDiffW.h
)
INCLUDE_DIRECTORIES(
..
../core
../parser
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
ADD_LIBRARY(pymoptimizer SHARED ${PYMOPTIMIZER_SOURCES} ${PYMOPTIMIZER_HEADERS})
TARGET_LINK_LIBRARIES(pymoptimizer pymcore pymparser mosek6_0)
ADD_DEPENDENCIES(pymoptimizer pymcore pymparser)
