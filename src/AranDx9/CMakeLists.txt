ADD_LIBRARY(AranDx9 SHARED
AranDx9.cpp AranDx9.h
AranDx9PCH.cpp AranDx9PCH.h
ArnAnim.cpp ArnAnim.h
ArnFile.cpp ArnFile.h
ArnHierarchy.cpp ArnHierarchy.h
ArnMeshDx9.cpp ArnMeshDx9.h
ArnNodeFactory.cpp ArnNodeFactory.h
ArnSkeletonDx9.cpp ArnSkeletonDx9.h
ArnSkinInfo.cpp ArnSkinInfo.h
ArnTextureDx9.cpp ArnTextureDx9.h
Character.cpp Character.h
CharacterAnimationCallback.cpp CharacterAnimationCallback.h
CharacterInterface.cpp CharacterInterface.h
dllmain.cpp
InputMan.cpp InputMan.h
LoiterCallback.cpp LoiterCallback.h
ModelReader.cpp ModelReader.h
ResourceMan.cpp ResourceMan.h
UndefinedCallback.cpp UndefinedCallback.h
VideoManDx9.cpp VideoManDx9.h
WalkCallback.cpp WalkCallback.h
)
INCLUDE_DIRECTORIES(
..
../AranMath
../Aran
../AranPhy
)
ADD_DEFINITIONS(
-D_USRDLL
-DARANDX9_EXPORTS
-DdDOUBLE
)
TARGET_LINK_LIBRARIES(AranDx9
AranMath
Aran
d3d9
d3dx9d
)
ADD_MSVC_PRECOMPILED_HEADER(AranDx9 AranDx9PCH.h AranDx9PCH.cpp)
