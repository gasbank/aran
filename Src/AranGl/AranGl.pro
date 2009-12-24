TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

win32 {
    DEFINES += WIN32 \
        _USRDLL \
        ARANGL_EXPORTS
    #CONFIG -= embed_manifest_dll
    CONFIG(debug, debug|release) {
        LIBS += $${WORKING_ROOT}/AranMathD.lib
        LIBS += $${WORKING_ROOT}/AranD.lib
        LIBS += $${WORKING_ROOT}/AranIkD.lib
    }
    else {
        LIBS += $${WORKING_ROOT}/AranMath.lib
        LIBS += $${WORKING_ROOT}/Aran.lib
        LIBS += $${WORKING_ROOT}/AranIk.lib
    }
}

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
