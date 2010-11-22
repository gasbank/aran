ADD_LIBRARY(AranMath SHARED
AranMathPCH.cpp AranMathPCH.h
ArnConsts.cpp ArnConsts.h
ArnIntersection.cpp ArnIntersection.h
ArnMath.cpp ArnMath.h
ArnMatrix.cpp ArnMatrix.h
ArnPlane.cpp ArnPlane.h
ArnQuat.cpp ArnQuat.h
ArnVec3.cpp ArnVec3.h
ArnVec4.cpp ArnVec4.h
LinearR2.cpp LinearR2.h
LinearR3.cpp LinearR3.h
LinearR4.cpp LinearR4.h

MathMisc.h
Spherical.h
)
INCLUDE_DIRECTORIES(
..
)
ADD_DEFINITIONS(
-D_USRDLL
-DARANMATH_EXPORTS
)
ADD_MSVC_PRECOMPILED_HEADER(AranMath AranMathPCH.h AranMathPCH.cpp)
