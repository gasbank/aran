CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pymcore)
SET(PYMCORE_SOURCES
MathUtil.cpp 
MuscleFiber.cpp 
RigidBody.cpp 
PymJointAnchor.cpp 
PymPodUtil.cpp 
ConvexHull.cpp
ConvexHullCapi.cpp
PymBiped.cpp 
PymuscleConfig.cpp 
StateDependents.cpp 
dRdv_real.cpp 
)
SET(PYMCORE_HEADERS
PymCorePch.h
MathUtil.h 
MuscleFiber.h 
RigidBody.h 
PymJointAnchor.h 
PymPodUtil.h 
ConvexHull.h
ConvexHullCapi.h
PymBiped.h 
PymuscleConfig.h 
StateDependents.h 
dRdv_real.h 
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
ADD_LIBRARY(pymcore SHARED ${PYMCORE_SOURCES})
