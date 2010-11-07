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
pymrscore.h
pymrsrender.h
pymrsphysics.h
PrsGraph.h
PrsGraphCapi.h
PrsGraphData.h
)
SET(CMAKE_OSX_ARCHITECTURES i386)
INCLUDE_DIRECTORIES(C:/devel/src/include C:/devel/src/CHOLMOD/Include "C:/Program Files/Mosek/6/tools/platform/win32x86/h" C:/devel/libconfig-1.3.2)
INCLUDE_DIRECTORIES(C:/devel/src/libconfig-1.3.2)
INCLUDE_DIRECTORIES(C:/devel/src/glew-1.5.7/include)
INCLUDE_DIRECTORIES(C:/boost_1_44_0)
LINK_DIRECTORIES(C:/devel/src/lib "C:/Program Files/Mosek/6/tools/platform/win32x86/bin")
LINK_DIRECTORIES(C:/devel/src/libconfig-1.3.2/Debug)
LINK_DIRECTORIES(C:/devel/src/glew-1.5.7/lib)
#ADD_EXECUTABLE(KGServer ${KGSERVER_SOURCES})

ADD_LIBRARY(pymrs SHARED ${PYMRS_SOURCES})
TARGET_LINK_LIBRARIES(pymrs pymoptimizer opengl32 glu32 glew32)
ADD_DEPENDENCIES(pymrs pymoptimizer pymparser)
