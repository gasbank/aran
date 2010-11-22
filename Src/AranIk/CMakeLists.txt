ADD_LIBRARY(AranIk SHARED
AranIk.cpp AranIk.h
AranIkPCH.cpp AranIkPCH.h
ArnIkSolver.cpp ArnIkSolver.h
Jacobian.cpp Jacobian.h
MatrixRmn.cpp MatrixRmn.h
Node.cpp Node.h
RgbImage.cpp RgbImage.h
Tree.cpp Tree.h
VectorRn.cpp VectorRn.h

Misc.cpp
)
INCLUDE_DIRECTORIES(
..
../AranMath
../Aran
)
ADD_DEFINITIONS(
-D_USRDLL
-DARANIK_EXPORTS
)
TARGET_LINK_LIBRARIES(AranIk
AranMath
Aran
)
ADD_MSVC_PRECOMPILED_HEADER(AranIk AranIkPCH.h AranIkPCH.cpp)
