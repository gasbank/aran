CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pymoptimizer)
SET(PYMOPTIMIZER_SOURCES
Optimize.cpp 
SymbolicTensor.cpp 
ToSparse.cpp 
TripletMatrix.cpp 
ExpBodyMoEq_real.cpp 
fibDiffW.cpp
)
SET(PYMOPTIMIZER_HEADERS
Optimize.h 
SymbolicTensor.h 
ToSparse.h 
TripletMatrix.h 
ExpBodyMoEq_real.h 
fibDiffW.h
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
ADD_LIBRARY(pymoptimizer SHARED ${PYMOPTIMIZER_SOURCES})
TARGET_LINK_LIBRARIES(pymoptimizer cholmod mosek6_0 libconfig)
ADD_DEPENDENCIES(pymoptimizer pymparser)
