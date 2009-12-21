TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

LIBS += -ltinyxml_2_5_3

# Input
PRECOMPILED_HEADER = AranPCH.h
HEADERS += Animation.h \
           AranApi.h \
           AranPCH.h \
           ArnAction.h \
           ArnAnimationController.h \
           ArnBinaryChunk.h \
           ArnBone.h \
           ArnCamera.h \
           ArnContainer.h \
           ArnGenericBuffer.h \
           ArnGlExtEntry.h \
           ArnIndexBuffer.h \
           ArnIpo.h \
           ArnJoint.h \
           ArnLight.h \
           ArnMaterial.h \
           ArnMesh.h \
           ArnNode.h \
           ArnObject.h \
           ArnRenderableObject.h \
           ArnSceneGraph.h \
           ArnSkeleton.h \
           ArnTexture.h \
           ArnVertexBuffer.h \
           ArnXformable.h \
           ArnXmlLoader.h \
           BwWin32Timer.h \
           DefaultRenderLayer.h \
           doxygen_main.h \
           DungeonInterface.h \
           inotify-cxx.h \
           Log.h \
           MyError.h \
           PreciseTimer.h \
           Renderer.h \
           RenderLayer.h \
           resource.h \
           Singleton.h \
           Structs.h \
           version.h \
           VideoMan.h \
           ArnNode.inl \
           ArnSceneGraph.inl \
           ArnXformable.inl \
           ArnMesh.inl \
           ArnGenericBuffer.inl \
           PreciseTimer.inl
SOURCES += Animation.cpp \
           AranApi.cpp \
           AranPCH.cpp \
           ArnAction.cpp \
           ArnAnimationController.cpp \
           ArnBinaryChunk.cpp \
           ArnBone.cpp \
           ArnCamera.cpp \
           ArnContainer.cpp \
           ArnGenericBuffer.cpp \
           ArnIndexBuffer.cpp \
           ArnIpo.cpp \
           ArnJoint.cpp \
           ArnLight.cpp \
           ArnMaterial.cpp \
           ArnMesh.cpp \
           ArnNode.cpp \
           ArnObject.cpp \
           ArnRenderableObject.cpp \
           ArnSceneGraph.cpp \
           ArnSkeleton.cpp \
           ArnTexture.cpp \
           ArnVertexBuffer.cpp \
           ArnXformable.cpp \
           ArnXmlLoader.cpp \
           BwWin32Timer.cpp \
           DungeonInterface.cpp \
           inotify-cxx.cpp \
           Log.cpp \
           MyError.cpp \
           PreciseTimer.cpp \
           Renderer.cpp \
           RenderLayer.cpp \
           VideoMan.cpp
