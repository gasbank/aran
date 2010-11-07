CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pymparser)
SET(PYMPARSER_SOURCES 
PymCmdLineParser.cpp
TrajParser.cpp
)
SET(PYMPARSER_HEADERS
PymCmdLineParser.h
TrajParser.h
)
SET(CMAKE_OSX_ARCHITECTURES i386)
INCLUDE_DIRECTORIES(C:/devel/src/include C:/devel/src/CHOLMOD/Include "C:/Program Files/Mosek/6/tools/platform/win32x86/h" C:/devel/libconfig-1.3.2)
INCLUDE_DIRECTORIES(C:/devel/src/libconfig-1.3.2)
INCLUDE_DIRECTORIES(C:/devel/src/glew-1.5.7/include)
INCLUDE_DIRECTORIES(C:/boost_1_44_0)
INCLUDE_DIRECTORIES(..)
LINK_DIRECTORIES(C:/devel/src/lib "C:/Program Files/Mosek/6/tools/platform/win32x86/bin")
LINK_DIRECTORIES(C:/devel/src/libconfig-1.3.2/Debug)
LINK_DIRECTORIES(C:/devel/src/glew-1.5.7/lib)
#ADD_EXECUTABLE(KGServer ${KGSERVER_SOURCES})
#ADD_DEPENDENCIES(KGServer KGCommon)
ADD_DEFINITIONS(-D_USRDLL -DPYMPARSER_EXPORTS)
ADD_LIBRARY(pymparser SHARED ${PYMPARSER_SOURCES})
