ADD_LIBRARY(Aran SHARED
ArnAction.cpp ArnAction.h
ArnAnimationController.cpp ArnAnimationController.h
ArnBinaryChunk.cpp ArnBinaryChunk.h
ArnBone.cpp ArnBone.h
ArnCamera.cpp ArnCamera.h
ArnContainer.cpp ArnContainer.h
ArnGenericBuffer.cpp ArnGenericBuffer.h
ArnIndexBuffer.cpp ArnIndexBuffer.h
ArnIpo.cpp ArnIpo.h
ArnLight.cpp ArnLight.h
ArnMaterial.cpp ArnMaterial.h
ArnMesh.cpp ArnMesh.h
ArnNode.cpp ArnNode.h
ArnObject.cpp ArnObject.h
ArnRenderableObject.cpp ArnRenderableObject.h
ArnSceneGraph.cpp ArnSceneGraph.h
ArnSkeleton.cpp ArnSkeleton.h
ArnTexture.cpp ArnTexture.h
ArnVertexBuffer.cpp ArnVertexBuffer.h
ArnXformable.cpp ArnXformable.h
ArnXmlLoader.cpp ArnXmlLoader.h
Animation.cpp Animation.h
AranApi.cpp AranApi.h
AranPCH.cpp AranPCH.h
DungeonInterface.cpp DungeonInterface.h
Log.cpp Log.h
MyError.cpp MyError.h
PreciseTimer.cpp PreciseTimer.h
Renderer.cpp Renderer.h
RenderLayer.cpp RenderLayer.h
VideoMan.cpp VideoMan.h
ArnJoint.cpp ArnJoint.h
BwWin32Timer.cpp BwWin32Timer.h

doxygen_main.h
Singleton.h
)
INCLUDE_DIRECTORIES(
..
../AranMath
)
ADD_DEFINITIONS(
-D_USRDLL
-DARAN_EXPORTS
-DTIXML_USE_STL
-DUSE_DEVIL_AS_IMAGELIB
)
TARGET_LINK_LIBRARIES(Aran
AranMath
tinyxmlSTL_D
zlib_D
DevIL
winmm
)
ADD_MSVC_PRECOMPILED_HEADER(Aran AranPCH.h AranPCH.cpp)
