TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

# Input
PRECOMPILED_HEADER = AranGlPCH.h
HEADERS += AranGl.h \
           AranGlPCH.h \
           ArnGlExt.h \
           ArnGlExtEntry.h \
           ArnMeshGl.h \
           ArnSkinningShaderGl.h \
           ArnTextureGl.h \
           BasicObjects.h \
           glext.h \
           glInfo.h \
           targetver.h \
           VideoManGl.h
SOURCES += AranGl.cpp \
           AranGlPCH.cpp \
           ArnGlExt.cpp \
           ArnMeshGl.cpp \
           ArnSkinningShaderGl.cpp \
           ArnTextureGl.cpp \
           BasicObjects.cpp \
           glInfo.cpp \
           VideoManGl.cpp
