ADD_LIBRARY(AranGl SHARED
AranGl.cpp AranGl.h
AranGlPCH.cpp AranGlPCH.h
ArnGlExt.cpp ArnGlExt.h
ArnMeshGl.cpp ArnMeshGl.h
ArnSkinningShaderGl.cpp ArnSkinningShaderGl.h
ArnTextureGl.cpp ArnTextureGl.h
BasicObjects.cpp BasicObjects.h
glInfo.cpp glInfo.h
VideoManGl.cpp VideoManGl.h
)
INCLUDE_DIRECTORIES(
..
../AranMath
../Aran
../AranPhy
../AranIk
)
ADD_DEFINITIONS(
-D_USRDLL
-DARANGL_EXPORTS
-DdDOUBLE
)
TARGET_LINK_LIBRARIES(AranGl
AranMath
Aran
AranIk
opengl32
glu32
)
ADD_MSVC_PRECOMPILED_HEADER(AranGl AranGlPCH.h AranGlPCH.cpp)
